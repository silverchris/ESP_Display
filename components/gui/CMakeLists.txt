file(GLOB SOURCES  "*.cpp" "*.c")

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS .
        REQUIRES lvgl_esp32_drivers lvgl lv_examples lvgl_tft lvgl_touch ha)