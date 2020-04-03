#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unordered_map>

#include <ArduinoJson.h>


#include "ha.hpp"
#include "websocket.h"


std::unordered_map<std::string, ha_device *> ha_devices;

std::unordered_map<std::string, ha_area *> ha_areas;

std::unordered_map<std::string, ha_entity *> ha_entities;

ha_entity::ha_entity(char *entity_id, char *dname) {
    strncpy(id, entity_id, 40);
    strncpy(name, dname, 50);

    if (strncmp(entity_id, "light.", 5) == 0) {
        type = ha_entity_type::ha_light;
    } else if (strncmp(entity_id, "switch.", 7) == 0) {
        type = ha_entity_type::ha_switch;
    } else if (strncmp(entity_id, "binary_sensor.", 14) == 0) {
        type = ha_entity_type::ha_binary_sensor;
    }
    state = 0;
}

void ha_entity::update(JsonObjectConst &doc) {
    for (auto &call : callbacks) {
        call();
    }
}

void ha_entity_switch::toggle() {
    StaticJsonDocument<300> doc_out;
    doc_out["type"] = "call_service";
    doc_out["domain"] = "light";
    doc_out["service"] = "toggle";
    doc_out["service_data"]["entity_id"] = id;
    ws_queue_add(doc_out);
}

void ha_entity_switch::update(JsonObjectConst &doc) {
    state = static_cast<uint8_t>(strncmp(doc["state"], "on", 2) == 0);
    ha_entity::update(doc);
}

void ha_entity_light::dim(uint8_t) {

}

void ha_entity_light::update(JsonObjectConst &doc) {
    if (doc["attributes"].containsKey("brightness")) {
        brightness = doc["attributes"]["brightness"];
    }
    ha_entity_switch::update(doc);
}

ha_device::ha_device(char *name) {
    strncpy(device_name, name, 50);
}

ha_entity *ha_device::add_entity(char *entity_id, char *name) {
    ha_entity *entity;
    if (strncmp(entity_id, "light.", 5) == 0) {
        entity = new ha_entity_light(entity_id, name);
    } else if (strncmp(entity_id, "switch.", 7) == 0) {
        entity = new ha_entity_switch(entity_id, name);
    } else {
        entity = new ha_entity(entity_id, name);
    }
//    else if (strncmp(entity_id, "binary_sensor.", 14) == 0) {
//        entity = new ha_entity_light(entity_id);
//    }
    entities.emplace(entity->type, entity);
    ha_entities.emplace(entity_id, entity);
    return entity;
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
    if (ha_devices.count(id)) {
        ha_entity *entity = ha_devices[id]->add_entity(entity_id, ha_devices[id]->device_name);
        ha_entities.emplace(entity_id, entity);
    }
}

void update_entity(const char *entity_id, JsonObjectConst &doc) {
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

