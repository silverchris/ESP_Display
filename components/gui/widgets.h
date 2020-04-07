#ifndef ESP_DISPLAY_WIDGETS_H
#define ESP_DISPLAY_WIDGETS_H

#ifdef __cplusplus

#include "ha.hpp"

class widget_base {
public:
    virtual void refresh() = 0;
};

class lvgl_light : public widget_base {
public:
    lvgl_light(lv_obj_t *parent, ha_entity_light *entity, lv_event_cb_t callback_func);

    lv_obj_t *label;
    lv_obj_t *icon;
    lv_obj_t *btn;
    ha_entity_light *entity_ptr;

    void callback(lv_obj_t *obj, lv_event_t event);

    void refresh() override;
};

class lvgl_temperature : public widget_base {
protected:
    lv_obj_t *cont;
    lv_obj_t *label;
    lv_obj_t *temp;
    ha_entity_weather *entity_ptr;

public:
    lvgl_temperature(lv_obj_t *parent, ha_entity_weather *entity);

    void refresh() override;
};

#endif

#endif //ESP_DISPLAY_WIDGETS_H
