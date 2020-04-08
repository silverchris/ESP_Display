#include <cstdio>
#include <map>

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"

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

    btn = lv_btn_create(parent, nullptr);
    lv_btn_set_toggle(btn, true);
    lvgl_callback callback_data;
    callback_data.widget = (void *)this;
    lv_obj_set_user_data(btn, callback_data);
    lv_obj_set_event_cb(btn, callback_func);

    lv_btn_set_fit2(btn, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(btn, width);

    icon = lv_label_create(btn, nullptr);
    lv_label_set_text(icon, "#FFFFFF \xEF\x83\xAB#");
    lv_label_set_recolor(icon, true);
    lv_label_set_style(icon, LV_LABEL_STYLE_MAIN, &style_symbol);
    label = lv_label_create(btn, nullptr);
    lv_label_set_text(label, txt);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_label_small);

    if (entity_ptr->state != 0) {
        lv_label_set_text(icon, "#FFFF00 \xEF\x83\xAB#");
    } else {
        lv_label_set_text(icon, "#FFFFFF \xEF\x83\xAB#");
    }
    lv_btn_set_state(btn, (entity_ptr->state != 0) ? LV_BTN_STYLE_TGL_REL : LV_BTN_STYLE_TGL_PR);

}

void lvgl_light::callback(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        if (lv_btn_get_state(btn) != LV_BTN_STATE_INA) {
            lv_btn_set_state(btn, LV_BTN_STATE_INA);
            entity_ptr->toggle();
        }
    }
}

void lvgl_light::refresh() {
    if (entity_ptr->state != 0) {
        lv_label_set_text(icon, "#FFFF00 \xEF\x83\xAB#");
    } else {
        lv_label_set_text(icon, "#FFFFFF \xEF\x83\xAB#");
    }
    lv_btn_set_state(btn, (entity_ptr->state != 0) ? LV_BTN_STYLE_TGL_REL : LV_BTN_STYLE_TGL_PR);
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
    lv_obj_set_size(cont, lv_obj_get_width_fit(parent), 75);
    lv_cont_set_layout(cont, LV_LAYOUT_ROW_M);
    lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);
    label = lv_label_create(cont, nullptr);
    lv_label_set_text(label, txt);
    lv_label_set_recolor(label, true);
    bar = lv_bar_create(cont, nullptr);
    lv_obj_set_size(bar, (lv_coord_t) lv_obj_get_width_fit(cont) - 40, 30);
    lv_bar_set_anim_time(bar, 1000);
}

void lvgl_bar_horizontal::refresh() {
    lv_bar_set_value(bar, entity_ptr->state, LV_ANIM_ON);
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
    auto width = (lv_coord_t) ((lv_obj_get_width_fit(parent) / 4) * 0.5); // TODO: fix this to be more flexible
    cont = lv_cont_create(parent, nullptr);
    lv_cont_set_layout(cont, LV_LAYOUT_COL_M);
    lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);
    printf("set layout\n");
    bar = lv_bar_create(cont, nullptr);
    lv_bar_set_value(bar, entity_ptr->state, LV_ANIM_ON);
    lv_obj_set_size(bar, width, 125);
    lv_obj_align(bar, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_anim_time(bar, 1000);
    printf("created bar\n");
    label = lv_label_create(cont, nullptr);
    lv_label_set_text(label, txt);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_symbol);
    lv_label_set_recolor(label, true);
    lv_cont_set_fit2(cont, LV_FIT_TIGHT, LV_FIT_TIGHT);
    printf("done creating widget\n");
}

void lvgl_bar_vertical::refresh() {
    lv_bar_set_value(bar, entity_ptr->state, LV_ANIM_ON);
}

class IFactory {
public:
    virtual widget_base *create(lv_obj_t *parent, ha_entity *entity, const char *label_text = nullptr,
                                lv_event_cb_t callback_func = nullptr) = 0;
};

class LightFactory : public IFactory {
    lvgl_light *create(lv_obj_t *parent, ha_entity *entity, const char *label_text = nullptr,
                       lv_event_cb_t callback_func = nullptr) {
        return new lvgl_light(parent, (ha_entity_light *) entity, label_text, callback_func);
    }
};

class TemperatureFactory : public IFactory {
    lvgl_temperature *create(lv_obj_t *parent, ha_entity *entity, const char *label_text = nullptr,
                             lv_event_cb_t callback_func = nullptr) {
        return new lvgl_temperature(parent, (ha_entity_weather *) entity, label_text);
    }
};

class BarHorizontalFactory : public IFactory {
    lvgl_bar_horizontal *create(lv_obj_t *parent, ha_entity *entity, const char *label_text = nullptr,
                                lv_event_cb_t callback_func = nullptr) {
        return new lvgl_bar_horizontal(parent, (ha_entity_sensor *) entity, label_text);
    }
};

class BarVerticalFactory : public IFactory {
    lvgl_bar_vertical *create(lv_obj_t *parent, ha_entity *entity, const char *label_text = nullptr,
                              lv_event_cb_t callback_func = nullptr) {
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