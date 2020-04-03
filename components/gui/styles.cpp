#include "lvgl/lvgl.h"
#include "gui.h"

lv_style_t style_tv_btn_rel;
lv_style_t style_tv_btn_pr;

lv_style_t style_symbol;
lv_style_t style_temperature;
lv_style_t style_label_small;
lv_style_t style_preload;


void style_setup(void) {
    LV_FONT_DECLARE(Symbols);

    lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
    style_tv_btn_rel.text.font = &Symbols;

    lv_style_copy(&style_tv_btn_pr, &lv_style_btn_pr);
    style_tv_btn_pr.text.font = &Symbols;

    lv_style_copy(&style_symbol, &lv_style_plain);
    style_symbol.text.font = &Symbols;

    lv_style_copy(&style_temperature, &lv_style_plain);
    style_temperature.text.font = &lv_font_roboto_28;

    lv_style_copy(&style_label_small, &lv_style_btn_rel);
    style_label_small.text.font = &lv_font_roboto_12;

    lv_style_copy(&style_preload, &lv_style_plain);
    style_preload.line.width = 10;                         /*10 px thick arc*/
    style_preload.line.color = lv_color_hex3(0x258);       /*Blueish arc color*/

    style_preload.body.border.color = lv_color_hex3(0xBBB); /*Gray background color*/
    style_preload.body.border.width = 10;
    style_preload.body.padding.left = 0;

}

