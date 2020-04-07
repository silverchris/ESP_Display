#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lvgl/lvgl.h"
#include "lvgl_driver.h"
#include "gui.h"
#include "widgets.h"
#include "ha_event.h"


static void IRAM_ATTR lv_tick_task(void *arg);
void guiTask();

static void IRAM_ATTR lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(portTICK_RATE_MS);
}

//Creates a semaphore to handle concurrent call to lvgl stuff
//If you wish to call *any* lvgl function from other threads/tasks
//you should lock on the very same semaphore!
SemaphoreHandle_t xGuiSemaphore;

void event_ha_update(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data)
{
    ha_event_data in = *((ha_event_data*) event_data);
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t)100) == pdTRUE) {
        auto widget = (widget_base *) in.ptr;
        widget->refresh();
        xSemaphoreGive(xGuiSemaphore);
    }
}

void event_ha_ready(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data)
{
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
        lv_obj_t *scr = lv_disp_get_scr_act(nullptr);
        lv_obj_clean(scr);
        lv_obj_t *tv = lv_tabview_create(scr, nullptr);
        lv_obj_set_size(tv, lv_obj_get_width_fit(scr), lv_obj_get_height_fit(scr));
        lv_tabview_set_sliding(tv, false);
        lv_tabview_set_anim_time(tv, 0);

        lv_obj_t *tab1 = lv_tabview_add_tab(tv, "\xEF\x80\x95");
        lv_obj_t *tab2 = lv_tabview_add_tab(tv, "\xEF\x83\xAB");
        lv_obj_t *tab3 = lv_tabview_add_tab(tv, "Status");

        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr);
        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel);
        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr);

        main_tab(tab1);
        light_tab(tab2);
        xSemaphoreGive(xGuiSemaphore);
        esp_event_handler_register_with(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_UPDATE, event_ha_update, nullptr);
    }
}


void guiTask() {
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    lvgl_driver_init();

    static auto buf1 = (lv_color_t *)malloc(sizeof(lv_color_t)*CONFIG_LVGL_DISPLAY_WIDTH*20);
    static auto buf2 = (lv_color_t *)malloc(sizeof(lv_color_t)*CONFIG_LVGL_DISPLAY_WIDTH*20);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf1, buf2, 320*20);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

#if CONFIG_LVGL_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

    LV_FONT_DECLARE(Symbols);
    lv_theme_t *th = lv_theme_material_init(270, NULL);
    lv_theme_set_current(th);
    style_setup();

    /*Create a Preloader object*/
    lv_obj_t *preload = lv_preload_create(lv_scr_act(), NULL);
    lv_obj_set_size(preload, 100, 100);
    lv_obj_align(preload, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_preload_set_style(preload, LV_PRELOAD_STYLE_MAIN, &style_preload);


    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &lv_tick_task,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    //On ESP32 it's better to create a periodic task instead of esp_register_freertos_tick_hook
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10*1000)); //10ms (expressed as microseconds)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        vTaskDelay(1);
        //Try to lock the semaphore, if success, call lvgl stuff
        if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }
    }
#pragma clang diagnostic pop
}

void gui_init() {
    xTaskCreatePinnedToCore((TaskFunction_t) guiTask, "gui", 4096 * 2, nullptr, 0, nullptr, 1);
    vTaskDelay(100);
    esp_event_handler_register_with(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_READY, event_ha_ready, nullptr);


}