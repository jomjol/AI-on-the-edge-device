#include "StayAwake.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "STAYAWAKE";

// Use the default NVS partition + a small namespace just for this flag.
static const char *NVS_NAMESPACE = "aiote";
static const char *NVS_KEY       = "stay_awake";


bool StayAwake_Get()
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK) {
        return false;
    }
    uint8_t v = 0;
    esp_err_t err = nvs_get_u8(h, NVS_KEY, &v);
    nvs_close(h);
    if (err != ESP_OK) return false;  // not set yet -> default off
    return v != 0;
}


bool StayAwake_Set(bool value)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed");
        return false;
    }
    esp_err_t err = nvs_set_u8(h, NVS_KEY, value ? 1 : 0);
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set/commit failed: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "Stay-Awake override set to %s", value ? "ON" : "OFF");
    return true;
}
