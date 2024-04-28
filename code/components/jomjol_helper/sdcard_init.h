/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stddef.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_types.h"
#include "driver/sdspi_host.h"
#include "ff.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convenience function to get FAT filesystem on SD card registered in VFS
 *
 * This is an all-in-one function which does the following:
 * - initializes SDMMC driver or SPI driver with configuration in host_config
 * - initializes SD card with configuration in slot_config
 * - mounts FAT partition on SD card using FATFS library, with configuration in mount_config
 * - registers FATFS library with VFS, with prefix given by base_prefix variable
 *
 * This function is intended to make example code more compact.
 * For real world applications, developers should implement the logic of
 * probing SD card, locating and mounting partition, and registering FATFS in VFS,
 * with proper error checking and handling of exceptional conditions.
 *
 * @note Use this API to mount a card through SDSPI is deprecated. Please call
 *       `esp_vfs_fat_sdspi_mount()` instead for that case.
 *
 * @param base_path     path where partition should be registered (e.g. "/sdcard")
 * @param host_config   Pointer to structure describing SDMMC host. When using
 *                      SDMMC peripheral, this structure can be initialized using
 *                      SDMMC_HOST_DEFAULT() macro. When using SPI peripheral,
 *                      this structure can be initialized using SDSPI_HOST_DEFAULT()
 *                      macro.
 * @param slot_config   Pointer to structure with slot configuration.
 *                      For SDMMC peripheral, pass a pointer to sdmmc_slot_config_t
 *                      structure initialized using SDMMC_SLOT_CONFIG_DEFAULT.
 * @param mount_config  pointer to structure with extra parameters for mounting FATFS
 * @param[out] out_card  if not NULL, pointer to the card information structure will be returned via this argument
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if esp_vfs_fat_sdmmc_mount was already called
 *      - ESP_ERR_NO_MEM if memory can not be allocated
 *      - ESP_FAIL if partition can not be mounted
 *      - other error codes from SDMMC or SPI drivers, SDMMC protocol, or FATFS drivers
 */
esp_err_t esp_vfs_fat_sdmmc_mount_mh(const char* base_path, const sdmmc_host_t* host_config, const void* slot_config, const esp_vfs_fat_mount_config_t* mount_config, sdmmc_card_t** out_card);

/**
 * @brief Convenience function to get FAT filesystem on SD card registered in VFS
 *
 * This is an all-in-one function which does the following:
 * - initializes an SPI Master device based on the SPI Master driver with configuration in
 *   slot_config, and attach it to an initialized SPI bus.
 * - initializes SD card with configuration in host_config_input
 * - mounts FAT partition on SD card using FATFS library, with configuration in mount_config
 * - registers FATFS library with VFS, with prefix given by base_prefix variable
 *
 * This function is intended to make example code more compact.
 * For real world applications, developers should implement the logic of
 * probing SD card, locating and mounting partition, and registering FATFS in VFS,
 * with proper error checking and handling of exceptional conditions.
 *
 * @note This function try to attach the new SD SPI device to the bus specified in host_config.
 *       Make sure the SPI bus specified in `host_config->slot` have been initialized by
 *       `spi_bus_initialize()` before.
 *
 * @param base_path     path where partition should be registered (e.g. "/sdcard")
 * @param host_config_input Pointer to structure describing SDMMC host. This structure can be
 *                          initialized using SDSPI_HOST_DEFAULT() macro.
 * @param slot_config   Pointer to structure with slot configuration.
 *                      For SPI peripheral, pass a pointer to sdspi_device_config_t
 *                      structure initialized using SDSPI_DEVICE_CONFIG_DEFAULT().
 * @param mount_config  pointer to structure with extra parameters for mounting FATFS
 * @param[out] out_card If not NULL, pointer to the card information structure will be returned via
 *                      this argument. It is suggested to hold this handle and use it to unmount the card later if
 *                      needed. Otherwise it's not suggested to use more than one card at the same time and unmount one
 *                      of them in your application.
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if esp_vfs_fat_sdmmc_mount was already called
 *      - ESP_ERR_NO_MEM if memory can not be allocated
 *      - ESP_FAIL if partition can not be mounted
 *      - other error codes from SDMMC or SPI drivers, SDMMC protocol, or FATFS drivers
 */
esp_err_t esp_vfs_fat_sdspi_mount_mh(const char* base_path, const sdmmc_host_t* host_config_input, const sdspi_device_config_t* slot_config, const esp_vfs_fat_mount_config_t* mount_config, sdmmc_card_t** out_card);

#ifdef __cplusplus
}
#endif










