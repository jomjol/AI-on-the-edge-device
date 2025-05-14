FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "."
                    REQUIRES esp_eth nvs_flash wpa_supplicant jomjol_wlan jomjol_helper jomjol_mqtt esp_netif)


