#ifndef GUI_H
#define GUI_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus

#include "ha.hpp"
extern "C" {
#endif

void gui_init();

#include "freertos/FreeRTOS.h"
#include <freertos/ringbuf.h>
RingbufHandle_t screenshot_init();

extern lv_style_t style_bar_label;
extern lv_style_t style_temperature;
extern lv_style_t style_label_small;
extern lv_style_t style_preload;
extern lv_style_t style_row;



void style_setup(void);


#ifdef __cplusplus
}
#endif


#endif