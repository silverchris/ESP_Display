#include <cstdio>
#include "lvgl/lvgl.h"

#include "ha.hpp"

#include "gui.h"

#include "widgets.h"




lvgl_light::lvgl_light(lv_obj_t *parent, ha_entity_light *entity, lv_event_cb_t callback_func) {
    entity_ptr = entity;
    entity->callbacks.push_back(this);

    auto width = (lv_coord_t) ((lv_obj_get_width_fit(parent) / 2) - 20);

    btn = lv_btn_create(parent, nullptr);
    lv_btn_set_toggle(btn, true);
    lv_obj_set_event_cb(btn, callback_func);
    lv_btn_set_fit2(btn, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(btn, width);

    icon = lv_label_create(btn, nullptr);
    lv_label_set_text(icon, "#FFFFFF \xEF\x83\xAB#");
    lv_label_set_recolor(icon, true);
    lv_label_set_style(icon, LV_LABEL_STYLE_MAIN, &style_symbol);
    label = lv_label_create(btn, nullptr);
    lv_label_set_text(label, entity->name);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_label_small);
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


lvgl_temperature::lvgl_temperature(lv_obj_t *parent, ha_entity_weather *entity){
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