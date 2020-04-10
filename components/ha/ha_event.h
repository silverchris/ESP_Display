#ifndef ESP_DISPLAY_HA_EVENT_H
#define ESP_DISPLAY_HA_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <freertos/FreeRTOS.h>
#include <esp_event_base.h>
#include <esp_event.h>
#include <esp_system.h>
#include <esp_log.h>

ESP_EVENT_DECLARE_BASE(ESP_HA_EVENT);

extern esp_event_loop_handle_t ha_event_loop_hdl;

enum {
    HA_EVENT_READY,
    HA_EVENT_UPDATE
};

struct ha_event_data {
    void *ptr;
};


void ha_event_init(void);

void ha_event_deinit();

void ha_event_post(int32_t event_id, void *event_data, size_t event_data_size, TickType_t ticks_to_wait);

#ifdef __cplusplus
}
#endif

#endif //ESP_DISPLAY_HA_EVENT_H
