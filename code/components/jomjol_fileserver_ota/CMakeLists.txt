FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "." "../../include"
                    REQUIRES tfmicro esp_http_server app_update esp_http_client nvs_flash jomjol_tfliteclass jomjol_flowcontroll spiffs jomjol_helper jomjol_controlGPIO)


