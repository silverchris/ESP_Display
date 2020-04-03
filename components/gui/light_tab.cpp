#include <cstdio>
#include <cstring>
#include <vector>
#include <functional>
#include "lvgl/lvgl.h"

#include "ha.hpp"

#include "gui.h"

std::unordered_map<lv_obj_t *, lvgl_light *> lights;

void callback_func(lv_obj_t *obj, lv_event_t event) {
    if (lights.count(obj)) {
        lights[obj]->callback(obj, event);
    }
}

void refresh_func() {
    for (const std::pair<lv_obj_t *, lvgl_light *> &light : lights) {
        light.second->refresh();
    }
}

lvgl_light::lvgl_light(lv_obj_t *parent, class ha_entity *entity) {
    entity_ptr = entity;
    entity->callbacks.push_back(refresh_func);

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

    this->refresh();
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


void light_tab(lv_obj_t *parent) {
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
    lv_page_set_scrl_layout(parent, LV_LAYOUT_CENTER);

    lv_obj_t *cont = lv_cont_create(parent, nullptr);
    lv_cont_set_layout(cont, LV_LAYOUT_PRETTY);
    lv_cont_set_fit2(cont, LV_FIT_FLOOD, LV_FIT_FLOOD);

    for (const std::pair<const std::string, ha_entity *> &entity : ha_entities) {
        if (entity.second->type == ha_entity_type::ha_light) {
            printf("light: %s\n", entity.second->id);
            auto *light = new lvgl_light(cont, entity.second);
            lights.emplace(light->btn, light);
        }
    }
}
