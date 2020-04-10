#ifndef HA_H
#define HA_H

#include <cstring>
#include <vector>
#include <functional>
#include <map>
#include <ArduinoJson.h>

#include "ha_entities.h"

struct StrCompare : public std::binary_function<const char *, const char *, bool> {
public:
    bool operator()(const char *str1, const char *str2) const { return std::strcmp(str1, str2) < 0; }
};


//class ha_device {
//public:
//    explicit ha_device(char *name);
//
//    ha_entity *add_entity(char *entity_id, char *name);
//
//    void toggle();
//
//    void off();
//
//    void on();
//
//    char device_name[50] = "";
//    std::unordered_map<ha_entity_type, ha_entity *> entities;
//};


//struct ha_area {
//    char name[50];
//    std::unordered_map<std::string, ha_device *> devices;
//};

//extern std::unordered_map<std::string, ha_device *> ha_devices;
//
//extern std::unordered_map<std::string, ha_area *> ha_areas;

//extern std::unordered_map<char *, ha_entity *, cmp_str> ha_entities;

//void create_device(char *id, char *area_id, char *name);
//
//void destroy_device(char *id);

void add_entity(const char *id, const char *entity_id);

void update_entity(const char *entity_id, JsonObjectConst &doc);

ha_entity *get_entity(const char *entity_id);

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
