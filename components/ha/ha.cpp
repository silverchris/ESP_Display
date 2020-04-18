#include <cstdio>
#include <map>

#include <ArduinoJson.h>


#include "ha.hpp"
#include "ha_entities.h"
#include "ha_devices.h"
#include "ha_areas.h"


std::map<const char *, ha_device *, StrCompare> ha_devices;

//std::map<const char *, ha_area *, StrCompare> ha_areas;

std::map<const char *, ha_entity *, StrCompare> ha_entities;


ha_entity *add_entity(const char *entity_id) {
    ha_entity *entity = nullptr;

    char name[] = "null";
    size_t id_len = 50;
    char *id = (char *) calloc(sizeof(char), id_len);
    strncpy(id, entity_id, id_len-1);

    entity = new_entity(id, name);
    if (entity != nullptr) {
        ha_entities.emplace(id, entity);
    } else {
        free(id);
    }
    return entity;
}

void update_entity(const char *entity_id, JsonObjectConst &doc) {
    if (ha_entities.count(entity_id)) {
        ha_entities[entity_id]->update(doc);
    }
}

ha_entity *get_entity(const char *entity_id) {
    if (ha_entities.count(entity_id)) {
        ha_entity *entity = nullptr;
        entity = ha_entities[entity_id];
        return entity;
    }
    return nullptr;
}

void add_device(const char *id, const char *name, const char *area) {
    ha_device *device = nullptr;

    device = new_device(name, area);
    if (device != nullptr) {
        ha_devices.emplace(id, device);
    }
}


ha_device *get_device(const char *device_id){
    if (ha_devices.count(device_id)) {
        ha_device *device = nullptr;
        device = ha_devices[device_id];
        return device;
    }
    return nullptr;
}

void device_entity_assoc(const char * deviceid, const char *entityid){
    if(ha_entities.count(entityid) > 0 && ha_devices.count(deviceid)){
        ha_devices[deviceid]->add_entity(ha_entities[entityid]);
    }
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

