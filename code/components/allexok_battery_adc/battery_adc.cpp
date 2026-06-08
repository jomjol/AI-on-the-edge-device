#ifdef BOARD_ESP32_S3_ALEKSEI

#include "battery_adc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#include <stdlib.h>
#include <stdint.h>

static const char *TAG = "BATTERY";

// GPIO2 = ADC1_CH1 on ESP32-S3.
static constexpr adc_unit_t       BAT_UNIT     = ADC_UNIT_1;
static constexpr adc_channel_t    BAT_CHANNEL  = ADC_CHANNEL_1;
static constexpr adc_atten_t      BAT_ATTEN    = ADC_ATTEN_DB_12;     // ~0-3.1 V input range
static constexpr adc_bitwidth_t   BAT_BITWIDTH = ADC_BITWIDTH_DEFAULT;
static constexpr int              BLOCKS       = 7;
static constexpr int              PER_BLOCK    = 41;                  // 287 reads total per call

static adc_oneshot_unit_handle_t  s_adc_handle  = nullptr;
static adc_cali_handle_t          s_cali_handle = nullptr;
static bool                       s_ready       = false;
static int                        s_last_raw    = -1;
static int                        s_last_mv     = -1;


bool Battery_Init()
{
    if (s_ready) return true;

    adc_oneshot_unit_init_cfg_t unit_cfg = {};
    unit_cfg.unit_id  = BAT_UNIT;
    unit_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
    if (adc_oneshot_new_unit(&unit_cfg, &s_adc_handle) != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_new_unit failed");
        return false;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {};
    chan_cfg.atten    = BAT_ATTEN;
    chan_cfg.bitwidth = BAT_BITWIDTH;
    if (adc_oneshot_config_channel(s_adc_handle, BAT_CHANNEL, &chan_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_config_channel failed");
        return false;
    }

    // ESP32-S3 uses curve-fitting calibration (eFuse-backed). If the chip
    // wasn't calibrated at factory the call returns ESP_ERR_NOT_SUPPORTED
    // and we fall back to a linear conversion based on the bit width.
    adc_cali_curve_fitting_config_t cali_cfg = {};
    cali_cfg.unit_id  = BAT_UNIT;
    cali_cfg.chan     = BAT_CHANNEL;
    cali_cfg.atten    = BAT_ATTEN;
    cali_cfg.bitwidth = BAT_BITWIDTH;
    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_cali_handle) != ESP_OK) {
        ESP_LOGW(TAG, "ADC curve-fitting calibration unavailable; using uncalibrated conversion");
        s_cali_handle = nullptr;
    }

    s_ready = true;
    ESP_LOGI(TAG, "Battery monitor initialised on GPIO2 (ADC1_CH1)");
    return true;
}


bool Battery_IsReady()
{
    return s_ready;
}


static int cmp_int(const void *a, const void *b)
{
    int ai = *(const int *)a, bi = *(const int *)b;
    return (ai > bi) - (ai < bi);
}


// Convert a raw ADC reading to millivolts using calibration when available,
// or a linear fallback (12 dB attenuation -> ~0-3100 mV full scale).
static int raw_to_mv(int raw)
{
    int mv = 0;
    if (s_cali_handle && adc_cali_raw_to_voltage(s_cali_handle, raw, &mv) == ESP_OK) {
        return mv;
    }
    return (raw * 3100) / 4095;
}


// Median-of-medians: take BLOCKS independent blocks of PER_BLOCK reads each,
// median-filter each block to reject SAR spikes, then average the per-block
// medians for stability. Approach inherited from allexoK's original draft.
float Battery_ReadVoltage()
{
    if (!s_ready) return -1.0f;

    int block_raw[PER_BLOCK];
    long sum_raw = 0;
    long sum_mv  = 0;
    int  ok_blocks = 0;

    for (int b = 0; b < BLOCKS; ++b) {
        int filled = 0;
        for (int i = 0; i < PER_BLOCK; ++i) {
            int r = 0;
            if (adc_oneshot_read(s_adc_handle, BAT_CHANNEL, &r) == ESP_OK) {
                block_raw[filled++] = r;
            }
        }
        if (filled == 0) continue;

        qsort(block_raw, filled, sizeof(block_raw[0]), cmp_int);
        int median_raw = block_raw[filled / 2];
        sum_raw += median_raw;
        sum_mv  += raw_to_mv(median_raw);
        ok_blocks++;
    }

    if (ok_blocks == 0) {
        ESP_LOGW(TAG, "Battery_ReadVoltage: no successful samples");
        return -1.0f;
    }

    s_last_raw = sum_raw / ok_blocks;
    s_last_mv  = sum_mv  / ok_blocks;
    return (s_last_mv / 1000.0f) * BAT_DIVIDER_RATIO;
}


int Battery_LastRawAdc()
{
    return s_ready ? s_last_raw : -1;
}


float Battery_LastAdcVoltage()
{
    if (!s_ready || s_last_mv < 0) return -1.0f;
    return s_last_mv / 1000.0f;
}


int Battery_PercentFromVoltage(float v)
{
    if (v < 0.0f) return -1;
    if (v >= LIION_USB_BACKFEED_THRESHOLD_V) return 100;

    // Piecewise Li-ion discharge curve. Coarse but useful for a status icon.
    struct { float v; int pct; } curve[] = {
        { 4.20f, 100 },
        { 4.10f,  95 },
        { 4.00f,  85 },
        { 3.90f,  75 },
        { 3.80f,  60 },
        { 3.75f,  45 },
        { 3.70f,  30 },
        { 3.60f,  15 },
        { 3.40f,   5 },
        { LIION_EMPTY_V, 0 },
    };

    if (v >= curve[0].v) return curve[0].pct;
    const int n = sizeof(curve) / sizeof(curve[0]);
    for (int i = 1; i < n; ++i) {
        if (v >= curve[i].v) {
            float span = curve[i-1].v - curve[i].v;
            float frac = (v - curve[i].v) / span;
            return curve[i].pct + (int)(frac * (curve[i-1].pct - curve[i].pct));
        }
    }
    return 0;
}

#endif // BOARD_ESP32_S3_ALEKSEI
