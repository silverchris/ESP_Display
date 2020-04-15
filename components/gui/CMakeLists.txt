file(GLOB SOURCES "*.cpp" "*.c")

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS .
        REQUIRES esp_adc_cal spiffs lvgl_esp32_drivers lvgl lvgl_tft lvgl_touch ha)