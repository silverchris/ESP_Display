//#include <vector>
//#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>

#include <ArduinoJson.h>


#include "ha.hpp"
#include "ha_entities.h"


//std::unordered_map<char *, ha_device *, cmp_str> ha_devices;
//
//std::unordered_map<char *, ha_area *, cmp_str> ha_areas;

std::map<const char *, ha_entity *, StrCompare> ha_entities;





//ha_device::ha_device(char *name) {
//    strncpy(device_name, name, 50);
//}
//
//ha_entity *ha_device::add_entity(char *entity_id, char *name) {
//    ha_entity *entity = nullptr;
//    if (strncmp(entity_id, "light.", 5) == 0) {
//        entity = new ha_entity_light(entity_id, name);
//    } else if (strncmp(entity_id, "switch.", 7) == 0) {
//        entity = new ha_entity_switch(entity_id, name);
//    } else if (strncmp(entity_id, "sensor.", 7) == 0) {
//        entity = new ha_entity_sensor(entity_id, name);
//    }
////    else if (strncmp(entity_id, "binary_sensor.", 14) == 0) {
////        entity = new ha_entity_light(entity_id);
////    }
//    if (entity != nullptr) {
//        entities.emplace(entity->type, entity);
//    }
//    return entity;
//}
//
//void ha_device::toggle() {
//
//}
//
//void ha_device::off() {
//
//}
//
//void ha_device::on() {
//
//};
//
//void create_device(char *id, char *area_id, char *name) {
//    auto *device = new ha_device(name);
//    ha_devices.emplace(id, device);
//    if (ha_areas.count(area_id)) {
//        ha_areas[area_id]->devices.emplace(id, device);
//    }
//}
//
//void destroy_device(char *id) {
//    for (const std::pair<const std::string, ha_area *> &element : ha_areas) {
//        if (element.second->devices.count(id)) {
//            element.second->devices.erase(id);
//        }
//    }
//    if (!ha_devices[id]->entities.empty()) {
//        for (const std::pair<const ha_entity_type, ha_entity *> &element : ha_devices[id]->entities) {
//            free(element.second);
//            ha_devices[id]->entities.erase(element.first);
//        }
//    }
//    free(ha_devices[id]);
//    ha_devices.erase(id);
//}

void add_entity(const char *id, const char *entity_id) {
    ha_entity *entity = nullptr;

//    if (ha_devices.count(id)) {
//        entity = ha_devices[id]->add_entity(entity_id, ha_devices[id]->device_name);
//    } else {
        char name[] = "null";
        if (strncmp(entity_id, "light.", 5) == 0) {
            entity = new ha_entity_light(entity_id, name);
        } else if (strncmp(entity_id, "switch.", 7) == 0) {
            entity = new ha_entity_switch(entity_id, name);
        } else if (strncmp(entity_id, "weather.", 8) == 0) {
            entity = new ha_entity_weather(entity_id, name);
        } else if (strncmp(entity_id, "sensor.", 7) == 0) {
            entity = new ha_entity_sensor(entity_id, name);
        }
//    }
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

ha_entity *get_entity(const char *entity_id) {
    printf("getting: %s, %i\n", entity_id, ha_entities.count(entity_id));
    if (ha_entities.count(entity_id)) {
        ha_entity *entity = nullptr;
        entity = ha_entities[entity_id];
        printf("entity ptr: %p\n", entity);
        return entity;
    }
    return nullptr;
}

//void create_area(char *id, char *name) {
//    auto *area = new ha_area;
//    strncpy(area->name, name, 50);
//    ha_areas.emplace(id, area);
//}
//
//void destroy_area(char *id) {
//    if (ha_areas[id]->devices.empty()) {
//        free(ha_areas[id]);
//        ha_areas.erase(id);
//    }
//}

