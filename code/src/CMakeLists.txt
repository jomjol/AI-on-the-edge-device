# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

if(NOT CMAKE_BUILD_EARLY_EXPANSION) 
ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
           ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)
endif()

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# idf_component_register(SRCS ${app_sources})

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS ".")
