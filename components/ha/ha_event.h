#ifndef ESP_DISPLAY_HA_EVENT_H
#define ESP_DISPLAY_HA_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(ESP_HA_EVENT);

extern esp_event_loop_handle_t ha_event_loop_hdl;

enum {
    HA_EVENT_READY
};

void ha_event_init(void);
void ha_event_deinit();

#ifdef __cplusplus
}
#endif

#endif //ESP_DISPLAY_HA_EVENT_H
