#ifndef HA_H
#define HA_H

#include <cstring>
#include <vector>
#include <unordered_map>
#include <ArduinoJson.h>

enum class ha_entity_type {
    ha_none = 0,
    ha_light = 1,
    ha_binary_sensor = 2,
    ha_switch = 3
};

class ha_entity {
public:
    ha_entity(char *entity_id, char *dname);

    virtual void update(JsonObjectConst &doc);

    virtual void toggle() {}

    virtual void on() {}

    virtual void off() {}

    virtual void dim(uint8_t) {}

    char id[50] = "";
    char name[50] = "";
    ha_entity_type type = ha_entity_type::ha_none;
    uint8_t state;

    std::vector<void (*)(void)> callbacks;

};

class ha_entity_switch : public ha_entity {
public:
    ha_entity_switch(char *entity_id, char *dname) : ha_entity(entity_id, dname) {}

    ha_entity_type type = ha_entity_type::ha_switch;

    void toggle() override;

    void update(JsonObjectConst &doc) override;
};

class ha_entity_light : public ha_entity_switch {
public:
    ha_entity_light(char *entity_id, char *dname) : ha_entity_switch(entity_id, dname) {}

    ha_entity_type type = ha_entity_type::ha_light;

    void update(JsonObjectConst &doc) override;

    void dim(uint8_t) override;

    uint8_t brightness{};
};

class ha_device {
public:
    explicit ha_device(char *name);

    ha_entity *add_entity(char *entity_id, char *name);

    void toggle();

    void off();

    void on();

    char device_name[50] = "";
    std::unordered_map<ha_entity_type, ha_entity *> entities;
};


struct ha_area {
    char name[50];
    std::unordered_map<std::string, ha_device *> devices;
};

extern std::unordered_map<std::string, ha_device *> ha_devices;

extern std::unordered_map<std::string, ha_area *> ha_areas;

extern std::unordered_map<std::string, ha_entity *> ha_entities;

void create_device(char *id, char *area_id, char *name);

void destroy_device(char *id);

void add_entity(char *id, char *entity_id);

void update_entity(const char *entity_id, JsonObjectConst &doc);

void create_area(char *id, char *name);

void destroy_area(char *id);

void print_areas();

void print_devices();

void print_entities();

#endif //HA_H
