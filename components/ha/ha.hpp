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
    ha_switch = 3,
    ha_weather = 4
};

class ha_entity {
protected:
    unsigned int supported_features = 0;
public:
    ha_entity(char *entity_id, char *dname);

    virtual void update(JsonObjectConst &doc);

    virtual void features(void *feature_struct) {};

    char id[50] = "";
    char name[50] = "";
    ha_entity_type type = ha_entity_type::ha_none;
    unsigned int state;

//    std::vector<void (*)(void)> callbacks;
    std::vector<void *> callbacks;


};

class ha_entity_switch : public ha_entity {
public:
    ha_entity_switch(char *entity_id, char *dname) : ha_entity(entity_id, dname) {}

    ha_entity_type type = ha_entity_type::ha_switch;

    void toggle();

    void update(JsonObjectConst &doc) override;
};

/* ha_entity_light definitions and structures */

enum {
    HA_LIGHT_SUPPORT_BRIGHTNESS = 1,
    HA_LIGHT_SUPPORT_COLOR_TEMP = 2,
    HA_LIGHT_SUPPORT_EFFECT = 4,
    HA_LIGHT_SUPPORT_FLASH = 8,
    HA_LIGHT_SUPPORT_COLOR = 16,
    HA_LIGHT_SUPPORT_TRANSITION = 32,
    HA_LIGHT_SUPPORT_WHITE_VALUE = 128
};

struct ha_light_features {
    bool SUPPORT_BRIGHTNESS;
    bool SUPPORT_COLOR;
    bool SUPPORT_COLOR_TEMP;
    bool SUPPORT_EFFECT;
    bool SUPPORT_FLASH;
    bool SUPPORT_TRANSITION;
    bool SUPPORT_WHITE_VALUE;
};

class ha_entity_light : public ha_entity {
public:
    ha_entity_light(char *entity_id, char *dname) : ha_entity(entity_id, dname) {}

    ha_entity_type type = ha_entity_type::ha_light;

    void update(JsonObjectConst &doc) override;

    void dim(uint8_t);

    void toggle();

    void features(void * feature_struct) override ;

    int brightness = 0;
    int color_temp = 0;
    float hs_color[2] = {0, 0};
    int max_minreds = 0;
    int min_mireds = 0;
    int white_value = 0;
};


/* ha_entity_weather */
enum ha_weather_states {
    HA_WEATHER_CLEAR_NIGHT,
    HA_WEATHER_CLOUDY,
    HA_WEATHER_EXCEPTIONAL,
    HA_WEATHER_FOG,
    HA_WEATHER_HAIL,
    HA_WEATHER_LIGHTNING,
    HA_WEATHER_LIGHTNING_RAINY,
    HA_WEATHER_PARTLYCLOUDY,
    HA_WEATHER_POURING,
    HA_WEATHER_RAINY,
    HA_WEATHER_SNOWY,
    HA_WEATHER_SNOWY_RAINY,
    HA_WEATHER_SUNNY,
    HA_WEATHER_WINDY,
    HA_WEATHER_WINDY_VARIANT
};

class ha_entity_weather: public ha_entity {


public:
    ha_entity_type type = ha_entity_type::ha_weather;

    ha_entity_weather(char *entity_id, char *dname) : ha_entity(entity_id, dname) {}


    float temperature = 0;
    float pressure = 0;
    float humidity = 0;
    float visibility = 0;
    float wind_speed = 0;
    float wind_bearing = 0;
    char friendly_name[50] = "";

    void update(JsonObjectConst &doc) override;

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
