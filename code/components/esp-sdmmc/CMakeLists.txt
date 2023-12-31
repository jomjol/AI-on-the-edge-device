idf_component_register(SRCS "sdmmc_cmd_mh.c"
                            "sdmmc_common_mh.c"
                            "sdmmc_init_mh.c"
                            "sdmmc_io_mh.c"
                            "sdmmc_mmc_mh.c"
                            "sdmmc_sd_mh.c"
                    INCLUDE_DIRS "." include
                    REQUIRES driver esp-fatfs
                    PRIV_REQUIRES soc)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
