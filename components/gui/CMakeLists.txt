file(GLOB SOURCES  "*.cpp" "*.c")

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS .
        REQUIRES spiffs lvgl_esp32_drivers lvgl lvgl_tft lvgl_touch ha)