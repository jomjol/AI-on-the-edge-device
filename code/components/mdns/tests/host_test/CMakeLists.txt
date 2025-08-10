cmake_minimum_required(VERSION 3.5)


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
if(${IDF_TARGET} STREQUAL "linux")
    set(EXTRA_COMPONENT_DIRS "../../../../common_components/linux_compat")
    set(COMPONENTS main)
endif()

project(mdns_host)

# Enable sanitizers only without console (we'd see some leaks on argtable when console exits)
if(NOT CONFIG_TEST_CONSOLE AND CONFIG_IDF_TARGET_LINUX)
idf_component_get_property(mdns mdns COMPONENT_LIB)
target_link_options(${mdns} INTERFACE -fsanitize=address -fsanitize=undefined)
endif()
