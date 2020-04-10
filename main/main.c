#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_system.h"

#include "wifi.h"
#include "spiffs.h"
#include "httpd.h"
#include "backlight.h"

#include "gui.h"
#include "websocket.h"
#include "ha_event.h"


void app_main() {
    spiffs_init();
    wifi_init();
    httpd_init();

    ha_event_init();
    gui_init();
    backlight_init();

    websocket_init();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1) {
        printf("Heap free %i\n", xPortGetFreeHeapSize());
        vTaskDelay(250);
    }
#pragma clang diagnostic pop

}