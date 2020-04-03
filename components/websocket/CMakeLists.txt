file(GLOB SOURCES  "*.cpp" "*.c")

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS .
        REQUIRES esp_event esp_wifi esp_websocket_client nvs_flash ArduinoJson ha)