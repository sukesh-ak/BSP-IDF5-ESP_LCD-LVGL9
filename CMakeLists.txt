cmake_minimum_required(VERSION 3.16)

# Make sure sdkconfig is created in the build folder
set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(IDF-ESP_LCD-LVGL)
