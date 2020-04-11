#ifndef HA_H
#define HA_H

#include <cstring>
#include <vector>
#include <functional>
#include <map>
#include <ArduinoJson.h>

#include "ha_entities.h"
#include "ha_devices.h"

struct StrCompare : public std::binary_function<const char *, const char *, bool> {
public:
    bool operator()(const char *str1, const char *str2) const { return std::strcmp(str1, str2) < 0; }
};


void add_entity(const char *id, const char *entity_id);

void update_entity(const char *entity_id, JsonObjectConst &doc);

ha_entity *get_entity(const char *entity_id);

void add_device(const char *id, const char *name, const char *area);

ha_device *get_device(const char *device_id);

void device_entity_assoc(const char * deviceid, const char *entityid);

//void create_area(char *id, char *name);
//
//void destroy_area(char *id);
//
//void print_areas();
//
//void print_devices();
//
//void print_entities();

#endif //HA_H
