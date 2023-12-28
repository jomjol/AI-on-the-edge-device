cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common components/esp-tflite-micro)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
           ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

if(EXISTS "${SDKCONFIG}.defaults")
    if(EXISTS "sdkconfig.defaults")
        set(SDKCONFIG_DEFAULTS "${SDKCONFIG}.defaults;sdkconfig.defaults")
        message(STATUS "-- Using defaults: ${SDKCONFIG_DEFAULTS} + sdkconfig.defaults")
    else()
        set(SDKCONFIG_DEFAULTS "${SDKCONFIG}.defaults")
    endif()
    message(STATUS "-- Using defaults: ${SDKCONFIG_DEFAULTS}")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(AI-on-the-edge)
