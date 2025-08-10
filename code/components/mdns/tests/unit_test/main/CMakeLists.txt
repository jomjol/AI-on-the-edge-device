
idf_component_register(SRCS "test_mdns.c"
                       REQUIRES test_utils
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES unity mdns)
