if(CONFIG_MDNS_NETWORKING_SOCKET)
    set(MDNS_NETWORKING "mdns_networking_socket.c")
else()
    set(MDNS_NETWORKING "mdns_networking_lwip.c")
endif()

if(CONFIG_MDNS_ENABLE_CONSOLE_CLI)
    set(MDNS_CONSOLE "mdns_console.c")
else()
    set(MDNS_CONSOLE "")
endif()

set(MDNS_MEMORY "mdns_mem_caps.c")

idf_build_get_property(target IDF_TARGET)
if(${target} STREQUAL "linux")
    set(dependencies esp_netif_linux esp_event)
    set(private_dependencies esp_timer console esp_system)
    set(srcs "mdns.c" ${MDNS_MEMORY} ${MDNS_NETWORKING} ${MDNS_CONSOLE})
else()
    set(dependencies lwip console esp_netif)
    set(private_dependencies esp_timer esp_wifi)
    set(srcs "mdns.c" ${MDNS_MEMORY} ${MDNS_NETWORKING} ${MDNS_CONSOLE})
endif()

idf_component_register(
        SRCS ${srcs}
        INCLUDE_DIRS "include"
        PRIV_INCLUDE_DIRS "private_include"
        REQUIRES ${dependencies}
        PRIV_REQUIRES ${private_dependencies})

if(${target} STREQUAL "linux")
    target_link_libraries(${COMPONENT_LIB} PRIVATE "-lbsd")
endif()


if(CONFIG_ETH_ENABLED)
    idf_component_optional_requires(PRIVATE esp_eth)
endif()
