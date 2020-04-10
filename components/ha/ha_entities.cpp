#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <ArduinoJson.h>

#include "ha.hpp"
#include "ha_event.h"
#include "websocket.h"

#include "ha_entities.h"

ha_entity::ha_entity(const char *entity_id, const char *dname) {
    printf("Creating entity: %s\n", id);
    strncpy(id, entity_id, sizeof(id));
    if(dname != nullptr){
        strncpy(name, dname, sizeof(name));
    }

    if (strncmp(entity_id, "light.", 5) == 0) {
        type = ha_entity_type::ha_light;
    } else if (strncmp(entity_id, "switch.", 7) == 0) {
        type = ha_entity_type::ha_switch;
    } else if (strncmp(entity_id, "binary_sensor.", 14) == 0) {
        type = ha_entity_type::ha_binary_sensor;
    } else if (strncmp(entity_id, "weather.", 8) == 0) {
        type = ha_entity_type::ha_weather;
    } else if (strncmp(entity_id, "sensor.", 7) == 0) {
        type = ha_entity_type::ha_sensor;
    }
    state = 0;
}

void ha_entity::update(JsonObjectConst &doc) {
    supported_features = doc["attributes"]["supported_features"];
    for (void *call : callbacks) {
        ha_event_data out = {.ptr = call};
        ha_event_post(HA_EVENT_UPDATE, &out, sizeof(ha_event_data), 100);
    }
}

void ha_entity_switch::toggle() {
}

void ha_entity_switch::update(JsonObjectConst &doc) {
    state = static_cast<uint8_t>(strncmp(doc["state"], "on", 2) == 0);
    ha_entity::update(doc);
}

void ha_entity_light::dim(uint8_t dim_value) {
    StaticJsonDocument<300> doc_out;
    doc_out["type"] = "call_service";
    doc_out["domain"] = "light";
    doc_out["service"] = "turn_on";
    doc_out["service_data"]["entity_id"] = id;
    doc_out["service_data"]["transition"] = 1;
    doc_out["service_data"]["brightness"] = (uint8_t)dim_value;
    ws_queue_add(doc_out);
}

void ha_entity_light::toggle() {
    StaticJsonDocument<300> doc_out;
    doc_out["type"] = "call_service";
    doc_out["domain"] = "light";
    doc_out["service"] = "toggle";
    doc_out["service_data"]["entity_id"] = id;
    ws_queue_add(doc_out);
}

void ha_entity_light::features(void *feature_struct_ptr) {
    auto fstruct = (ha_light_features *) feature_struct_ptr;
    printf("supports brightness %i\n", supported_features & HA_LIGHT_SUPPORT_BRIGHTNESS);
    fstruct->SUPPORT_BRIGHTNESS = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_BRIGHTNESS);
    fstruct->SUPPORT_COLOR_TEMP = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_COLOR_TEMP);
    fstruct->SUPPORT_EFFECT = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_EFFECT);
    fstruct->SUPPORT_FLASH = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_FLASH);
    fstruct->SUPPORT_COLOR = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_COLOR);
    fstruct->SUPPORT_TRANSITION = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_TRANSITION);
    fstruct->SUPPORT_WHITE_VALUE = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_WHITE_VALUE);
}


void ha_entity_light::update(JsonObjectConst &doc) {
    state = static_cast<uint8_t>(strncmp(doc["state"], "on", 2) == 0);
    brightness = doc["attributes"]["brightness"];
    color_temp = doc["attributes"]["color_temp"];
    min_mireds = doc["attributes"]["min_mireds"];
    min_mireds = doc["attributes"]["min_mireds"];
    white_value = doc["attributes"]["white_value"];
    ha_entity::update(doc);
}

void ha_entity_weather::update(JsonObjectConst &doc) {
    temperature = doc["attributes"]["temperature"];
    pressure = doc["attributes"]["pressure"];
    humidity = doc["attributes"]["humidity"];
    visibility = doc["attributes"]["visibility"];
    wind_speed = doc["attributes"]["wind_speed"];
    wind_bearing = doc["attributes"]["wind_bearing"];
    strncpy(friendly_name, (const char *) doc["attributes"]["friendly_name"], sizeof(friendly_name));

    ha_entity::update(doc);
}

void ha_entity_sensor::update(JsonObjectConst &doc) {
    state = strtol((const char *)doc["state"], nullptr, 10);
    if(doc["attributes"].containsKey("unit_of_measurement")){
        strncpy(unit_of_measurement, (const char *) doc["attributes"]["unit_of_measurement"], sizeof(unit_of_measurement));
    }
    ha_entity::update(doc);
}