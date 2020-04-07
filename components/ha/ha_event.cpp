#include <freertos/FreeRTOS.h>
#include <esp_event_base.h>
#include <esp_event.h>
#include <esp_log.h>

#include "ha_event.h"

#define TAG "HA_EVENT"


esp_event_loop_handle_t ha_event_loop_hdl;

ESP_EVENT_DEFINE_BASE(ESP_HA_EVENT);


void ha_event_init(void) {
    /* Create Event loop */
    esp_event_loop_args_t loop_args = {
            .queue_size = 16,
            .task_name = "ha_event",
            .task_priority = 0,
            .task_stack_size = 2000,
            .task_core_id = 0
    };
    esp_event_loop_create(&loop_args, &ha_event_loop_hdl);
    ESP_LOGI(TAG, "HA Event init OK");
}

void ha_event_deinit() {
    esp_event_loop_delete(ha_event_loop_hdl);
}

void ha_event_post(int32_t event_id, void *event_data, size_t event_data_size, TickType_t ticks_to_wait){
    esp_event_post_to(ha_event_loop_hdl, ESP_HA_EVENT, event_id, event_data, event_data_size, ticks_to_wait);
}