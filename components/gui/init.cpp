#include "lvgl/lvgl.h"
#include "gui.h"

void gui_init() {
    lv_obj_t *scr = lv_disp_get_scr_act(nullptr);
    lv_obj_clean(scr);
    lv_obj_t *tv = lv_tabview_create(scr, nullptr);
    lv_obj_set_size(tv, lv_obj_get_width_fit(scr), lv_obj_get_height_fit(scr));

    lv_obj_t *tab1 = lv_tabview_add_tab(tv, "\xEF\x80\x95");
    lv_obj_t *tab2 = lv_tabview_add_tab(tv, "\xEF\x83\xAB");
    lv_obj_t *tab3 = lv_tabview_add_tab(tv, "Status");

    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr);

    main_tab(tab1);
    light_tab(tab2);
}