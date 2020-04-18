#ifndef ESP_DISPLAY_HA_ENTITIES_H
#define ESP_DISPLAY_HA_ENTITIES_H

#include <vector>
#include <ArduinoJson.h>

int get_state(const char *entity, char *out);

enum class ha_entity_type {
    ha_none = 0,
    ha_light = 1,
    ha_binary_sensor = 2,
    ha_switch = 3,
    ha_weather = 4,
    ha_sensor = 5
};

class ha_entity {
protected:
    unsigned int supported_features = 0;
public:
    ha_entity(const char *entity_id, const char *dname);

    virtual void update(JsonObjectConst &doc);

    virtual void features(void *feature_struct) {};

    virtual int getState() { return 0; };

    char id[50] = "";
    char name[50] = "";
    ha_entity_type type = ha_entity_type::ha_none;

    std::vector<void *> callbacks;


};

class ha_entity_switch : public ha_entity {
private:
    bool state;
public:
    ha_entity_switch(const char *entity_id, const char *dname) : ha_entity(entity_id, dname) {}

    ha_entity_type type = ha_entity_type::ha_switch;

    void toggle();

    void update(JsonObjectConst &doc) override;

    int getState();
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
private:
    bool state;
public:
    ha_entity_light(const char *entity_id, const char *dname) : ha_entity(entity_id, dname) {}

    ha_entity_type type = ha_entity_type::ha_light;

    void update(JsonObjectConst &doc) override;

    void dim(uint8_t);

    void toggle();

    void features(void *feature_struct) override;

    int getState();

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

class ha_entity_weather : public ha_entity {


public:
    ha_entity_type type = ha_entity_type::ha_weather;

    ha_entity_weather(const char *entity_id, const char *dname) : ha_entity(entity_id, dname) {}


    float temperature = 0;
    float pressure = 0;
    float humidity = 0;
    float visibility = 0;
    float wind_speed = 0;
    float wind_bearing = 0;
    char friendly_name[50] = "";

    void update(JsonObjectConst &doc) override;

    int getState();

};

class ha_entity_sensor : public ha_entity {

    char state[20];

public:
    ha_entity_type type = ha_entity_type::ha_sensor;

    char unit_of_measurement[10] = "";

    ha_entity_sensor(const char *entity_id, const char *dname) : ha_entity(entity_id, dname) {}

    void update(JsonObjectConst &doc) override;

    int getState();

    float getStateAsFloat();

    char *getStateAsString();
};


class EntityFactory {
public:
    virtual ha_entity *create(const char *entity_id, const char *dname) = 0;
};

ha_entity *new_entity(const char *entity_id, const char *dname);


#endif //ESP_DISPLAY_HA_ENTITIES_H
