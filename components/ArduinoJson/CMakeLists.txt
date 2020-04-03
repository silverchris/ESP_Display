file(GLOB SOURCES  "ArduinoJson/src/*.cpp" "ArduinoJson/src/*.c")

idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS ArduinoJson/src/)