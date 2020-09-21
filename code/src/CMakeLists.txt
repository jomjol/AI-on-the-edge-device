# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

<<<<<<< Updated upstream
FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
=======


#if(NOT CMAKE_BUILD_EARLY_EXPANSION) 
#ADD_CUSTOM_COMMAND(
#    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
#           ${CMAKE_CURRENT_SOURCE_DIR}/_version.cpp
#    COMMAND ${CMAKE_COMMAND} -P
#            ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)
##endif()

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.* )
>>>>>>> Stashed changes

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS ".")
