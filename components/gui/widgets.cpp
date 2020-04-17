#include <cstdio>
#include <map>

#include "freertos/FreeRTOS.h"

#include "esp_timer.h"

#include "lvgl/lvgl.h"

#include "ha_entities.h"

#include "gui.h"

#include "ha.hpp"


#include "widgets.h"


lvgl_light::lvgl_light(lv_obj_t *parent, ha_entity_light *entity, const char *label_text, lv_event_cb_t callback_func) {
    entity_ptr = entity;
    entity->callbacks.push_back(this);
    const char *txt;
    if (label_text == nullptr) {
        txt = entity->name;
    } else {
        txt = label_text;
    }

    auto width = (lv_coord_t) ((lv_obj_get_width_fit(parent) / 2) - 20);

    lv_obj_t *cont = lv_cont_create(parent, nullptr);
    lv_cont_set_layout(cont, LV_LAYOUT_COL_M);
    lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);

    btn = lv_btn_create(cont, nullptr);
    lv_btn_set_toggle(btn, true);
    lvgl_callback callback_data{};
    callback_data.widget = (void *) this;
    lv_obj_set_user_data(btn, callback_data);
    lv_obj_set_event_cb(btn, callback_func);

//    lv_btn_set_fit2(btn, LV_FIT_FILL, LV_FIT_FILL);
    lv_obj_set_width(btn, lv_obj_get_width_fit(cont));
    lv_obj_set_height(btn, lv_obj_get_height_fit(cont));

    icon = lv_label_create(btn, nullptr);
    lv_label_set_text(icon, "#FFFFFF \xEF\x83\xAB#");
    lv_label_set_recolor(icon, true);
//    lv_label_set_style(icon, LV_LABEL_STYLE_MAIN, &style_symbol);
    label = lv_label_create(btn, nullptr);
    lv_label_set_text(label, txt);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_label_small);

    if (entity_ptr->getState() != 0) {
        lv_label_set_text(icon, "#FFFF00 \xEF\x83\xAB#");
    } else {
        lv_label_set_text(icon, "#FFFFFF \xEF\x83\xAB#");
    }
    lv_btn_set_state(btn, (entity_ptr->getState() != 0) ? LV_BTN_STYLE_TGL_REL : LV_BTN_STYLE_TGL_PR);

    dim_direction = false;
    press_time = 0;

}

void lvgl_light::callback(__unused lv_obj_t *obj, lv_event_t event) {
    uint32_t press = (uint32_t) esp_timer_get_time() / 1000;
    if (event == LV_EVENT_SHORT_CLICKED) {
        if (lv_btn_get_state(btn) != LV_BTN_STATE_INA) {
            lv_btn_set_state(btn, LV_BTN_STATE_INA);
            entity_ptr->toggle();
        }
    } else if (event == LV_EVENT_PRESSING) {
        if (press_time == 0) {
            press_time = press;
        }
        if (press - press_time >= 500) {
            int dim = entity_ptr->brightness;
            if (dim_direction) {
                dim += 10;
            } else {
                dim -= 10;
            }
            if (dim < 0 || dim > 254) {
                dim_direction = !dim_direction;
                if (dim < 0) {
                    dim = 0;
                } else {
                    dim = 254;
                }
            }
            entity_ptr->dim((uint8_t) dim);

            press_time = press;
        }
    } else if (event == LV_EVENT_RELEASED) {
        press_time = 0;

    }
}

void lvgl_light::refresh() {
    if (entity_ptr->getState() != 0) {
        lv_label_set_text(icon, "#FFFF00 \xEF\x83\xAB#");
    } else {
        lv_label_set_text(icon, "#FFFFFF \xEF\x83\xAB#");
    }
    lv_btn_set_state(btn, (entity_ptr->getState() != 0) ? LV_BTN_STYLE_TGL_REL : LV_BTN_STYLE_TGL_PR);
}


lvgl_temperature::lvgl_temperature(lv_obj_t *parent, ha_entity_weather *entity, const char *label_text) {
    entity_ptr = entity;
    entity->callbacks.push_back(this);

    cont = lv_cont_create(parent, nullptr);
    lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);
    lv_cont_set_layout(cont, LV_LAYOUT_COL_M);
    label = lv_label_create(cont, nullptr);
    lv_label_set_text(label, entity->friendly_name);
    temp = lv_label_create(cont, nullptr);
    char temperature[4];
    sprintf(temperature, "%2.1f", entity->temperature);
    lv_label_set_text(temp, temperature);
    lv_label_set_style(temp, LV_LABEL_STYLE_MAIN, &style_temperature);
}

void lvgl_temperature::refresh() {
    lv_label_set_text(label, entity_ptr->friendly_name);
    char temperature[4];
    sprintf(temperature, "%2.1f", entity_ptr->temperature);
    lv_label_set_text(temp, temperature);
}

lvgl_bar_horizontal::lvgl_bar_horizontal(lv_obj_t *parent, ha_entity_sensor *entity, const char *label_text) {
    entity_ptr = entity;
    entity->callbacks.push_back(this);
    const char *txt;
    if (label_text == nullptr) {
        txt = entity->name;
    } else {
        txt = label_text;
    }
    cont = lv_cont_create(parent, nullptr);
    lv_obj_set_size(cont, static_cast<lv_coord_t>(lv_obj_get_width_fit(parent) - 12), 75);
    lv_cont_set_layout(cont, LV_LAYOUT_ROW_M);
    lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);

    label = lv_label_create(cont, nullptr);
    lv_label_set_text(label, txt);
    lv_label_set_recolor(label, true);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_bar_label);

    bar = lv_bar_create(cont, nullptr);
    lv_bar_set_value(bar, (int16_t) entity_ptr->getState(), LV_ANIM_ON);
    lv_obj_set_size(bar, lv_obj_get_width_fit(cont) - lv_obj_get_width(label), 25);
    lv_bar_set_anim_time(bar, 1000);
}

void lvgl_bar_horizontal::refresh() {
    lv_bar_set_value(bar, (int16_t) entity_ptr->getState(), LV_ANIM_ON);
}

lvgl_bar_vertical::lvgl_bar_vertical(lv_obj_t *parent, ha_entity_sensor *entity, const char *label_text) {
    entity_ptr = entity;
    entity->callbacks.push_back(this);
    const char *txt;
    if (label_text == nullptr) {
        txt = entity->name;
    } else {
        txt = label_text;
    }

    cont = lv_cont_create(parent, nullptr);
    lv_cont_set_layout(cont, LV_LAYOUT_COL_M);
    lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);

    auto width = (lv_coord_t) (lv_obj_get_width_fit(cont)* 0.3); // TODO: fix this to be more flexible

    bar = lv_bar_create(cont, nullptr);
    lv_bar_set_value(bar, (int16_t) entity_ptr->getState(), LV_ANIM_ON);
    lv_obj_set_size(bar, width, 125);
    lv_obj_align(bar, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_anim_time(bar, 1000);

    label = lv_label_create(cont, nullptr);
    lv_label_set_text(label, txt);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_bar_label);
    lv_label_set_recolor(label, true);
    lv_cont_set_fit2(cont, LV_FIT_TIGHT, LV_FIT_TIGHT);
}

void lvgl_bar_vertical::refresh() {
    lv_bar_set_value(bar, (int16_t) entity_ptr->getState(), LV_ANIM_ON);
}

class IFactory {
public:
    virtual widget_base *create(lv_obj_t *parent, ha_entity *entity, const char *label_text,
                                lv_event_cb_t callback_func) = 0;
};

class LightFactory : public IFactory {
    lvgl_light *create(lv_obj_t *parent, ha_entity *entity, const char *label_text,
                       lv_event_cb_t callback_func) override {
        return new lvgl_light(parent, (ha_entity_light *) entity, label_text, callback_func);
    }
};

class TemperatureFactory : public IFactory {
    lvgl_temperature *create(lv_obj_t *parent, ha_entity *entity, const char *label_text,
                             lv_event_cb_t callback_func) override {
        return new lvgl_temperature(parent, (ha_entity_weather *) entity, label_text);
    }
};

class BarHorizontalFactory : public IFactory {
    lvgl_bar_horizontal *create(lv_obj_t *parent, ha_entity *entity, const char *label_text,
                                lv_event_cb_t callback_func) override {
        return new lvgl_bar_horizontal(parent, (ha_entity_sensor *) entity, label_text);
    }
};

class BarVerticalFactory : public IFactory {
    lvgl_bar_vertical *
    create(lv_obj_t *parent, ha_entity *entity, const char *label_text, lv_event_cb_t callback_func) override {
        return new lvgl_bar_vertical(parent, (ha_entity_sensor *) entity, label_text);
    }
};

typedef std::map<const char *, IFactory *, StrCompare> WidgetsByName;

widget_base *get_widget_by_type(const char *widget_name, lv_obj_t *parent, ha_entity *entity, const char *label_text,
                                lv_event_cb_t callback_func) {
    WidgetsByName widgets_by_name;
    widgets_by_name["light"] = new LightFactory();
    widgets_by_name["temperature"] = new TemperatureFactory();
    widgets_by_name["bar_horizontal"] = new BarHorizontalFactory();
    widgets_by_name["bar_vertical"] = new BarVerticalFactory();

//    std::string widget_string(widget_name);

    if (widgets_by_name.count(widget_name) > 0) {
        IFactory *factory = widgets_by_name[widget_name];
        return factory->create(parent, entity, label_text, callback_func);
    }
    return nullptr;
}