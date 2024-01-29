/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "diskio_impl_mh.h"
#include "ffconf_mh.h"
#include "ff_mh.h"
#include "esp_log.h"
#include "diskio_rawflash_mh.h"
#include "esp_compiler.h"
#include "spi_flash_mmap.h"


static const char* TAG = "diskio_rawflash";

const esp_partition_t* ff_raw_handles[FF_VOLUMES];


DSTATUS ff_raw_initialize (BYTE pdrv)
{
    return 0;
}

DSTATUS ff_raw_status (BYTE pdrv)
{
    return 0;
}

DRESULT ff_raw_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    ESP_LOGV(TAG, "ff_raw_read - pdrv=%i, sector=%i, count=%in", (unsigned int)pdrv, (unsigned int)sector, (unsigned int)count);
    const esp_partition_t* part = ff_raw_handles[pdrv];
    assert(part);
    esp_err_t err = esp_partition_read(part, sector * SPI_FLASH_SEC_SIZE, buff, count * SPI_FLASH_SEC_SIZE);
    if (unlikely(err != ESP_OK)) {
        ESP_LOGE(TAG, "esp_partition_read failed (0x%x)", err);
        return RES_ERROR;
    }
    return RES_OK;
}


DRESULT ff_raw_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    return RES_ERROR;
}

DRESULT ff_raw_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
    const esp_partition_t* part = ff_raw_handles[pdrv];
    ESP_LOGV(TAG, "ff_raw_ioctl: cmd=%in", cmd);
    assert(part);
    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((DWORD *) buff) = part->size / SPI_FLASH_SEC_SIZE;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD *) buff) = SPI_FLASH_SEC_SIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            return RES_ERROR;
    }
    return RES_ERROR;
}


esp_err_t ff_diskio_register_raw_partition(BYTE pdrv, const esp_partition_t* part_handle)
{
    if (pdrv >= FF_VOLUMES) {
        return ESP_ERR_INVALID_ARG;
    }
    static const ff_diskio_impl_t raw_impl = {
        .init = &ff_raw_initialize,
        .status = &ff_raw_status,
        .read = &ff_raw_read,
        .write = &ff_raw_write,
        .ioctl = &ff_raw_ioctl
    };
    ff_diskio_register(pdrv, &raw_impl);
    ff_raw_handles[pdrv] = part_handle;
    return ESP_OK;

}


BYTE ff_diskio_get_pdrv_raw(const esp_partition_t* part_handle)
{
    for (int i = 0; i < FF_VOLUMES; i++) {
        if (part_handle == ff_raw_handles[i]) {
            return i;
        }
    }
    return 0xff;
}
