#include <vector>
#include <unordered_map>
#include <csignal>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

#include <ArduinoJson.h>

#include "websocket.h"

#include "ha_callbacks.h"

#define STREAM_TIMEOUT 1000000  // default number of micro-seconds to wait
#define WS_BUFFER_SIZE 4000

static const char *TAG = "WEBSOCKET";

TaskHandle_t json_task_handle;

std::unordered_map<uint16_t, void (*)(const JsonDocument &json)> CallbackMap;

esp_websocket_client_handle_t client;

uint16_t message_id;

void ws_queue_add(JsonDocument &doc, void (*f)(const JsonDocument &json)) {
    if (strcmp(doc["type"], "auth") != 0) {
        doc["id"] = message_id;
        message_id++;
        CallbackMap.emplace(doc["id"], f);
    } else {
        CallbackMap.emplace(0, f);
    }
    char message[500];
    serializeJson(doc, message, sizeof(message));
    esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
    doc.clear();
}

void ws_queue_add(JsonDocument &doc) {
    doc["id"] = message_id;
    message_id++;
    char message[500];
    serializeJson(doc, message, sizeof(message));
    esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
    doc.clear();
}


int CustomReader::read() {
    int c = -1;
    int64_t _startMillis = esp_timer_get_time();
    do {
        if (!buf.empty()) {
            if (xSemaphoreTake(lock, (TickType_t) 10) == pdTRUE) {
                c = buf.front();
                if (c != -2) {
                    buf.pop_front();
                }
            }
            xSemaphoreGive(lock);
        }
        if (c == -2)
            return c;
        if (c >= 0)
            return c;
        vTaskDelay(25);
    } while (esp_timer_get_time() - _startMillis < STREAM_TIMEOUT);
    return -1;     // -1 indicates timeout
}

int CustomReader::peek() {
    int c = -1;
    if (!buf.empty()) {
        if (xSemaphoreTake(lock, (TickType_t) 10) == pdTRUE) {
            c = buf.front();
        }
        xSemaphoreGive(lock);
    }
    return c;
}

void CustomReader::pop() {
    if (!buf.empty()) {
        if (xSemaphoreTake(lock, (TickType_t) 10) == pdTRUE) {
            buf.pop_front();
        }
        xSemaphoreGive(lock);
    }
}


void CustomReader::fill(char *buffer, size_t length) {
    if (xSemaphoreTake(lock, (TickType_t) 10) == pdTRUE) {
        for (int i = 0; i < length; i++) {
            buf.push_back(buffer[i]);
        }
    }
    xSemaphoreGive(lock);


}

void CustomReader::putc(int character) {
    buf.push_back(character);
}


bool CustomReader::empty() {
    return buf.empty();
}


[[noreturn]] void json_task(void *pvParameters) {
    auto jsonstream = (CustomReader *) pvParameters;
//    char *out = (char *)malloc(10000);
    DynamicJsonDocument doc_in(20000);
    DynamicJsonDocument filter(1000);
    filter["type"] = true;
    filter["id"] = true;
    filter["success"] = true;
    filter["result"][0]["area_id"] = true;
    filter["result"][0]["id"] = true;
    filter["result"][0]["name"] = true;
    filter["result"][0]["device_id"] = true;
    filter["result"][0]["entity_id"] = true;
    filter["result"][0]["state"] = true;
    filter["result"][0]["attributes"] = true;
    filter["result"][0]["context"]["id"] = true;
    filter["event"]["event_type"] = true;
    filter["event"]["data"]["entity_id"] = true;
    filter["event"]["data"]["new_state"]["state"] = true;
    filter["event"]["data"]["new_state"]["attributes"] = true;

    DeserializationError error;

    while (true) {
        if (!jsonstream->empty()) {
            error = deserializeJson(doc_in, *jsonstream, DeserializationOption::Filter(filter));
        }
        if (jsonstream->peek() == -2) {
            jsonstream->pop(); //throw away our end of message character
            if (error) {
                printf("deserializeJson() failed: %s\n", error.c_str());
                doc_in.clear();
            } else {
//                serializeJsonPretty(doc_in, out, 10000);
//                printf("json out: %s\n", out);
                const char *type = doc_in["type"];
//                printf("type: %s\n", type);
                if (strcmp(type, "auth_ok") == 0) {
                    callback_auth();
                } else if (strcmp(type, "auth_required") == 0) {
                } else if (strcmp(type, "event") == 0) {
                    callback_state_events(doc_in);
                    printf("done event callback\n");
                } else if (strcmp(type, "result") == 0) {
                    uint16_t id = doc_in["id"];
                    if (CallbackMap.count(id) > 0) {
                        CallbackMap[id](doc_in);
                        CallbackMap.erase(id);
                        printf("done callback\n");
                    }
                }
                doc_in.clear();
            }
        }
        if (jsonstream->empty()) {
            vTaskSuspend(json_task_handle);
        } else {
            taskYIELD();
        }
    }
//    free(out);
}


static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    auto *data = (esp_websocket_event_data_t *) event_data;
    auto *jsonstream = (CustomReader *) handler_args;

    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGD(TAG, "WEBSOCKET_EVENT_DATA");
            ESP_LOGD(TAG, "Received opcode=%d", data->op_code);
            ESP_LOGD(TAG, "Data Len=%d", data->data_len);
            ESP_LOGD(TAG, "Payload Len=%d", data->payload_len);
            ESP_LOGD(TAG, "Payload Offset=%d", data->payload_offset);
            ESP_LOGD(TAG, "Received=%.*s\r\n", data->data_len, (char *) data->data_ptr);
            if (data->op_code == 1) {
                jsonstream->fill((char *) data->data_ptr, (size_t) data->data_len);
                if (data->payload_offset + data->data_len >= data->payload_len) {
                    jsonstream->putc(-2);
                }
                vTaskResume(json_task_handle);
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
            break;
    }
}

void websocket_init(void) {
    esp_websocket_client_config_t websocket_cfg = {};

    websocket_cfg.uri = "ws://" CONFIG_HA_ADDRESS "/api/websocket";
    websocket_cfg.port = 8123;
    websocket_cfg.buffer_size = WS_BUFFER_SIZE;
    websocket_cfg.task_stack = 10000;


    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    client = esp_websocket_client_init(&websocket_cfg);

    auto jsonstream = new CustomReader;

    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *) jsonstream);

    xTaskCreate(json_task, "json_task", 10000, (void *) jsonstream, 0, &json_task_handle);

    esp_websocket_client_start(client);

    while (true) {
        if (esp_websocket_client_is_connected(client)) {
            StaticJsonDocument<300> doc;
            doc["type"] = "auth";
            doc["access_token"] = CONFIG_HA_KEY;
            message_id++;
            char message[500];
            serializeJson(doc, message, sizeof(message));
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
            break;
        }
    }
}