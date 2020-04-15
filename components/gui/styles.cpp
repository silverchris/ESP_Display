#include <lvgl/src/lv_core/lv_style.h>
#include "lvgl/lvgl.h"
#include "gui.h"

//lv_style_t style_tv_btn_rel;
//lv_style_t style_tv_btn_pr;

lv_style_t style_bar_label;
lv_style_t style_temperature;
lv_style_t style_label_small;
lv_style_t style_preload;
lv_style_t style_row;


void style_setup(void) {
//    LV_FONT_DECLARE(font_12);
//    LV_FONT_DECLARE(font_16);
//    LV_FONT_DECLARE(font_22);
//    LV_FONT_DECLARE(font_28);

//    lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
//    style_tv_btn_rel.text.font = &font_16;
//
//    lv_style_copy(&style_tv_btn_pr, &lv_style_btn_pr);
//    style_tv_btn_pr.text.font = &font_16;
//
    lv_style_copy(&style_bar_label, &lv_style_plain);
    style_bar_label.text.font = &font_22;

    lv_style_copy(&style_temperature, &lv_style_plain);
    style_temperature.text.font = &font_28;

    lv_style_copy(&style_label_small, &lv_style_btn_rel);
    style_label_small.text.font = &font_12;

    lv_style_copy(&style_preload, &lv_style_plain);
    style_preload.line.width = 10;                         /*10 px thick arc*/
    style_preload.line.color = lv_color_hex3(0x258);       /*Blueish arc color*/
    style_preload.body.border.color = lv_color_hex3(0xBBB); /*Gray background color*/
    style_preload.body.border.width = 10;
    style_preload.body.padding.left = 0;


    lv_style_copy(&style_row, &lv_style_transp_tight);
    style_row.body.padding.left = 25;
    style_row.body.padding.right = 25;
}

