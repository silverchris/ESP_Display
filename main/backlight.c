#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_event.h"

#include "backlight.h"
#include "gui.h"

// GPIO 21

#define LEDC_TEST_FADE_TIME    (3000)

void backlight_set(int percent){
    int value = (int)((float)percent/100.00)*8191;

    ledc_set_duty(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, value);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0);
}

void backlight_off(){
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, 0, 3000);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
}

uint8_t backlight_state;

void backlight_task(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    backlight_state = 1;
    while(1){
        if(get_activity() > 60000){
            backlight_state = 0;
            backlight_off();
        } else if(backlight_state ==0){
            backlight_set(100);
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}

void backlight_init(void) {
    gpio_set_direction(21, GPIO_MODE_OUTPUT);

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
            .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
            .freq_hz = 2000,                      // frequency of PWM signal
            .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
            .timer_num = LEDC_TIMER_0,            // timer index
            .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = 21,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
    };

    ledc_channel_config(&ledc_channel);


    // Initialize fade service.
    ledc_fade_func_install(0);

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 8191);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    xTaskCreate(backlight_task, "backlight_task", 1000, NULL, 0, NULL);
}