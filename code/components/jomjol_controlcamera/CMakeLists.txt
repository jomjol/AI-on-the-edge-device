FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

list(APPEND EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES esp32-camera-master esp_http_server jomjol_logfile jomjol_image_proc nvs_flash jomjol_fileserver_ota jomjol_controlGPIO)


