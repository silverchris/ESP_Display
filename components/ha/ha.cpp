#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unordered_map>

#include <ArduinoJson.h>


#include "ha.hpp"
#include "ha_event.h"
#include "websocket.h"


std::unordered_map<std::string, ha_device *> ha_devices;

std::unordered_map<std::string, ha_area *> ha_areas;

std::unordered_map<std::string, ha_entity *> ha_entities;

ha_entity::ha_entity(char *entity_id, char *dname) {
    strncpy(id, entity_id, sizeof(id));
    strncpy(name, dname, sizeof(name));
    printf("Creating entity: %s\n", id);

    if (strncmp(entity_id, "light.", 5) == 0) {
        type = ha_entity_type::ha_light;
    } else if (strncmp(entity_id, "switch.", 7) == 0) {
        type = ha_entity_type::ha_switch;
    } else if (strncmp(entity_id, "binary_sensor.", 14) == 0) {
        type = ha_entity_type::ha_binary_sensor;
    } else if (strncmp(entity_id, "weather.", 8) == 0) {
        type = ha_entity_type::ha_weather;
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

void ha_entity_light::dim(uint8_t) {

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

ha_device::ha_device(char *name) {
    strncpy(device_name, name, 50);
}

ha_entity *ha_device::add_entity(char *entity_id, char *name) {
    ha_entity *entity = nullptr;
    if (strncmp(entity_id, "light.", 5) == 0) {
        entity = new ha_entity_light(entity_id, name);
    } else if (strncmp(entity_id, "switch.", 7) == 0) {
        entity = new ha_entity_switch(entity_id, name);
    }
//    else if (strncmp(entity_id, "binary_sensor.", 14) == 0) {
//        entity = new ha_entity_light(entity_id);
//    }
    if (entity != nullptr) {
        entities.emplace(entity->type, entity);
    }
    return entity;
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


void ha_device::toggle() {

}

void ha_device::off() {

}

void ha_device::on() {

};

void create_device(char *id, char *area_id, char *name) {
    auto *device = new ha_device(name);
    ha_devices.emplace(id, device);
    if (ha_areas.count(area_id)) {
        ha_areas[area_id]->devices.emplace(id, device);
    }
}

void destroy_device(char *id) {
    for (const std::pair<const std::string, ha_area *> &element : ha_areas) {
        if (element.second->devices.count(id)) {
            element.second->devices.erase(id);
        }
    }
    if (!ha_devices[id]->entities.empty()) {
        for (const std::pair<const ha_entity_type, ha_entity *> &element : ha_devices[id]->entities) {
            free(element.second);
            ha_devices[id]->entities.erase(element.first);
        }
    }
    free(ha_devices[id]);
    ha_devices.erase(id);
}

void add_entity(char *id, char *entity_id) {
    ha_entity *entity = nullptr;

    if (ha_devices.count(id)) {
        entity = ha_devices[id]->add_entity(entity_id, ha_devices[id]->device_name);
    } else {
        char name[] = "null";
        if (strncmp(entity_id, "light.", 5) == 0) {
            entity = new ha_entity_light(entity_id, name);
        } else if (strncmp(entity_id, "switch.", 7) == 0) {
            entity = new ha_entity_switch(entity_id, name);
        } else if (strncmp(entity_id, "weather.", 8) == 0) {
            entity = new ha_entity_weather(entity_id, name);
        }
    }
    if (entity != nullptr) {
        ha_entities.emplace(entity_id, entity);
    }
}

void update_entity(const char *entity_id, JsonObjectConst &doc) {
    printf("updating: %s, %i\n", entity_id, ha_entities.count(entity_id));
    if (ha_entities.count(entity_id)) {
        ha_entities[entity_id]->update(doc);
        printf("%s %i\n", entity_id, ha_entities[entity_id]->state);
    }
}

void create_area(char *id, char *name) {
    auto *area = new ha_area;
    strncpy(area->name, name, 50);
    ha_areas.emplace(id, area);
}

void destroy_area(char *id) {
    if (ha_areas[id]->devices.empty()) {
        free(ha_areas[id]);
        ha_areas.erase(id);
    }
}


void print_areas() {
//    for (const std::pair<const std::string, ha_area *> &element : ha_areas) {
//        printf("id: %s, name: %s\n", element.first.c_str(), element.second->name);
//        for (const std::pair<const std::string, ha_device *> &device : element.second->devices) {
//            printf("\tdevice: %s\n", device.second->device_name);
//            for (const std::pair<const ha_entity_type, ha_entity *> &entity :device.second->entities) {
//                printf("\t\tentity: %s %u\n", entity.second->id, entity.second->type);
//            }
//        }
//    }
}

void print_devices() {
//    for (const std::pair<const std::string, ha_device *> &device : ha_devices) {
//        printf("\tdevice: %s\n", device.second->device_name);
//        for (const std::pair<const ha_entity_type, ha_entity *> &entity :device.second->entities) {
//            printf("\t\tentity: %s %u\n", entity.second->id, entity.second->type);
//        }
//    }
}

void print_entities() {
//    for (const std::pair<const std::string, ha_entity *> &entity : ha_entities) {
//        printf("\t\tentity: %s %u\n", entity.second->id, entity.second->type);
//    }
}

