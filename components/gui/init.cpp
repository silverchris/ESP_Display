#include <sys/unistd.h>
#include <iostream>
#include <fstream>

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"


#include <ArduinoJson.h>

#include "lvgl/lvgl.h"
#include "lvgl_driver.h"
#include "gui.h"
#include "widgets.h"
#include "ha.hpp"
#include "ha_event.h"
#include "ha_entities.h"
#include "backlight.h"

uint8_t backlight_state;


void callback_func(lv_obj_t *obj, lv_event_t event) {
    if (backlight_state) { // Disable button callbacks when backlight is off
        auto data = (lvgl_callback) lv_obj_get_user_data(obj);
        if (data.widget != nullptr) {
            ((widget_base *) data.widget)->callback(obj, event);
        }
    }
}

void load_gui_config(lv_obj_t *tv) {
    ESP_LOGI("CONFIG", "Reading file");
    std::ifstream config_file;
    config_file.open("/spiffs/config.json");
    // TODO: Catch errors if this fails

    DynamicJsonDocument config(10000);
    DeserializationError error;

    error = deserializeJson(config, config_file);

    if (error) {
        ESP_LOGE("CONFIG", "deserializeJson() failed: %s\n", error.c_str());
        config.clear();
    }
    for (JsonObject page: config.as<JsonArray>()) {
        lv_obj_t *tab = lv_tabview_add_tab(tv, (const char *) page["name"]);
        for (JsonArray row: page["rows"].as<JsonArray>()) {
            lv_page_set_style(tab, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
            lv_page_set_scrl_layout(tab, LV_LAYOUT_CENTER);

            lv_obj_t *cont = lv_cont_create(tab, nullptr);
            lv_cont_set_layout(cont, LV_LAYOUT_PRETTY);
            lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &style_row);
            lv_cont_set_fit2(cont, LV_FIT_FLOOD, LV_FIT_TIGHT);

            for (JsonObject item: row) {
                const char *label = nullptr;
                if (item.containsKey("label")) {
                    label = item["label"];
                }
                ha_entity *entity = get_entity((const char *) item["entity"]);
                if (entity != nullptr) {
                    get_widget_by_type(item["type"].as<char *>(), cont, entity, label, &callback_func);
                }
            }
        }
    }
    config_file.close();

}


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

void event_ha_update(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    ha_event_data in = *((ha_event_data *) event_data);
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t) 100) == pdTRUE) {
        auto widget = (widget_base *) in.ptr;
        widget->refresh();
        xSemaphoreGive(xGuiSemaphore);
    }
}

void brightness_func(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        uint8_t percent = lv_slider_get_value(obj);
        backlight_set(percent);
    }
}

void config_window(void)
{

    lv_obj_t * win = lv_win_create(lv_scr_act(), nullptr);
    lv_win_set_title(win, "Settings");


    lv_obj_t * close_btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE);           /*Add close button and use built-in close action*/
    lv_obj_set_event_cb(close_btn, lv_win_close_event_cb);

    lv_obj_t *brightness_cont = lv_cont_create(win, nullptr);
    lv_cont_set_layout(brightness_cont, LV_LAYOUT_PRETTY);
    lv_cont_set_style(brightness_cont, LV_CONT_STYLE_MAIN, &lv_style_transp_tight);
    lv_cont_set_fit2(brightness_cont, LV_FIT_FLOOD, LV_FIT_TIGHT);

    lv_obj_t *brightness_label = lv_label_create(brightness_cont, nullptr);
    lv_label_set_text(brightness_label, "Brightness");
    lv_obj_align(brightness_label, nullptr, LV_ALIGN_IN_TOP_LEFT, 10, 10);

    lv_obj_t *brightness = lv_slider_create(brightness_cont, nullptr);
    lv_obj_set_width(brightness, LV_DPI * 2);
    lv_obj_align(brightness, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(brightness, brightness_func);
    lv_slider_set_range(brightness, 0, 100);
    lv_slider_set_value(brightness, backlight_level, static_cast<lv_anim_enable_t>(false));


}

static void tv_btnm_event_cb(lv_obj_t *tab_btnm, lv_event_t event) {
    if (event == LV_EVENT_LONG_PRESSED && lv_btnm_get_active_btn(tab_btnm) == 0) {
        config_window();
    }

    if (event != LV_EVENT_CLICKED) return;

    uint16_t btn_id = lv_btnm_get_active_btn(tab_btnm);
    if (btn_id == LV_BTNM_BTN_NONE) return;

    lv_btnm_clear_btn_ctrl_all(tab_btnm, LV_BTNM_CTRL_TGL_STATE);
    lv_btnm_set_btn_ctrl(tab_btnm, btn_id, LV_BTNM_CTRL_TGL_STATE);

    lv_obj_t *tabview = lv_obj_get_parent(tab_btnm);

    uint32_t id_prev = lv_tabview_get_tab_act(tabview);
    lv_tabview_set_tab_act(tabview, btn_id, LV_ANIM_ON);
    uint32_t id_new = lv_tabview_get_tab_act(tabview);

    lv_res_t res = LV_RES_OK;
    if (id_prev != id_new) res = lv_event_send(tabview, LV_EVENT_VALUE_CHANGED, &id_new);

    if (res != LV_RES_OK) return;
}

void event_ha_ready(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t) 10) == pdTRUE) {
        lv_obj_t *scr = lv_disp_get_scr_act(nullptr);
        lv_obj_clean(scr);
        lv_obj_t *tv = lv_tabview_create(scr, nullptr);
        lv_obj_set_size(tv, lv_obj_get_width_fit(scr), lv_obj_get_height_fit(scr));
        lv_tabview_set_sliding(tv, false);
        lv_tabview_set_anim_time(tv, 0);

//        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
//        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr);
//        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel);
//        lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr);

        auto *tv_ext = (lv_tabview_ext_t *) lv_obj_get_ext_attr(tv);

        lv_obj_set_event_cb(tv_ext->btns, tv_btnm_event_cb);


        load_gui_config(tv);

        xSemaphoreGive(xGuiSemaphore);
        esp_event_handler_register_with(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_UPDATE, event_ha_update, nullptr);
    }
}

static void backlight_task(void *args) {
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t) 10) == pdTRUE) {
        if (lv_disp_get_inactive_time(nullptr) > 60000) {
            if (backlight_state == 1){
                backlight_state = 0;
                backlight_off();
            }
        } else if (backlight_state == 0) {
            backlight_state = 1;
            backlight_set(100);
        }
        xSemaphoreGive(xGuiSemaphore);
    }
}

uint32_t get_activity() {
    uint32_t activity = 0;
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t) 10) == pdTRUE) {
        activity = lv_disp_get_inactive_time(nullptr);
        xSemaphoreGive(xGuiSemaphore);
    }
    return activity;
}

void guiTask() {
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    lvgl_driver_init();

    static auto buf1 = (lv_color_t *) malloc(sizeof(lv_color_t) * CONFIG_LVGL_DISPLAY_WIDTH * 20);
    static auto buf2 = (lv_color_t *) malloc(sizeof(lv_color_t) * CONFIG_LVGL_DISPLAY_WIDTH * 20);
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf1, buf2, 320 * 20);

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

    lv_theme_t *th = lv_theme_material_init(270, nullptr);
    lv_theme_set_current(th);
    style_setup();

    /*Create a Preloader object*/
    lv_obj_t *preload = lv_preload_create(lv_scr_act(), nullptr);
    lv_obj_set_size(preload, 100, 100);
    lv_obj_align(preload, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_preload_set_style(preload, LV_PRELOAD_STYLE_MAIN, &style_preload);


    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &lv_tick_task,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    //On ESP32 it's better to create a periodic task instead of esp_register_freertos_tick_hook
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000)); //10ms (expressed as microseconds)

    backlight_state = 1;

    const esp_timer_create_args_t backlight_timer_args = {
            .callback = &backlight_task,
            .name = "periodic_backlight"
    };
    esp_timer_handle_t backlight_timer;
    ESP_ERROR_CHECK(esp_timer_create(&backlight_timer_args, &backlight_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(backlight_timer, 100 * 1000)); //10ms (expressed as microseconds)

//    xTaskCreatePinnedToCore((TaskFunction_t) guiTask, "gui", 4096 * 2, nullptr, 0, nullptr, 1);
    vTaskDelay(100/portTICK_RATE_MS);
    esp_event_handler_register_with(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_READY, event_ha_ready, nullptr);


}