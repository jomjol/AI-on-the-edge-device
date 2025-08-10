idf_build_get_property(idf_target IDF_TARGET)
if(${IDF_TARGET} STREQUAL "linux")
    idf_component_register(SRCS esp_netif_linux.c
                           INCLUDE_DIRS include $ENV{IDF_PATH}/components/esp_netif/include
                           REQUIRES esp_event)
else()
    idf_component_register()
endif()
