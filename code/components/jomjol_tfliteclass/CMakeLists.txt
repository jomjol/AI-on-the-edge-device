FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES jomjol_image_proc jomjol_logfile esp_http_server esp32-camera-master jomjol_controlcamera jomjol_flowcontroll jomjol_helper)


