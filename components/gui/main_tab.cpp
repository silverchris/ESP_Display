//#include "lvgl/lvgl.h"
//#include "gui.h"
//#include "widgets.h"
//
//#define TANKS 4
//const char *tank_names[TANKS] = {"#0000FF \xEF\x81\x83#", "\xEF\x8B\x8D", "\xEF\x9F\x98", "\xEF\x91\xAA"};
//lv_obj_t *tank_ptrs[TANKS];
//lv_obj_t *bat_bar;
//
//void create_tanks(lv_obj_t *parent) {
//    lv_obj_t *tank_cont = lv_cont_create(parent, nullptr);
//    lv_cont_set_layout(tank_cont, LV_LAYOUT_PRETTY);
//    lv_cont_set_style(tank_cont, LV_CONT_STYLE_MAIN, &lv_style_transp_tight);
//    lv_cont_set_fit2(tank_cont, LV_FIT_FLOOD, LV_FIT_TIGHT);
//
//    auto width = (lv_coord_t) ((lv_obj_get_width_fit(tank_cont) / TANKS) * 0.5);
//
//
//    for (int i = 0; i < TANKS; i++) {
//        lv_obj_t *cont = lv_cont_create(tank_cont, nullptr);
//        lv_cont_set_layout(cont, LV_LAYOUT_COL_M);
//        lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);
//        lv_obj_t *bar = lv_bar_create(cont, nullptr);
//        lv_obj_set_size(bar, width, 125);
//        lv_obj_align(bar, nullptr, LV_ALIGN_CENTER, 0, 0);
//        lv_bar_set_anim_time(bar, 1000);
//        lv_obj_t *label = lv_label_create(cont, nullptr);
//        lv_label_set_text(label, tank_names[i]);
//        lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_symbol);
//        lv_label_set_recolor(label, true);
//        tank_ptrs[i] = bar;
//        lv_cont_set_fit2(cont, LV_FIT_TIGHT, LV_FIT_TIGHT);
//    }
//}
//
//void update_tanks(int16_t water, int16_t grey, int16_t black, int16_t lpg) {
//    lv_bar_set_value(tank_ptrs[0], water, LV_ANIM_ON);
//    lv_bar_set_value(tank_ptrs[1], grey, LV_ANIM_ON);
//    lv_bar_set_value(tank_ptrs[2], black, LV_ANIM_ON);
//    lv_bar_set_value(tank_ptrs[3], lpg, LV_ANIM_ON);
//}
//
//
//
//void create_temperature(lv_obj_t *parent) {
//    lv_obj_t *cont = lv_cont_create(parent, nullptr);
//    lv_cont_set_layout(cont, LV_LAYOUT_PRETTY);
//    lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &lv_style_transp);
//    lv_cont_set_fit2(cont, LV_FIT_FLOOD, LV_FIT_TIGHT);
//
//    for (const std::pair<const std::string, ha_entity *> &entity : ha_entities) {
//        if (entity.second->type == ha_entity_type::ha_weather) {
//            printf("weather: %s\n", entity.second->id);
////            new lvgl_temperature(cont, (ha_entity_weather *) entity.second, nullptr);
//                get_widget_by_type("temperature", cont, entity.second);
//        }
//    }
//
////    lv_obj_t *cont_outside = lv_cont_create(cont, nullptr);
////    lv_cont_set_style(cont_outside, LV_CONT_STYLE_MAIN, &lv_style_transp);
////    lv_cont_set_layout(cont_outside, LV_LAYOUT_COL_M);
////    lv_obj_t *outside_label = lv_label_create(cont_outside, nullptr);
////    lv_label_set_text(outside_label, "Outside");
////    lv_obj_t *outside_temp = lv_label_create(cont_outside, nullptr);
////    lv_label_set_text(outside_temp, "20.1");
////    lv_label_set_style(outside_temp, LV_LABEL_STYLE_MAIN, &style_temperature);
//
//    lv_obj_t *cont_inside = lv_cont_create(cont, nullptr);
//    lv_cont_set_style(cont_inside, LV_CONT_STYLE_MAIN, &lv_style_transp);
//    lv_cont_set_layout(cont_inside, LV_LAYOUT_COL_M);
//    lv_obj_t *inside_label = lv_label_create(cont_inside, nullptr);
//    lv_label_set_text(inside_label, "Inside");
//    lv_obj_t *inside_temp = lv_label_create(cont_inside, nullptr);
//    lv_label_set_text(inside_temp, "22.3");
//    lv_label_set_style(inside_temp, LV_LABEL_STYLE_MAIN, &style_temperature);
//}
//
//void create_battery(lv_obj_t *parent) {
//    lv_obj_t *bat_cont = lv_cont_create(parent, nullptr);
//    lv_obj_set_size(bat_cont, lv_obj_get_width_fit(parent), 75);
//    lv_cont_set_layout(bat_cont, LV_LAYOUT_ROW_M);
//    lv_cont_set_style(bat_cont, LV_CONT_STYLE_MAIN, &lv_style_transp);
//    lv_obj_t *bat_label = lv_label_create(bat_cont, nullptr);
//    lv_label_set_text(bat_label, "Bat");
//    lv_label_set_recolor(bat_label, true);
//    bat_bar = lv_bar_create(bat_cont, nullptr);
//    lv_obj_set_size(bat_bar, (lv_coord_t) lv_obj_get_width_fit(bat_cont) - 40, 30);
//    lv_bar_set_anim_time(bat_bar, 1000);
//}
//
//void update_battery(int16_t battery) {
//    lv_bar_set_value(bat_bar, battery, LV_ANIM_ON);
//}
//
//void callback(lv_obj_t *obj, lv_event_t event) {
//    if (event == LV_EVENT_REFRESH) {
//        printf("Refreshed\n");
//    }
//}
//
//void create_buttons(lv_obj_t *parent) {
//    lv_obj_t *but_cont = lv_cont_create(parent, nullptr);
//    lv_obj_set_size(but_cont, lv_obj_get_width_fit(parent), 50);
//    lv_cont_set_layout(but_cont, LV_LAYOUT_PRETTY);
//    lv_cont_set_fit2(but_cont, LV_FIT_FLOOD, LV_FIT_TIGHT);
//    lv_cont_set_style(but_cont, LV_CONT_STYLE_MAIN, &lv_style_transp);
//
//    lv_obj_t *label;
//
//    lv_obj_t *btn1 = lv_btn_create(but_cont, nullptr);
//    label = lv_label_create(btn1, nullptr);
//    lv_label_set_text(label, "Pump");
//    lv_obj_set_event_cb(btn1, callback);
//
//
//    lv_obj_t *btn2 = lv_btn_create(but_cont, nullptr);
//    label = lv_label_create(btn2, nullptr);
//    lv_label_set_text(label, "WH");
//    lv_obj_set_event_cb(btn2, callback);
//}
//
//
//void main_tab(lv_obj_t *parent) {
//    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
//    lv_page_set_scrl_layout(parent, LV_LAYOUT_CENTER);
//
//    create_temperature(parent);
//    create_tanks(parent);
//    create_battery(parent);
//    create_buttons(parent);
//
//
//}
