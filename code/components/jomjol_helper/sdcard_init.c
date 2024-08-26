/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include "sdcard_init.h"

#include "esp_log.h"
#include "ffconf.h"
#include "esp_compiler.h"
#include "esp_vfs.h"
#include "vfs_fat_internal.h"
#include "diskio_impl.h"
#include "diskio_sdmmc.h"
#include "soc/soc_caps.h"
#include "driver/sdmmc_defs.h"

#if SOC_SDMMC_HOST_SUPPORTED
	#include "driver/sdmmc_host.h"
#endif

static sdmmc_card_t* s_cards[FF_VOLUMES] = { NULL };
static bool s_disk_status_check_en[FF_VOLUMES] = { };

static const char* TAG = "sdcard_init";

#define CHECK_EXECUTE_RESULT(err, str) do { \
    if ((err) !=ESP_OK) { \
        ESP_LOGE(TAG, str" (0x%x).", err); \
        goto cleanup; \
    } \
    } while(0)

typedef struct mh_vfs_fat_sd_ctx_t {
    BYTE pdrv;                                  //Drive number that is mounted
    esp_vfs_fat_mount_config_t mount_config;    //Mount configuration
    FATFS *fs;                                  //FAT structure pointer that is registered
    sdmmc_card_t *card;                         //Card info
    char *base_path;                            //Path where partition is registered
} mh_vfs_fat_sd_ctx_t;

static mh_vfs_fat_sd_ctx_t *s_ctx[FF_VOLUMES] = {};

/**
 * This `s_saved_ctx_id` is only used by `esp_vfs_fat_sdmmc_unmount`, which is deprecated.
 * This variable together with `esp_vfs_fat_sdmmc_unmount` should be removed in next major version
 */
static uint32_t s_saved_ctx_id = FF_VOLUMES;


static void call_host_deinit_mh(const sdmmc_host_t *host_config);
static esp_err_t partition_card_mh(const esp_vfs_fat_mount_config_t *mount_config, const char *drv, sdmmc_card_t *card, BYTE pdrv);


//Check if SD/MMC card is present
static DSTATUS ff_sdmmc_card_available_mh(BYTE pdrv)
{
    sdmmc_card_t* card = s_cards[pdrv];
    assert(card);
    esp_err_t err = sdmmc_get_status(card);
	
    if (unlikely(err != ESP_OK)) {
        ESP_LOGE(TAG, "Check status failed (0x%x)", err);
        return STA_NOINIT;
    }
    return 0;
}

/**
*   ff_sdmmc_status() and ff_sdmmc_initialize() return STA_NOINIT when sdmmc_get_status()
*   fails. This error value is checked throughout the FATFS code.
*   Both functions return 0 on success.
*/
DSTATUS ff_sdmmc_initialize_mh (BYTE pdrv)
{
    return ff_sdmmc_card_available_mh(pdrv);
}

DSTATUS ff_sdmmc_status_mh(BYTE pdrv)
{
    if (s_disk_status_check_en[pdrv]) {
        return ff_sdmmc_card_available_mh(pdrv);
    }
    return 0;
}

DRESULT ff_sdmmc_read_mh (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    sdmmc_card_t* card = s_cards[pdrv];
    assert(card);
    esp_err_t err = sdmmc_read_sectors(card, buff, sector, count);
    if (unlikely(err != ESP_OK)) {
        ESP_LOGE(TAG, "sdmmc_read_blocks failed (%d)", err);
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT ff_sdmmc_write_mh (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    sdmmc_card_t* card = s_cards[pdrv];
    assert(card);
    esp_err_t err = sdmmc_write_sectors(card, buff, sector, count);
    if (unlikely(err != ESP_OK)) {
        ESP_LOGE(TAG, "sdmmc_write_blocks failed (%d)", err);
        return RES_ERROR;
    }
    return RES_OK;
}

#if FF_USE_TRIM
DRESULT ff_sdmmc_trim_mh (BYTE pdrv, DWORD start_sector, DWORD sector_count)
{
    sdmmc_card_t* card = s_cards[pdrv];
    assert(card);
    sdmmc_erase_arg_t arg;

    arg = sdmmc_can_discard(card) == ESP_OK ? SDMMC_DISCARD_ARG : SDMMC_ERASE_ARG;
    esp_err_t err = sdmmc_erase_sectors(card, start_sector, sector_count, arg);
    if (unlikely(err != ESP_OK)) {
        ESP_LOGE(TAG, "sdmmc_erase_sectors failed (%d)", err);
        return RES_ERROR;
    }
    return RES_OK;
}
#endif //FF_USE_TRIM

DRESULT ff_sdmmc_ioctl_mh (BYTE pdrv, BYTE cmd, void* buff)
{
    sdmmc_card_t* card = s_cards[pdrv];
    assert(card);
    switch(cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((DWORD*) buff) = card->csd.capacity;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD*) buff) = card->csd.sector_size;
            return RES_OK;
        case GET_BLOCK_SIZE:
            return RES_ERROR;
#if FF_USE_TRIM
        case CTRL_TRIM:
            if (sdmmc_can_trim(card) != ESP_OK) {
                return RES_PARERR;
            }
            return ff_sdmmc_trim_mh (pdrv, *((DWORD*)buff), //start_sector
                    (*((DWORD*)buff + 1) - *((DWORD*)buff) + 1)); //sector_count
#endif //FF_USE_TRIM
    }
    return RES_ERROR;
}

void ff_sdmmc_set_disk_status_check_mh(BYTE pdrv, bool enable)
{
    s_disk_status_check_en[pdrv] = enable;
}

void ff_diskio_register_sdmmc_mh(BYTE pdrv, sdmmc_card_t* card)
{
    static const ff_diskio_impl_t sdmmc_impl = {
        .init = &ff_sdmmc_initialize_mh,
        .status = &ff_sdmmc_status_mh,
        .read = &ff_sdmmc_read_mh,
        .write = &ff_sdmmc_write_mh,
        .ioctl = &ff_sdmmc_ioctl_mh
    };
    s_cards[pdrv] = card;
    s_disk_status_check_en[pdrv] = false;
    ff_diskio_register(pdrv, &sdmmc_impl);
}

BYTE ff_diskio_get_pdrv_card_mh(const sdmmc_card_t* card)
{
    for (int i = 0; i < FF_VOLUMES; i++) {
        if (card == s_cards[i]) {
            return i;
        }
    }
    return 0xff;
}

static bool s_get_context_id_by_card_mh(const sdmmc_card_t *card, uint32_t *out_id)
{
    mh_vfs_fat_sd_ctx_t *p_ctx = NULL;
    for (int i = 0; i < FF_VOLUMES; i++) {
        p_ctx = s_ctx[i];
        if (p_ctx) {
            if (p_ctx->card == card) {
                *out_id = i;
                return true;
            }
        }
    }
    return false;
}

static uint32_t s_get_unused_context_id_mh(void)
{
    for (uint32_t i = 0; i < FF_VOLUMES; i++) {
        if (!s_ctx[i]) {
            return i;
        }
    }
    return FF_VOLUMES;
}

static esp_err_t mount_prepare_mem_mh(const char *base_path, BYTE *out_pdrv, char **out_dup_path, sdmmc_card_t** out_card)
{
    esp_err_t err = ESP_OK;
    char* dup_path = NULL;
    sdmmc_card_t* card = NULL;

    // connect SDMMC driver to FATFS
    BYTE pdrv = FF_DRV_NOT_USED;

    if (ff_diskio_get_drive(&pdrv) != ESP_OK || pdrv == FF_DRV_NOT_USED) {
        ESP_LOGD(TAG, "the maximum count of volumes is already mounted");
        return ESP_ERR_NO_MEM;
    }

    // not using ff_memalloc here, as allocation in internal RAM is preferred
    card = (sdmmc_card_t*)malloc(sizeof(sdmmc_card_t));

    if (card == NULL) {
        ESP_LOGD(TAG, "could not locate new sdmmc_card_t");
        err = ESP_ERR_NO_MEM;
        goto cleanup;
    }

    dup_path = strdup(base_path);

    if(!dup_path){
        ESP_LOGD(TAG, "could not copy base_path");
        err = ESP_ERR_NO_MEM;
        goto cleanup;
    }

    *out_card = card;
    *out_pdrv = pdrv;
    *out_dup_path = dup_path;
    return ESP_OK;
cleanup:
    free(card);
    free(dup_path);
    return err;
}

static esp_err_t s_f_mount_mh(sdmmc_card_t *card, FATFS *fs, const char *drv, uint8_t pdrv, const esp_vfs_fat_mount_config_t *mount_config)
{
    esp_err_t err = ESP_OK;
    FRESULT res = f_mount(fs, drv, 1);
    if (res != FR_OK) {
        err = ESP_FAIL;
        ESP_LOGW(TAG, "failed to mount card (%d)", res);

        bool need_mount_again = (res == FR_NO_FILESYSTEM || res == FR_INT_ERR) && mount_config->format_if_mount_failed;

        if (!need_mount_again) {
            return ESP_FAIL;
        }

        err = partition_card_mh(mount_config, drv, card, pdrv);

        if (err != ESP_OK) {
            return err;
        }

        ESP_LOGW(TAG, "mounting again");
        res = f_mount(fs, drv, 0);

        if (res != FR_OK) {
            err = ESP_FAIL;
            ESP_LOGD(TAG, "f_mount failed after formatting (%d)", res);
            return err;
        }
    }

    return ESP_OK;
}

static esp_err_t mount_to_vfs_fat_mh(const esp_vfs_fat_mount_config_t *mount_config, sdmmc_card_t *card, uint8_t pdrv, const char *base_path, FATFS **out_fs)
{
    FATFS *fs = NULL;
    esp_err_t err;
    ff_diskio_register_sdmmc_mh(pdrv, card);
    ff_sdmmc_set_disk_status_check_mh(pdrv, mount_config->disk_status_check_enable);
    ESP_LOGD(TAG, "using pdrv=%i", pdrv);
    char drv[3] = {(char)('0' + pdrv), ':', 0};

    // connect FATFS to VFS
    err = esp_vfs_fat_register(base_path, drv, mount_config->max_files, &fs);
    *out_fs = fs;

    if (err == ESP_ERR_INVALID_STATE) {
        // it's okay, already registered with VFS
    } else if (err != ESP_OK) {
        ESP_LOGD(TAG, "esp_vfs_fat_register failed 0x(%x)", err);
        goto fail;
    }

    // Try to mount partition
    err = s_f_mount_mh(card, fs, drv, pdrv, mount_config);

    if (err != ESP_OK) {
        goto fail;
    }
    return ESP_OK;

fail:
    if (fs) {
        f_mount(NULL, drv, 0);
    }
    esp_vfs_fat_unregister_path(base_path);
    ff_diskio_unregister(pdrv);
    return err;
}

static esp_err_t partition_card_mh(const esp_vfs_fat_mount_config_t *mount_config, const char *drv, sdmmc_card_t *card, BYTE pdrv)
{
    FRESULT res = FR_OK;
    esp_err_t err;
    const size_t workbuf_size = 4096;
    void* workbuf = NULL;
    ESP_LOGW(TAG, "partitioning card");

    workbuf = ff_memalloc(workbuf_size);

    if (workbuf == NULL) {
        return ESP_ERR_NO_MEM;
    }

    LBA_t plist[] = {100, 0, 0, 0};
    res = f_fdisk(pdrv, plist, workbuf);

    if (res != FR_OK) {
        err = ESP_FAIL;
        ESP_LOGD(TAG, "f_fdisk failed (%d)", res);
        goto fail;
    }

    size_t alloc_unit_size = esp_vfs_fat_get_allocation_unit_size(card->csd.sector_size, mount_config->allocation_unit_size);

    ESP_LOGW(TAG, "formatting card, allocation unit size=%d", alloc_unit_size);
    const MKFS_PARM opt = {(BYTE)FM_ANY, 0, 0, 0, alloc_unit_size};
    res = f_mkfs(drv, &opt, workbuf, workbuf_size);

    if (res != FR_OK) {
        err = ESP_FAIL;
        ESP_LOGD(TAG, "f_mkfs failed (%d)", res);
        goto fail;
    }

    free(workbuf);
    return ESP_OK;
fail:
    free(workbuf);
    return err;
}

#if SOC_SDMMC_HOST_SUPPORTED
static esp_err_t init_sdmmc_host_mh(int slot, const void *slot_config, int *out_slot)
{
    *out_slot = slot;
    return sdmmc_host_init_slot(slot, (const sdmmc_slot_config_t*) slot_config);
}

esp_err_t esp_vfs_fat_sdmmc_mount_mh(const char* base_path, const sdmmc_host_t* host_config, const void* slot_config, const esp_vfs_fat_mount_config_t* mount_config, sdmmc_card_t** out_card)
{
    esp_err_t err;
    mh_vfs_fat_sd_ctx_t *ctx = NULL;
    uint32_t ctx_id = FF_VOLUMES;
    FATFS *fs = NULL;
    int card_handle = -1;   //uninitialized
    sdmmc_card_t* card = NULL;
    BYTE pdrv = FF_DRV_NOT_USED;
    char* dup_path = NULL;
    bool host_inited = false;

    err = mount_prepare_mem_mh(base_path, &pdrv, &dup_path, &card);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mount_prepare failed");
        return err;
    }

    err = (*host_config->init)();
    CHECK_EXECUTE_RESULT(err, "host init failed");
    //deinit() needs to be called to revert the init
    host_inited = true;
    //If this failed (indicated by card_handle != -1), slot deinit needs to called()
    //leave card_handle as is to indicate that (though slot deinit not implemented yet.
    err = init_sdmmc_host_mh(host_config->slot, slot_config, &card_handle);
    CHECK_EXECUTE_RESULT(err, "slot init failed");

    // probe and initialize card
    err = sdmmc_card_init(host_config, card);
    CHECK_EXECUTE_RESULT(err, "sdmmc_card_init failed");

    err = mount_to_vfs_fat_mh(mount_config, card, pdrv, dup_path, &fs);
    CHECK_EXECUTE_RESULT(err, "mount_to_vfs failed");

    if (out_card != NULL) {
        *out_card = card;
    }

    //For deprecation backward compatibility
    if (s_saved_ctx_id == FF_VOLUMES) {
        s_saved_ctx_id = 0;
    }

    ctx = calloc(sizeof(mh_vfs_fat_sd_ctx_t), 1);

    if (!ctx) {
        CHECK_EXECUTE_RESULT(ESP_ERR_NO_MEM, "no mem");
    }

    ctx->pdrv = pdrv;
    memcpy(&ctx->mount_config, mount_config, sizeof(esp_vfs_fat_mount_config_t));
    ctx->card = card;
    ctx->base_path = dup_path;
    ctx->fs = fs;
    ctx_id = s_get_unused_context_id_mh();
    assert(ctx_id != FF_VOLUMES);
    s_ctx[ctx_id] = ctx;

    return ESP_OK;
cleanup:
    if (host_inited) {
        call_host_deinit_mh(host_config);
    }

    free(card);
    free(dup_path);
    return err;
}
#endif

static esp_err_t init_sdspi_host_mh(int slot, const void *slot_config, int *out_slot)
{
    esp_err_t err = sdspi_host_init_device((const sdspi_device_config_t*)slot_config, out_slot);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,
"Failed to attach sdspi device onto an SPI bus (rc=0x%x), please initialize the \
bus first and check the device parameters."
            , err);
    }
    return err;
}

esp_err_t esp_vfs_fat_sdspi_mount_mh(const char* base_path, const sdmmc_host_t* host_config_input, const sdspi_device_config_t* slot_config, const esp_vfs_fat_mount_config_t* mount_config, sdmmc_card_t** out_card)
{
    const sdmmc_host_t* host_config = host_config_input;
    esp_err_t err;
    mh_vfs_fat_sd_ctx_t *ctx = NULL;
    uint32_t ctx_id = FF_VOLUMES;
    FATFS *fs = NULL;
    int card_handle = -1;   //uninitialized
    bool host_inited = false;
    BYTE pdrv = FF_DRV_NOT_USED;
    sdmmc_card_t* card = NULL;
    char* dup_path = NULL;

    err = mount_prepare_mem_mh(base_path, &pdrv, &dup_path, &card);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mount_prepare failed");
        return err;
    }

    //the init() function is usually empty, doesn't require any deinit to revert it
    err = (*host_config->init)();
    CHECK_EXECUTE_RESULT(err, "host init failed");

    err = init_sdspi_host_mh(host_config->slot, slot_config, &card_handle);
    CHECK_EXECUTE_RESULT(err, "slot init failed");
    //Set `host_inited` to true to indicate that host_config->deinit() needs
    //to be called to revert `init_sdspi_host`
    host_inited = true;

    //The `slot` argument inside host_config should be replaced by the SD SPI handled returned
    //above. But the input pointer is const, so create a new variable.

    sdmmc_host_t new_config;

    if (card_handle != host_config->slot) {
        new_config = *host_config_input;
        host_config = &new_config;
        new_config.slot = card_handle;
    }

    // probe and initialize card
    err = sdmmc_card_init(host_config, card);
    CHECK_EXECUTE_RESULT(err, "sdmmc_card_init failed");

    err = mount_to_vfs_fat_mh(mount_config, card, pdrv, dup_path, &fs);
    CHECK_EXECUTE_RESULT(err, "mount_to_vfs failed");

    if (out_card != NULL) {
        *out_card = card;
    }

    //For deprecation backward compatibility
    if (s_saved_ctx_id == FF_VOLUMES) {
        s_saved_ctx_id = 0;
    }

    ctx = calloc(sizeof(mh_vfs_fat_sd_ctx_t), 1);

    if (!ctx) {
        CHECK_EXECUTE_RESULT(ESP_ERR_NO_MEM, "no mem");
    }

    ctx->pdrv = pdrv;
    memcpy(&ctx->mount_config, mount_config, sizeof(esp_vfs_fat_mount_config_t));
    ctx->card = card;
    ctx->base_path = dup_path;
    ctx->fs = fs;
    ctx_id = s_get_unused_context_id_mh();
    assert(ctx_id != FF_VOLUMES);
    s_ctx[ctx_id] = ctx;

    return ESP_OK;

cleanup:
    if (host_inited) {
        call_host_deinit_mh(host_config);
    }

    free(card);
    free(dup_path);
    return err;
}

static void call_host_deinit_mh(const sdmmc_host_t *host_config)
{
    if (host_config->flags & SDMMC_HOST_FLAG_DEINIT_ARG) {
        host_config->deinit_p(host_config->slot);
    } else {
        host_config->deinit();
    }
}

static esp_err_t unmount_card_core_mh(const char *base_path, sdmmc_card_t *card)
{
    BYTE pdrv = ff_diskio_get_pdrv_card_mh(card);

    if (pdrv == 0xff) {
        return ESP_ERR_INVALID_ARG;
    }

    // unmount
    char drv[3] = {(char)('0' + pdrv), ':', 0};
    f_mount(0, drv, 0);
    // release SD driver
    ff_diskio_unregister(pdrv);

    call_host_deinit_mh(&card->host);
    free(card);

    esp_err_t err = esp_vfs_fat_unregister_path(base_path);
    return err;
}

esp_err_t esp_vfs_fat_sdmmc_unmount_mh(void)
{
    esp_err_t err = unmount_card_core_mh(s_ctx[s_saved_ctx_id]->base_path, s_ctx[s_saved_ctx_id]->card);
    free(s_ctx[s_saved_ctx_id]);
    s_ctx[s_saved_ctx_id] = NULL;
    s_saved_ctx_id = FF_VOLUMES;
    return err;
}

esp_err_t esp_vfs_fat_sdcard_unmount_mh(const char *base_path, sdmmc_card_t *card)
{
    uint32_t id = FF_VOLUMES;
    bool found = s_get_context_id_by_card_mh(card, &id);

    if (!found) {
        return ESP_ERR_INVALID_ARG;
    }

    free(s_ctx[id]);
    s_ctx[id] = NULL;

    esp_err_t err = unmount_card_core_mh(base_path, card);

    return err;
}

esp_err_t esp_vfs_fat_sdcard_format_mh(const char *base_path, sdmmc_card_t *card)
{
    esp_err_t ret = ESP_OK;

    if (!card) {
        ESP_LOGE(TAG, "card not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    BYTE pdrv = ff_diskio_get_pdrv_card_mh(card);

    if (pdrv == 0xff) {
        ESP_LOGE(TAG, "card driver not registered");
        return ESP_ERR_INVALID_STATE;
    }

    const size_t workbuf_size = 4096;
    void *workbuf = ff_memalloc(workbuf_size);

    if (workbuf == NULL) {
        return ESP_ERR_NO_MEM;
    }

    //unmount
    char drv[3] = {(char)('0' + pdrv), ':', 0};
    f_mount(0, drv, 0);

    //format
    uint32_t id = FF_VOLUMES;
    bool found = s_get_context_id_by_card_mh(card, &id);
    assert(found);
    size_t alloc_unit_size = esp_vfs_fat_get_allocation_unit_size(card->csd.sector_size, s_ctx[id]->mount_config.allocation_unit_size);
    ESP_LOGI(TAG, "Formatting card, allocation unit size=%d", alloc_unit_size);
    const MKFS_PARM opt = {(BYTE)FM_ANY, 0, 0, 0, alloc_unit_size};
    FRESULT res = f_mkfs(drv, &opt, workbuf, workbuf_size);
    free(workbuf);

    if (res != FR_OK) {
        ret = ESP_FAIL;
        ESP_LOGD(TAG, "f_mkfs failed (%d)", res);
    }

    //mount back
    esp_err_t err = s_f_mount_mh(card, s_ctx[id]->fs, drv, pdrv, &s_ctx[id]->mount_config);

    if (err != ESP_OK) {
        unmount_card_core_mh(base_path, card);
        ESP_LOGE(TAG, "failed to format, resources recycled, please mount again");
    }

    return ret;
}
