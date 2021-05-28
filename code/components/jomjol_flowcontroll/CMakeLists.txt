FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES jomjol_tfliteclass jomjol_helper jomjol_controlcamera jomjol_mqtt jomjol_fileserver_ota jomjol_image_proc jomjol_wlan)


