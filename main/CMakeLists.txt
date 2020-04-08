file(GLOB SOURCES "*.cpp" "*.c")
idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS ""
        REQUIRES spiffs esp_http_server lvgl_esp32_drivers lvgl lvgl_tft lvgl_touch gui ha websocket)
target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)