cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

set(PROJECT_VER "0.0.9.3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32cam-server-only)
