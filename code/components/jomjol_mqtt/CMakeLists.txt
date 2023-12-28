FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer esp-tflite-micro mqtt jomjol_tfliteclass jomjol_helper jomjol_mqtt jomjol_wlan json)
