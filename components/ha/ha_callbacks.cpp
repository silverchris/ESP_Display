#include <cstdio>
#include <ArduinoJson.h>
#include <esp_event.h>

#include "ha_event.h"
#include "websocket.h"

#include "ha.h"
#include "ha_entities.h"
#include "ha_devices.h"

StaticJsonDocument<100> doc_out;

void callback_state_events(const JsonDocument &json) {
    if (strncmp(json["type"], "event", 5) == 0) {
        const char *type = json["event"]["event_type"];
        if (strcmp(type, "state_changed") == 0) {
            JsonObjectConst event = json["event"]["data"];
            const char *entity = event["entity_id"];
            JsonObjectConst new_state = event["new_state"];
            update_entity(entity, new_state);
        }
    }
}

void callback_state_events_register(const JsonDocument &json) {
    ha_state_set(ha_state_finished);
}

void callback_state(const JsonDocument &json) {
    for (JsonObjectConst v : json["result"].as<JsonArrayConst>()) {
        const char *entity = v["entity_id"];
        add_entity((const char *) v["entity_id"]);
        update_entity(entity, v);

    }
    ha_state_set(ha_state_subscribe);
//    esp_event_post_to(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_READY, nullptr, 0, 100);
//    doc_out["type"] = "subscribe_events";
//    doc_out["event_type"] = "state_changed";
//    ws_queue_add(doc_out);
}

void callback_entities(const JsonDocument &json) {
    for (JsonObjectConst v : json["result"].as<JsonArrayConst>()) {
        if(v.containsKey("entity_id")) {
            add_entity((const char *) v["entity_id"]);
        }
    }
    ha_state_set(ha_state_subscribe);
//    esp_event_post_to(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_READY, nullptr, 0, 100);
//    doc_out["type"] = "get_states";
//    ws_queue_add(doc_out, callback_state);
}

void callback_devices(const JsonDocument &json) {
    for (JsonObjectConst v : json["result"].as<JsonArrayConst>()) {
        const char * name = v["name"];
        const char * id = v["id"];
        const char * area_id = v["area_id"];
        add_device(id, name, area_id);
    }
}

//void callback_area(const JsonDocument &json) {
//    for (JsonObjectConst v : json["result"].as<JsonArrayConst>()) {
//        std::string name = v["name"];
//        std::string id = v["area_id"];
//        create_area((char *) id.c_str(), (char *) name.c_str());
//        printf("Area: %s\n", name.c_str());
//    }
//    doc_out["type"] = "config/device_registry/list";
//    ws_queue_add(doc_out, callback_devices);
//}


void callback_auth() {
//    doc_out["type"] = "config/area_registry/list";
//    ws_queue_add(doc_out, callback_area);
//    doc_out["type"] = "get_states";
//    ws_queue_add(doc_out, callback_state);
    ha_state_set(ha_state_entities);

}
