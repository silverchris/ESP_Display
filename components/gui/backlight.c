#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_event.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "backlight.h"
#include "gui.h"

// GPIO 21
uint8_t backlight_level;

void backlight_set(uint8_t percent) {
    backlight_level = percent;
    uint32_t value = (uint32_t) ((float) (percent / 100.00) * 8191.00);
    printf("Setting backlight to %u\n", value);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, value);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    printf("Backlight finished\n");
}

void backlight_off() {
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0, 3000);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
}

esp_adc_cal_characteristics_t *adc_chars;

void adc_task(void *args){
    int adc_value = adc1_get_raw(ADC1_GPIO36_CHANNEL);
    int voltage = esp_adc_cal_raw_to_voltage(adc_value, adc_chars);
    printf("Raw: %d\tVoltage: %dmV\n", adc_value, voltage);
}


void backlight_init(void) {
    gpio_set_direction((gpio_num_t) 21, GPIO_MODE_OUTPUT);

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

    backlight_level = 100;

    gpio_num_t adc_in = (gpio_num_t)36;

    esp_err_t r;
    r = adc1_pad_get_io_num(ADC1_GPIO36_CHANNEL, &adc_in);
    assert(r == ESP_OK);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_GPIO36_CHANNEL, ADC_ATTEN_DB_11);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }

    const esp_timer_create_args_t adc_timer_args = {
            .callback = &adc_task,
            /* name is optional, but may help identify the timer when debugging */
            .name = "adc_task"
    };
    esp_timer_handle_t adc_timer;
    ESP_ERROR_CHECK(esp_timer_create(&adc_timer_args, &adc_timer));
    //On ESP32 it's better to create a periodic task instead of esp_register_freertos_tick_hook
    ESP_ERROR_CHECK(esp_timer_start_periodic(adc_timer, 1000 * 1000)); //10ms (expressed as microseconds)


}