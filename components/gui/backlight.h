
#ifndef ESP_DISPLAY_BACKLIGHT_H
#define ESP_DISPLAY_BACKLIGHT_H

#ifdef __cplusplus
extern "C" {
#endif


extern uint8_t backlight_level;

void backlight_init(void);

void backlight_set(uint8_t percent);

void backlight_off();

#ifdef __cplusplus
}
#endif

#endif //ESP_DISPLAY_BACKLIGHT_H
