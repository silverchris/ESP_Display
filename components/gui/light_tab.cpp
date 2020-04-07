#include <cstdio>
#include <vector>
#include <functional>
#include "lvgl/lvgl.h"

#include "ha.hpp"

#include "gui.h"

#include "widgets.h"

std::unordered_map<lv_obj_t *, lvgl_light *> lights;

void callback_func(lv_obj_t *obj, lv_event_t event) {
    if (lights.count(obj)) {
        lights[obj]->callback(obj, event);
    }
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
            auto *light = new lvgl_light(cont, (ha_entity_light *)entity.second, &callback_func);
            lights.emplace(light->btn, light);
        }
    }
}
