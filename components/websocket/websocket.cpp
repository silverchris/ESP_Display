#include <climits>
#include <vector>
#include <unordered_map>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_websocket_client.h"

#include "websocket.h"

#include "ha_callbacks.h"

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
    uint8_t c;
    if (next != 0) {
        c = next;
        next = 0;
        return c;
    }
    size_t size = 0;
    uint8_t *data = (uint8_t *) xRingbufferReceiveUpTo(buf, &size, 100, 1);
    if (size > 0) {
        c = *data;
        vRingbufferReturnItem(buf, data);
        if (c >= 32 && c <= 127) {
            return c;
        }
    }
    return -1;
}

int CustomReader::peek() {
    uint8_t c;
    size_t size = 0;
    uint8_t *data = (uint8_t *) xRingbufferReceiveUpTo(buf, &size, 100, 1);
    if (size > 0) {
        c = *data;
        vRingbufferReturnItem(buf, data);
        if (c != 29) {
            next = c;
        }
        return c;
    } else {
        return -1;
    }
}


bool CustomReader::empty() {
    return xRingbufferGetCurFreeSize(buf) == 10000;
}

void login() {
    StaticJsonDocument<300> doc;
    doc["type"] = "auth";
    doc["access_token"] = CONFIG_HA_KEY;
    message_id++;
    char message[500];
    serializeJson(doc, message, sizeof(message));
    esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
}

[[noreturn]] void json_task(void *pvParameters) {
    auto json_stream = (CustomReader *) pvParameters;
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
        if (!json_stream->empty())
            error = deserializeJson(doc_in, *json_stream, DeserializationOption::Filter(filter));
        if (json_stream->peek() == 29) {
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
                    login();
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
        if (json_stream->empty()) {
            vTaskSuspend(json_task_handle);
        } else {
            taskYIELD();
        }
    }
//    free(out);
}


static void websocket_event_handler(void *handler_args, __unused esp_event_base_t base, int32_t event_id, void *event_data) {
    auto *data = (esp_websocket_event_data_t *) event_data;
    auto buf = (RingbufHandle_t) handler_args;

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
                xRingbufferSend(buf, data->data_ptr, (size_t) data->data_len, 100);
                if (data->payload_offset + data->data_len >= data->payload_len) {
                    uint8_t eof = 29;
                    xRingbufferSend(buf, &eof, 1, 100);
                }
                vTaskResume(json_task_handle);
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
            break;
        default:
            break;
    }
}

StaticRingbuffer_t static_buffer;
uint8_t static_buffer_data[10000];

void websocket_init(void) {
    esp_websocket_client_config_t websocket_cfg = {};

    websocket_cfg.uri = "ws://" CONFIG_HA_ADDRESS "/api/websocket";
    websocket_cfg.port = 8123;
    websocket_cfg.buffer_size = WS_BUFFER_SIZE;
    websocket_cfg.task_stack = 10000;


    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    client = esp_websocket_client_init(&websocket_cfg);

    RingbufHandle_t buf_handle = xRingbufferCreateStatic(10000, RINGBUF_TYPE_BYTEBUF,
                                                         static_buffer_data, &static_buffer);

    if (buf_handle == nullptr) {
        ESP_LOGE(TAG, "Failed to create ring buffer\n");
    }

    auto json_stream = new CustomReader(buf_handle);

    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *) buf_handle);

    xTaskCreate(json_task, "json_task", 10000, (void *) json_stream, 0, &json_task_handle);

    esp_websocket_client_start(client);

    while (true) {
        if (esp_websocket_client_is_connected(client)) {
            login();
            break;
        }
    }
    ESP_LOGI(TAG, "WEBSOCKET_INIT FINISHED");

}