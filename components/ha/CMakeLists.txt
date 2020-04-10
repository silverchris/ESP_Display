file(GLOB SOURCES "*.cpp" "*.c")

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS .
        REQUIRES ArduinoJson websocket)