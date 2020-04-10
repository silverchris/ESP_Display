#ifndef ESP_DISPLAY_WIDGETS_H
#define ESP_DISPLAY_WIDGETS_H

#ifdef __cplusplus

#include "ha.hpp"
#include "ha_entities.h"

class widget_base {
public:
    virtual void refresh() {};

    virtual void callback(lv_obj_t *obj, lv_event_t event) {};

};

class lvgl_light : public widget_base {
public:
    lvgl_light(lv_obj_t *parent, ha_entity_light *entity, const char *label_text, lv_event_cb_t callback_func);

    lv_obj_t *label;
    lv_obj_t *icon;
    lv_obj_t *btn;
    ha_entity_light *entity_ptr;
    uint32_t press_time;
    bool dim_direction; // false for down, true for up

    void callback(lv_obj_t *obj, lv_event_t event) override;


    void refresh() override;
};

class lvgl_temperature : public widget_base {
protected:
    lv_obj_t *cont;
    lv_obj_t *label;
    lv_obj_t *temp;
    ha_entity_weather *entity_ptr;

public:
    lvgl_temperature(lv_obj_t *parent, ha_entity_weather *entity, const char *label_text);

    void refresh() override;
};

class lvgl_bar_horizontal : public widget_base {
    ha_entity_sensor *entity_ptr;

    lv_obj_t *cont;
    lv_obj_t *bar;
    lv_obj_t *label;
public:
    lvgl_bar_horizontal(lv_obj_t *parent, ha_entity_sensor *entity, const char *label_text);

    void refresh() override;

};

class lvgl_bar_vertical : public widget_base {
    ha_entity_sensor *entity_ptr;

    lv_obj_t *cont;
    lv_obj_t *bar;
    lv_obj_t *label;

public:
    lvgl_bar_vertical(lv_obj_t *parent, ha_entity_sensor *entity, const char *label_text);

    void refresh() override;

};

widget_base *
get_widget_by_type(const char *widget_name, lv_obj_t *parent, ha_entity *entity, const char *label_text = nullptr,
                   lv_event_cb_t callback_func = nullptr);

#endif

#endif //ESP_DISPLAY_WIDGETS_H
