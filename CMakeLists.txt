# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

add_compile_options("-Wno-unknown-pragmas"  "-Wl,-Map,output.map,--cref")
#add_link_options("")
add_definitions("-DARDUINOJSON_DECODE_UNICODE=1" "-DARDUINOJSON_ENABLE_ARDUINO_STRING=0" "-DARDUINOJSON_ENABLE_STD_STRING=0")

set(EXTRA_COMPONENT_DIRS
        components/lv_port_esp32/components/lvgl
        components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_tft
        components/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_touch
        components/lv_port_esp32/components/lvgl_esp32_drivers
        components/gui
        )

message( ${EXTRA_COMPONENT_DIRS} )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP_Display)

