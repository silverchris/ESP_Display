#include <cstdio>
#include <deque>
#include <unordered_map>
#include <ArduinoJson.h>
#include <esp_event.h>

#include "ha_event.h"
#include "websocket.h"

#include "ha.hpp"

StaticJsonDocument<100> doc_out;

void callback_state_events(const JsonDocument &json) {
    if (strncmp(json["type"], "event", 5) == 0) {
        const char *type = json["event"]["event_type"];
        if (strcmp(type, "state_changed") == 0) {
            JsonObjectConst event = json["event"]["data"];
            const char *entity = event["entity_id"];
            JsonObjectConst new_state = event["new_state"];
            update_entity(entity, new_state);
            printf("Event %s\n", entity);
        }
    }
}

void callback_entities(const JsonDocument &json) {
    for (JsonObjectConst v : json["result"].as<JsonArrayConst>()) {
        std::string id = v["device_id"];
        std::string entity_id = v["entity_id"];
        add_entity((char *) id.c_str(), (char *) entity_id.c_str());
    }
    esp_event_post_to(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_READY, nullptr, 0, 100);
}

void callback_devices(const JsonDocument &json) {
    for (JsonObjectConst v : json["result"].as<JsonArrayConst>()) {
        std::string name = v["name"];
        std::string id = v["id"];
        std::string area_id = v["area_id"];
        printf("Device: %s, ID: %s, Area: %s\n", name.c_str(), id.c_str(), area_id.c_str());
        create_device((char *) id.c_str(), (char *) area_id.c_str(), (char *) name.c_str());
        printf("Device: %s\n", name.c_str());
    }
    doc_out["type"] = "config/entity_registry/list";
    ws_queue_add(doc_out, callback_entities);
}

void callback_area(const JsonDocument &json) {
    for (JsonObjectConst v : json["result"].as<JsonArrayConst>()) {
        std::string name = v["name"];
        std::string id = v["area_id"];
        create_area((char *) id.c_str(), (char *) name.c_str());
        printf("Area: %s\n", name.c_str());
    }
    doc_out["type"] = "config/device_registry/list";
    ws_queue_add(doc_out, callback_devices);
}


void callback_auth(const JsonDocument &json) {
    doc_out["type"] = "subscribe_events";
    doc_out["event_type"] = "state_changed";
    ws_queue_add(doc_out);
    doc_out["type"] = "config/area_registry/list";
    ws_queue_add(doc_out, callback_area);
}
