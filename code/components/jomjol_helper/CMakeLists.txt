FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer esp-tflite-micro esp_driver_tsens jomjol_logfile fatfs sdmmc vfs nvs_flash)


