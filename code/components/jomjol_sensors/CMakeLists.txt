FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_timer jomjol_helper jomjol_logfile
                    PRIV_REQUIRES jomjol_mqtt jomjol_influxdb)
