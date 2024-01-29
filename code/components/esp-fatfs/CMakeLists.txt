set(srcs "diskio/diskio_mh.c"
         "diskio/diskio_rawflash_mh.c"
         "diskio/diskio_sdmmc_mh.c"
         "diskio/diskio_wl_mh.c"
         "src/ff_mh.c"
         "port/freertos/ffsystem_mh.c"
         "src/ffunicode_mh.c"
         "vfs/vfs_fat_mh.c"
         "vfs/vfs_fat_sdmmc_mh.c"
         "vfs/vfs_fat_spiflash_mh.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "." diskio vfs src
                       REQUIRES wear_levelling esp-sdmmc
                       PRIV_REQUIRES vfs spi_flash
                      )

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
