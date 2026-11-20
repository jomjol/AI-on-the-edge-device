FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS 
                        "." 
                        "../../include"

                    REQUIRES 
                        jomjol_configfile 
                        jomjol_controlcamera 
                        jomjol_controlGPIO 
                        jomjol_fileserver_ota 
                        jomjol_flowcontroll 
                        jomjol_helper 
                        jomjol_image_proc 
                        jomjol_influxdb 
                        jomjol_mqtt 
                        jomjol_tfliteclass 
                        jomjol_time_sntp 
                        jomjol_webhook 
                        jomjol_network 
                        openmetrics
                    )
