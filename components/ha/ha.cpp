#include <cstdio>
#include <map>

#include <ArduinoJson.h>


#include "ha.hpp"
#include "ha_entities.h"


//std::unordered_map<char *, ha_device *, cmp_str> ha_devices;
//
//std::unordered_map<char *, ha_area *, cmp_str> ha_areas;

std::map<const char *, ha_entity *, StrCompare> ha_entities;


void add_entity(const char *id, const char *entity_id) {
    ha_entity *entity = nullptr;

    char name[] = "null";
    entity = new_entity(entity_id, name);
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

