#include <vector>
#include <cstring>
#include <deque>
#include <unordered_map>
#include <csignal>

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

static const char *TAG = "WEBSOCKET";

#include <ArduinoJson.h>

#include "secrets.h"

#include "websocket.h"

#include "callbacks.hpp"

bool ha_ready = false;

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
    std::string message;
    serializeJson(doc, message);
    esp_websocket_client_send_text(client, message.c_str(), message.length(), portMAX_DELAY);
    doc.clear();
}

void ws_queue_add(JsonDocument &doc) {
    doc["id"] = message_id;
    message_id++;
    std::string message;
    serializeJson(doc, message);
    esp_websocket_client_send_text(client, message.c_str(), message.length(), portMAX_DELAY);
    doc.clear();
}

void handle_messages(char *json, int len) {
    DynamicJsonDocument doc_in(6000);
    DynamicJsonDocument filter(500);
    filter["type"] = true;
    filter["id"] = true;
    filter["success"] = true;
    filter["result"][0]["area_id"] = true;
    filter["result"][0]["id"] = true;
    filter["result"][0]["name"] = true;
    filter["result"][0]["device_id"] = true;
    filter["result"][0]["entity_id"] = true;
    filter["event"]["event_type"] = true;
    filter["event"]["data"]["entity_id"] = true;
    filter["event"]["data"]["new_state"]["state"] = true;
    filter["event"]["data"]["new_state"]["attributes"]["brightness"] = true;

    DeserializationError error = deserializeJson(doc_in, (const char *) json, len,
                                                 DeserializationOption::Filter(filter));

    // Test if parsing succeeds.
    if (error) {
        printf("%s\n", "deserializeJson() failed");
        return;
    }

    const char *type = doc_in["type"];

    if (strcmp(type, "auth_ok") == 0) {
        callback_auth(doc_in);
    } else if (strcmp(type, "auth_required") == 0) {
        StaticJsonDocument<300> doc_out;
        doc_out["type"] = "auth";
        doc_out["access_token"] = HA_TOKEN;
    } else if (strcmp(type, "event") == 0) {
        callback_state_events(doc_in);
        printf("done event callback\n");
    } else {
        uint16_t id = doc_in["id"];
        if (CallbackMap.count(id) > 0) {
            CallbackMap[id](doc_in);
            CallbackMap.erase(id);
            printf("done callback\n");
        }
    }
}

char buffer[10000];

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    auto *data = (esp_websocket_event_data_t *) event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
            ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
            if (data->op_code == 1) {
                ESP_LOGI(TAG, "Data Len=%d", data->data_len);
                ESP_LOGI(TAG, "Payload Len=%d", data->payload_len);
                ESP_LOGW(TAG, "Received=%.*s\r\n", data->data_len, (char *) data->data_ptr);
                strncat(buffer, (char *) data->data_ptr, data->data_len < 10000 ? data->data_len : 10000);
                ESP_LOGI(TAG, "string Len=%d", strlen(buffer));
                if (data->data_len == data->payload_len || strlen(buffer) == data->payload_len) {
                    handle_messages(buffer, data->payload_len);
                    buffer[0] = '\0';
                }
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
            break;
    }
}

void websocket_init(void) {
    esp_websocket_client_config_t websocket_cfg = {};

    websocket_cfg.uri = "ws://" HA_ADDRESS "/api/websocket";
    websocket_cfg.port = 8123;
    websocket_cfg.buffer_size = 500;
    websocket_cfg.task_stack = 10000;


    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *) client);

    esp_websocket_client_start(client);
    while (true) {
        if (esp_websocket_client_is_connected(client)) {
            StaticJsonDocument<300> doc_out;
            doc_out["type"] = "auth";
            doc_out["access_token"] = HA_TOKEN;
            message_id++;
            std::string message;
            serializeJson(doc_out, message);
            esp_websocket_client_send_text(client, message.c_str(), message.length(), portMAX_DELAY);
            break;
        }
    }
}