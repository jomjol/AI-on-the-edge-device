#include "ClassControllCamera.h"
#include "ClassLogFile.h"

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "Helper.h"
#include "statusled.h"
#include "CImageBasis.h"

#include "server_ota.h"
#include "server_GPIO.h"

#include "../../include/defines.h"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"

#include "driver/ledc.h"
#include "MainFlowControl.h"

#include "ov2640_sharpness.h"
#include "ov2640_specialEffect.h"
#include "ov2640_contrast_brightness.h"
#include "ov5640_autofocus.h"
#include "ov5640_autofocus_config.h"

#if (ESP_IDF_VERSION_MAJOR >= 5)
#include "soc/periph_defs.h"
#include "esp_private/periph_ctrl.h"
#include "soc/gpio_sig_map.h"
#include "soc/gpio_periph.h"
#include "soc/io_mux_reg.h"
#include "esp_rom_gpio.h"
#define gpio_pad_select_gpio esp_rom_gpio_pad_select_gpio
#define gpio_matrix_in(a, b, c) esp_rom_gpio_connect_in_signal(a, b, c)
#define gpio_matrix_out(a, b, c, d) esp_rom_gpio_connect_out_signal(a, b, c, d)
#define ets_delay_us(a) esp_rom_delay_us(a)
#endif

CCamera Camera;
camera_controll_config_temp_t CCstatus;

static const char *TAG = "CAM";

/* Camera live stream */
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

uint8_t *demoImage = NULL;    // Buffer holding the demo image in bytes
#define DEMO_IMAGE_SIZE 30000 // Max size of demo image in bytes

// Camera module bus communications frequency.
// Originally: config.xclk_freq_mhz = 20000000, but this lead to visual artifacts on many modules.
// See https://github.com/espressif/esp32-camera/issues/150#issuecomment-726473652 et al.
#if !defined(XCLK_FREQ_MHZ)
// int xclk = 8;
int xclk = 20; // Orginal value
#else
int xclk = XCLK_FREQ_MHZ;
#endif

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = (xclk * 1000000),
    .ledc_timer = LEDC_TIMER_0,     // LEDC timer to be used for generating XCLK
    .ledc_channel = LEDC_CHANNEL_0, // LEDC channel to be used for generating XCLK

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_VGA,    // QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    // .frame_size = FRAMESIZE_UXGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    .jpeg_quality = 12,                 // 0-63 lower number means higher quality
    .fb_count = 1,                     // if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM, /*!< The location where the frame buffer will be allocated */
    .grab_mode = CAMERA_GRAB_LATEST,   // only from new esp32cam version
};

typedef struct
{
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

CCamera::CCamera(void)
{
#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "CreateClassCamera");
#endif
    CCstatus.WaitBeforePicture = 2;

    ledc_init();
}

esp_err_t CCamera::InitCam(void)
{
    ESP_LOGD(TAG, "Init Camera");

    TickType_t cam_xDelay = 100 / portTICK_PERIOD_MS;

    CCstatus.CamConfig.ImageQuality = camera_config.jpeg_quality;
    CCstatus.CamConfig.ImageFrameSize = camera_config.frame_size;
    SetImageWidthHeightFromResolution(&CCstatus.CamConfig, CCstatus.CamConfig.ImageFrameSize);

    CCstatus.CamConfig.CameraFocusEnabled = false;
    CCstatus.CamConfig.CameraManualFocus = false;
    CCstatus.CamConfig.CameraManualFocusLevel = 0;
    CCstatus.CameraFocusLevel = 0;

    // De-init in case it was already initialized
    esp_camera_deinit();
    vTaskDelay(cam_xDelay);

    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    vTaskDelay(cam_xDelay);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    CCstatus.CameraInitSuccessful = true;
    CCstatus.CameraAFInitSuccessful = false;

    // Get a reference to the sensor
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        CCstatus.CamSensor_id = s->id.PID;

        // Dump camera module, warn for unsupported modules.
        switch (CCstatus.CamSensor_id)
        {
        case OV2640_PID:
            ESP_LOGI(TAG, "OV2640 camera module detected");
            break;
        case OV3660_PID:
            ESP_LOGI(TAG, "OV3660 camera module detected");
            break;
        case OV5640_PID:
            ESP_LOGI(TAG, "OV5640 camera module detected");
            break;
        default:
            ESP_LOGE(TAG, "Camera module is unknown and not properly supported!");
            CCstatus.CameraInitSuccessful = false;
            CCstatus.CameraAFInitSuccessful = false;
        }
    }

    if (CCstatus.CameraInitSuccessful)
    {
        if (s != NULL)
        {
            if (CCstatus.CamSensor_id == OV2640_PID)
            {
                uint8_t reg = s->get_reg(s, 0x09, 0xff);
                s->set_reg(s, 0x09, 0xff, (reg &= ~0x10));
            }
            else
            {
                s->set_reg(s, 0x3008, 0x42, 0x02);
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
            s->set_framesize(s, CCstatus.CamConfig.ImageFrameSize);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

bool CCamera::testCamera(void)
{
    bool success;
    camera_fb_t *fb = esp_camera_fb_get();

    if (fb)
    {
        success = true;
    }
    else
    {
        success = false;
    }

    esp_camera_fb_return(fb);

    return success;
}

void CCamera::ledc_init(void)
{
#ifdef USE_PWM_LEDFLASH
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {};

    ledc_timer.speed_mode = LEDC_MODE;
    ledc_timer.timer_num = LEDC_TIMER;
    ledc_timer.duty_resolution = LEDC_DUTY_RES;
    ledc_timer.freq_hz = LEDC_FREQUENCY; // Set output frequency at 5 kHz
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {};

    ledc_channel.speed_mode = LEDC_MODE;
    ledc_channel.channel = LEDC_CHANNEL;
    ledc_channel.timer_sel = LEDC_TIMER;
    ledc_channel.intr_type = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num = LEDC_OUTPUT_IO;
    ledc_channel.duty = 0; // Set duty to 0%
    ledc_channel.hpoint = 0;
    // ledc_channel.flags.output_invert = LEDC_OUTPUT_INVERT;

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
#endif
}

int CCamera::CalculateLEDIntensity(int _intrel)
{
    int LedIntensity = (int)((float)(std::min(std::max(0, _intrel), 100)) / 100 * 8191);
    ESP_LOGD(TAG, "Calculated led_intensity: %i of 8191", LedIntensity);
    return LedIntensity;
}

bool CCamera::initCameraAF(void)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        if (CCstatus.CameraAFInitSuccessful == false)
        {
            ESP_LOGI(TAG, "Initializing autofocus ...");
            // Load autofocus microcode into memory so it can be used when creating new reference image
            if (ov5640_autofocus_init(s) == 0) {
                ESP_LOGI(TAG, "Autofocus init success");
                CCstatus.CameraAFInitSuccessful = true;
                // We want autofocus powered down until we want to capture an image
                int rc = ov5640_autofocus_release(s);
                if (rc == 0) {
                    ESP_LOGI(TAG, "Release autofocus success");
                } else {
                    ESP_LOGI(TAG, "Release autofocus failed: %d", rc);
                }
            } else {
                ESP_LOGI(TAG, "Autofocus init failed");
                CCstatus.CameraAFInitSuccessful = false;
            }
        }
    }
    return CCstatus.CameraAFInitSuccessful;
}

bool CCamera::getCameraInitSuccessful(void)
{
    return CCstatus.CameraInitSuccessful;
}

bool CCamera::getCameraAFInitSuccessful(void)
{
    return CCstatus.CameraAFInitSuccessful;
}

esp_err_t CCamera::configureSensor(cam_config_t *camConfig)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        CameraDeepSleep(false);

        s->set_framesize(s, camConfig->ImageFrameSize);
        vTaskDelay(100 / portTICK_PERIOD_MS);
		
        // s->set_contrast(s, camConfig->ImageContrast);     // -2 to 2
        // s->set_brightness(s, camConfig->ImageBrightness); // -2 to 2
        SetCamContrastBrightness(s, camConfig->ImageContrast, camConfig->ImageBrightness);
		
        s->set_saturation(s, camConfig->ImageSaturation); // -2 to 2

        s->set_quality(s, camConfig->ImageQuality); // 0 - 63
		
        // s->set_gainceiling(s, camConfig->ImageGainceiling); // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
        SetCamGainceiling(s, camConfig->ImageGainceiling);
		
        s->set_gain_ctrl(s, camConfig->ImageAgc);     // 0 = disable , 1 = enable
        s->set_exposure_ctrl(s, camConfig->ImageAec); // 0 = disable , 1 = enable
        s->set_hmirror(s, camConfig->ImageHmirror); // 0 = disable , 1 = enable
        s->set_vflip(s, camConfig->ImageVflip);     // 0 = disable , 1 = enable
		
        s->set_whitebal(s, camConfig->ImageAwb);     // 0 = disable , 1 = enable
        s->set_aec2(s, camConfig->ImageAec2);       // 0 = disable , 1 = enable
        s->set_aec_value(s, camConfig->ImageAecValue); // 0 to 1200
        // s->set_special_effect(s, camConfig->ImageSpecialEffect); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        SetCamSpecialEffect(s, camConfig->ImageSpecialEffect);
        s->set_wb_mode(s, camConfig->ImageWbMode);               // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        s->set_ae_level(s, camConfig->ImageAeLevel);   // -2 to 2
		
        s->set_dcw(s, camConfig->ImageDcw); // 0 = disable , 1 = enable
        s->set_bpc(s, camConfig->ImageBpc); // 0 = disable , 1 = enable
        s->set_wpc(s, camConfig->ImageWpc); // 0 = disable , 1 = enable
        s->set_awb_gain(s, camConfig->ImageAwbGain); // 0 = disable , 1 = enable
        s->set_agc_gain(s, camConfig->ImageAgcGain);   // 0 to 30
		
        s->set_raw_gma(s, camConfig->ImageRawGma); // 0 = disable , 1 = enable
        s->set_lenc(s, camConfig->ImageLenc);         // 0 = disable , 1 = enable

        // s->set_sharpness(s, camConfig->ImageSharpness);   // auto-sharpness is not officially supported, default to 0
        SetCamSharpness(camConfig->ImageAutoSharpness, camConfig->ImageSharpness);
        s->set_denoise(s, camConfig->ImageDenoiseLevel); // The OV2640 does not support it, OV3660 and OV5640 (0 to 8)

        vTaskDelay(100 / portTICK_PERIOD_MS);

        SetQualityZoomSize(camConfig);

        vTaskDelay(100 / portTICK_PERIOD_MS);

        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

esp_err_t CCamera::setSensorDatenFromCCstatus(void)
{
    return configureSensor(&CCstatus.CamConfig);
}

void CCamera::readSensorConfig(sensor_t *s, cam_config_t *camConfig)
{
    camConfig->ImageFrameSize = (framesize_t)s->status.framesize;
    
    camConfig->ImageContrast = s->status.contrast;
    camConfig->ImageBrightness = s->status.brightness;
    camConfig->ImageSaturation = s->status.saturation;
    
    camConfig->ImageQuality = s->status.quality;
    
    camConfig->ImageGainceiling = (gainceiling_t)s->status.gainceiling;

    camConfig->ImageAgc = s->status.agc;
    camConfig->ImageAec = s->status.aec;
    camConfig->ImageHmirror = s->status.hmirror;
    camConfig->ImageVflip = s->status.vflip;
    
    camConfig->ImageAwb = s->status.awb;
    camConfig->ImageAec2 = s->status.aec2;
    camConfig->ImageAecValue = s->status.aec_value;
    camConfig->ImageSpecialEffect = s->status.special_effect;
    camConfig->ImageWbMode = s->status.wb_mode;
    camConfig->ImageAeLevel = s->status.ae_level;
    
    camConfig->ImageDcw = s->status.dcw;
    camConfig->ImageBpc = s->status.bpc;
    camConfig->ImageWpc = s->status.wpc;
    camConfig->ImageAwbGain = s->status.awb_gain;
    camConfig->ImageAgcGain = s->status.agc_gain;
    
    camConfig->ImageRawGma = s->status.raw_gma;
    camConfig->ImageLenc = s->status.lenc;

    // camConfig->ImageSharpness = s->status.sharpness; // gibt -1 zurück, da es nicht unterstützt wird
    camConfig->ImageDenoiseLevel = s->status.denoise;
}

esp_err_t CCamera::getSensorDatenToCCstatus(void)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        CCstatus.CamSensor_id = s->id.PID;
        readSensorConfig(s, &CCstatus.CamConfig);
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

int CCamera::PrecaptureCamSetup(bool *focusEnabled, bool *manualFocus, bool *needReloadZoomConfig)
{
    int ret = 0;

    cam_config_t *camConfig = CCstatus.isTempImage ? &CFstatus.CamConfig : &CCstatus.CamConfig;
    CCstatus.isTempImage = false;

    bool _focusEnabled = camConfig->CameraFocusEnabled;
    bool _manualFocus = camConfig->CameraManualFocus;
    *focusEnabled = _focusEnabled;
    *manualFocus = _manualFocus;
    uint16_t _manualFocusLevel = camConfig->CameraManualFocusLevel;
    bool _zoomEnabled = camConfig->ImageZoomEnabled;
    bool _reloadZoomConfig = _zoomEnabled && _focusEnabled && !_manualFocus;
    *needReloadZoomConfig = _reloadZoomConfig;

    // Autofocus does not work with zoom as it constantly overwrites zoom
    // registers until autofocus is released. As a workaround, we disable
    // zoom, run autofocus to find the focus level and switch to manual
    // focus to capture image with zoom.
    ret = ret | configureSensor(camConfig);
    if (_reloadZoomConfig)
        ResetZoomSizeOnCamera(camConfig);
    SetCamFocus(_focusEnabled, _manualFocus, _manualFocusLevel);
    if (_reloadZoomConfig)
    {
        ReleaseCamFocus(_focusEnabled, _manualFocus);
        SetCamFocus(_focusEnabled, true, CCstatus.CameraFocusLevel);
        SetQualityZoomSize(camConfig);
    }

    return ret;
}

// only available on OV3660 and OV5640
// https://github.com/espressif/esp32-camera/issues/672
int CCamera::CameraDeepSleep(bool sleep)
{
    int ret = 0;
    if (CCstatus.CameraDeepSleepEnable != sleep)
    {
        CCstatus.CameraDeepSleepEnable = sleep;

        sensor_t *s = esp_camera_sensor_get();

        if (s != NULL)
        {
            if (CCstatus.CamSensor_id == OV2640_PID)
            {
                // OV2640 (Normal mode >>> Standby mode = OK), (Standby mode >>> Normal mode = n.OK)
                // ret |= s->set_reg(s, 0x109, 0x10, enable ? 0x10 : 0);
                // LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "DeepSleep is not supported by OV2640");
                uint8_t reg = s->get_reg(s, 0x09, 0xff);
                ret = s->set_reg(s, 0x09, 0xff, sleep ? (reg |= 0x10) : (reg &= ~0x10));
            }
            else
            {
                ret = s->set_reg(s, 0x3008, 0x42, sleep ? 0x42 : 0x02);
            }

            std::string state = sleep ? "enabled" : "disabled";
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "DeepSleep: " + state);

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else
        {
            return -1;
        }
    }

    return ret;
}

int CCamera::SetCamFocus(bool focusEnabled, bool manualFocus, uint16_t manualFocusLevel)
{
    CameraDeepSleep(false);

    int ret = 0;

    if (CCstatus.CamSensor_id == OV5640_PID && focusEnabled)
    {
        if (initCameraAF())
        {
            ESP_LOGI(TAG, "OV5640 and AF inited");
            sensor_t *s = esp_camera_sensor_get();
            if (s != NULL)
            {
                if (manualFocus)
                {
                    ret = ov5640_manual_focus_set(s, manualFocusLevel);
                    if (ret == 0)
                    {
                        ESP_LOGI(TAG, "Set manual focus level %u success", manualFocusLevel);
                        CCstatus.CameraFocusLevel = manualFocusLevel;
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Set manual focus level %u failed: %d", manualFocusLevel, ret);
                    }
                }
                else
                {
                    ret = ov5640_autofocus_set_mode(s, AF_TRIG_SINGLE_AUTO_FOCUS);
                    if (ret == 0)
                    {
                        ESP_LOGI(TAG, "Set single autofocus mode success");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Set single autofocus mode failed: %d", ret);
                        return ret;
                    }

                    ESP_LOGI(TAG, "adjusting focus");

                    camera_fb_t *fb = esp_camera_fb_get();
                    uint8_t S_Zone[5];
                    uint8_t focus_status = ov5640_autofocus_get_status(s, S_Zone, 5);
                    for (int z = 0; z < 5; z++)
                    {
                        ESP_LOGI(TAG, "Zone[%d]: 0x%02x", z, S_Zone[z]);
                    }
                    esp_camera_fb_return(fb);
                    if (focus_status == FW_STATUS_S_FOCUSING)
                    {
                        ESP_LOGI(TAG, "Focusing: 0x%02x", focus_status);
                    }
                    else if (focus_status == FW_STATUS_S_FOCUSED)
                    {
                        ESP_LOGI(TAG, "Focused: 0x%02x", focus_status);
                        CCstatus.CameraFocusLevel = ov5640_get_focus_level(s);
                        ESP_LOGI(TAG, "Focus level: %u", CCstatus.CameraFocusLevel);
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Focus status 0x%02x", focus_status);
                    }
                }
            }
            else
            {
                ret = -1;
            }
        }
    }
    return ret;
}

int CCamera::ReleaseCamFocus(bool focusEnabled, bool manualFocus)
{
    CameraDeepSleep(false);

    int ret = 0;

    if (CCstatus.CamSensor_id == OV5640_PID && focusEnabled)
    {
        if (CCstatus.CameraAFInitSuccessful)
        {
            sensor_t *s = esp_camera_sensor_get();
            if (s != NULL)
            {
                if (manualFocus)
                {
                    ret = ov5640_manual_focus_release(s);
                    if (ret == 0)
                    {
                        ESP_LOGI(TAG, "Release manual focus success");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Release manual focus failed: %d", ret);
                    }
                }
                else
                {
                    ret = ov5640_autofocus_release(s);
                    if (ret == 0)
                    {
                        ESP_LOGI(TAG, "Release autofocus success");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Release autofocus failed: %d", ret);
                    }
                }
            }
            else
            {
                ret = -1;
            }
        }
    }
    return ret;
}

// on the OV5640, gainceiling must be set with the real value (x2>>>gainceilingLevel = 2, .... x128>>>gainceilingLevel = 128)
int CCamera::SetCamGainceiling(sensor_t *s, gainceiling_t gainceilingLevel)
{
    CameraDeepSleep(false);

	int ret = 0;
		
    if (CCstatus.CamSensor_id == OV2640_PID)
    {
        ret = s->set_gainceiling(s, gainceilingLevel); // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
    }
    else
    {
        int _level = (1 << ((int)gainceilingLevel + 1));

        ret = s->set_reg(s, 0x3A18, 0xFF, (_level >> 8) & 3) || s->set_reg(s, 0x3A19, 0xFF, _level & 0xFF);

        if (ret == 0)
        {
            // ESP_LOGD(TAG, "Set gainceiling to: %d", gainceilingLevel);
            s->status.gainceiling = gainceilingLevel;
        }
    }

    return ret;
}

void CCamera::SetCamSharpness(bool autoSharpnessEnabled, int sharpnessLevel)
{
    CameraDeepSleep(false);
 
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        if (CCstatus.CamSensor_id == OV2640_PID)
        {
            sharpnessLevel = min(2, max(-2, sharpnessLevel));
            // The OV2640 does not officially support sharpness, so the detour is made with the ov2640_sharpness.cpp.
            if (autoSharpnessEnabled)
            {
                ov2640_enable_auto_sharpness(s);
            }
            else
            {
                ov2640_set_sharpness(s, sharpnessLevel);
            }
        }
        else
        {
            sharpnessLevel = min(3, max(-3, sharpnessLevel));
            // for CAMERA_OV5640 and CAMERA_OV3660
            if (autoSharpnessEnabled)
            {
                // autoSharpness is not supported, default to zero
                s->set_sharpness(s, 0);
            }
            else
            {
                s->set_sharpness(s, sharpnessLevel);
            }
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SetCamSharpness, Failed to get Cam control structure");
    }
}

void CCamera::SetCamSpecialEffect(sensor_t *s, int specialEffect)
{
    CameraDeepSleep(false);

    if (CCstatus.CamSensor_id == OV2640_PID)
    {
        ov2640_set_special_effect(s, specialEffect);
    }
    else
    {
        s->set_special_effect(s, specialEffect);
    }
}

void CCamera::SetCamContrastBrightness(sensor_t *s, int _contrast, int _brightness)
{
    CameraDeepSleep(false);

    if (CCstatus.CamSensor_id == OV2640_PID)
    {
        ov2640_set_contrast_brightness(s, _contrast, _brightness);
    }
    else
    {
        s->set_contrast(s, _contrast);     // -2 to 2
        s->set_brightness(s, _brightness); // -2 to 2
    }
}

// - It always zooms to the image center when offsets are zero
// - if imageSize = 0 then the image is not zoomed
// - if imageSize = max value, then the image is fully zoomed in
// - a zoom step is >>> Width + 32 px / Height + 24 px
void CCamera::SanitizeZoomParams(cam_config_t *camConfig, int imageSize, int frameSizeX, int frameSizeY, int &imageWidth, int &imageHeight, int &zoomOffsetX, int &zoomOffsetY)
{
    // for OV2640, This works only if the aspect ratio of 4:3 is preserved in the window size.
    // use only values divisible by 8 without remainder
    imageWidth = camConfig->ImageWidth + (imageSize * 4 * 8);
    imageHeight = camConfig->ImageHeight + (imageSize * 3 * 8);

    int _maxX = frameSizeX - imageWidth;
    int _maxY = frameSizeY - imageHeight;

    if ((abs(zoomOffsetX) * 2) > _maxX)
    {
        if (zoomOffsetX > 0)
        {
            zoomOffsetX = _maxX;
        }
        else
        {
            zoomOffsetX = 0;
        }
    }
    else
    {
        if (zoomOffsetX > 0)
        {
            zoomOffsetX = ((_maxX / 2) + zoomOffsetX);
        }
        else
        {
            zoomOffsetX = ((_maxX / 2) + zoomOffsetX);
        }
    }

    if ((abs(zoomOffsetY) * 2) > _maxY)
    {
        if (zoomOffsetY > 0)
        {
            zoomOffsetY = _maxY;
        }
        else
        {
            zoomOffsetY = 0;
        }
    }
    else
    {
        if (zoomOffsetY > 0)
        {
            zoomOffsetY = ((_maxY / 2) + zoomOffsetY);
        }
        else
        {
            zoomOffsetY = ((_maxY / 2) + zoomOffsetY);
        }
    }
}

void CCamera::SetZoomSize(cam_config_t *camConfig, bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize, int imageVflip)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        if (zoomEnabled)
        {
            int _imageSize_temp = 0;
            int _imageWidth = camConfig->ImageWidth;
            int _imageHeight = camConfig->ImageHeight;
            int _offsetx = zoomOffsetX;
            int _offsety = zoomOffsetY;
            int frameSizeX;
            int frameSizeY;

            switch (CCstatus.CamSensor_id)
            {
            case OV5640_PID:
                frameSizeX = 2592;
                frameSizeY = 1944;
                // max imageSize = ((frameSizeX - CCstatus.ImageWidth) / 8 / 4) - 1
                // 59 = ((2560 - 640) / 8 / 4) - 1
                if (imageSize < 59)
                {
                    _imageSize_temp = (59 - imageSize);
                }
                SanitizeZoomParams(camConfig, _imageSize_temp, frameSizeX, frameSizeY, _imageWidth, _imageHeight, _offsetx, _offsety);
                SetCamWindow(s, frameSizeX, frameSizeY, _offsetx, _offsety, _imageWidth, _imageHeight, camConfig->ImageWidth, camConfig->ImageHeight, imageVflip);
                break;

            case OV3660_PID:
                frameSizeX = 2048;
                frameSizeY = 1536;
                // max imageSize = ((frameSizeX - CCstatus.ImageWidth) / 8 / 4) -1
                // 43 = ((2048 - 640) / 8 / 4) - 1
                if (imageSize < 43)
                {
                    _imageSize_temp = (43 - imageSize);
                }
                SanitizeZoomParams(camConfig, _imageSize_temp, frameSizeX, frameSizeY, _imageWidth, _imageHeight, _offsetx, _offsety);
                SetCamWindow(s, frameSizeX, frameSizeY, _offsetx, _offsety, _imageWidth, _imageHeight, camConfig->ImageWidth, camConfig->ImageHeight, imageVflip);
                break;

            case OV2640_PID:
                frameSizeX = 1600;
                frameSizeY = 1200;
                // max imageSize = ((frameSizeX - CCstatus.ImageWidth) / 8 / 4) -1
                // 29 = ((1600 - 640) / 8 / 4) - 1
                if (imageSize < 29)
                {
                    _imageSize_temp = (29 - imageSize);
                }
                SanitizeZoomParams(camConfig, _imageSize_temp, frameSizeX, frameSizeY, _imageWidth, _imageHeight, _offsetx, _offsety);
                SetCamWindow(s, frameSizeX, frameSizeY, _offsetx, _offsety, _imageWidth, _imageHeight, camConfig->ImageWidth, camConfig->ImageHeight, imageVflip);
                break;

            default:
                // do nothing
                break;
            }
        }
        else
        {
            s->set_framesize(s, camConfig->ImageFrameSize);
        }
    }
}

void CCamera::SetQualityZoomSize(cam_config_t *camConfig)
{
    int qual = camConfig->ImageQuality;
    framesize_t resol = camConfig->ImageFrameSize;
    bool zoomEnabled = camConfig->ImageZoomEnabled;
    int zoomOffsetX = camConfig->ImageZoomOffsetX;
    int zoomOffsetY = camConfig->ImageZoomOffsetY;
    int imageSize = camConfig->ImageZoomSize;
    int imageVflip = camConfig->ImageVflip;

    CameraDeepSleep(false);

    sensor_t *s = esp_camera_sensor_get();

    // OV2640 has no lower limit on jpeg quality
    if (CCstatus.CamSensor_id == OV5640_PID)
    {
        qual = min(63, max(12, qual));
    }

    SetImageWidthHeightFromResolution(camConfig, resol);

    if (s != NULL)
    {
        s->set_quality(s, qual);
        SetZoomSize(camConfig, zoomEnabled, zoomOffsetX, zoomOffsetY, imageSize, imageVflip);
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SetQualityZoomSize, Failed to get Cam control structure");
    }
}

void CCamera::ResetZoomSizeOnCamera(cam_config_t *camConfig)
{
    CameraDeepSleep(false);

    SetZoomSize(camConfig, false, 0, 0, 0, 0);
}

void CCamera::SetCamWindow(sensor_t *s, int frameSizeX, int frameSizeY, int xOffset, int yOffset, int xTotal, int yTotal, int xOutput, int yOutput, int imageVflip)
{
    CameraDeepSleep(false);

    if (CCstatus.CamSensor_id == OV2640_PID)
    {
        s->set_res_raw(s, 0, 0, 0, 0, xOffset, yOffset, xTotal, yTotal, xOutput, yOutput, false, false);
    }
    else
    {
        // for CAMERA_OV5640 and CAMERA_OV3660
        bool scale = !(xOutput == xTotal && yOutput == yTotal);
        bool binning = (xTotal >= (frameSizeX >> 1));

        if (imageVflip == true)
        {
            s->set_res_raw(s, xOffset, yOffset, xOffset + xTotal - 1, yOffset + yTotal - 1, 0, 0, frameSizeX, frameSizeY, xOutput, yOutput, scale, binning);
        }
        else
        {
            s->set_res_raw(s, xOffset, yOffset, xOffset + xTotal, yOffset + yTotal, 0, 0, frameSizeX, frameSizeY, xOutput, yOutput, scale, binning);
        }
    }
}

static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len)
{
    jpg_chunking_t *j = (jpg_chunking_t *)arg;

    if (!index)
    {
        j->len = 0;
    }

    if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK)
    {
        return 0;
    }

    j->len += len;

    return len;
}

esp_err_t CCamera::CaptureToBasisImage(CImageBasis *_Image, int delay)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CaptureToBasisImage - Start");
#endif

    _Image->EmptyImage(); // Delete previous stored raw image -> black image

    LEDOnOff(true); // Status-LED on
    if (delay > 0)
    {
        LightOnOff(true); // Flash-LED on
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CaptureToBasisImage - After LightOn");
#endif

    camera_fb_t *fb = NULL;
    bool _focusEnabled = false;
    bool _manualFocus = false;
    bool _reloadZoomConfig = false;
    PrecaptureCamSetup(&_focusEnabled, &_manualFocus, &_reloadZoomConfig);

    if (delay > 0)
    {
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = esp_camera_fb_get();

    ReleaseCamFocus(_focusEnabled, _reloadZoomConfig ? true : _manualFocus);

    if (!fb)
    {
        LEDOnOff(false);   // Status-LED off
        LightOnOff(false); // Flash-LED off

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "is not working anymore (CaptureToBasisImage) - most probably caused "
                                                "by a hardware problem (instablility, ...). System will reboot.");
        CameraDeepSleep(true);
        doReboot();

        return ESP_FAIL;
    }

    if (CCstatus.DemoMode)
    {
        // Use images stored on SD-Card instead of camera image
        /* Replace Framebuffer with image from SD-Card */
        loadNextDemoImage(fb);
    }

    CImageBasis *_zwImage = new CImageBasis("zwImage");

    if (_zwImage)
    {
        _zwImage->LoadFromMemory(fb->buf, fb->len);
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: Can't allocate _zwImage");
    }

    esp_camera_fb_return(fb);
    CameraDeepSleep(true);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CaptureToBasisImage - After fb_get");
#endif

    LEDOnOff(false); // Status-LED off

    if (delay > 0)
    {
        LightOnOff(false); // Flash-LED off
    }

    //    TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    //    vTaskDelay( xDelay );  // wait for power to recover

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CaptureToBasisImage - After LoadFromMemory");
#endif

    if (_zwImage == NULL)
    {
        return ESP_OK;
    }

    stbi_uc *p_target;
    stbi_uc *p_source;
    int channels = 3;
    int width = CCstatus.CamConfig.ImageWidth;
    int height = CCstatus.CamConfig.ImageHeight;

#ifdef DEBUG_DETAIL_ON
    std::string _zw = "Targetimage: " + std::to_string((int)_Image->rgb_image) + " Size: " + std::to_string(_Image->width) + ", " + std::to_string(_Image->height);
    _zw = _zw + " _zwImage: " + std::to_string((int)_zwImage->rgb_image) + " Size: " + std::to_string(_zwImage->width) + ", " + std::to_string(_zwImage->height);
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, _zw);
#endif

    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            p_target = _Image->rgb_image + (channels * (y * width + x));
            p_source = _zwImage->rgb_image + (channels * (y * width + x));

            for (int c = 0; c < channels; c++)
            {
                p_target[c] = p_source[c];
            }
        }
    }

    delete _zwImage;

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CaptureToBasisImage - Done");
#endif

    return ESP_OK;
}

esp_err_t CCamera::CaptureToFile(std::string nm, int delay)
{
    LEDOnOff(true); // Status-LED on

    if (delay > 0)
    {
        LightOnOff(true); // Flash-LED on
    }

    camera_fb_t *fb = NULL;
    bool _focusEnabled = false;
    bool _manualFocus = false;
    bool _reloadZoomConfig = false;
    PrecaptureCamSetup(&_focusEnabled, &_manualFocus, &_reloadZoomConfig);

    if (delay > 0)
    {
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = esp_camera_fb_get();

    ReleaseCamFocus(_focusEnabled, _reloadZoomConfig ? true : _manualFocus);

    if (!fb)
    {
        LEDOnOff(false);   // Status-LED off
        LightOnOff(false); // Flash-LED off
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: Capture Failed. "
                                                "Check camera module and/or proper electrical connection");
        CameraDeepSleep(true);
        // doReboot();

        return ESP_FAIL;
    }

    LEDOnOff(false); // Status-LED off

#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "w %d, h %d, size %d", fb->width, fb->height, fb->len);
#endif

    nm = FormatFileName(nm);

#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "Save Camera to: %s", nm.c_str());
#endif

    std::string ftype = toUpper(getFileType(nm));

#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "Filetype: %s", ftype.c_str());
#endif

    uint8_t *buf = NULL;
    size_t buf_len = 0;
    bool converted = false;

    if (ftype.compare("BMP") == 0)
    {
        frame2bmp(fb, &buf, &buf_len);
        converted = true;
    }

    if (ftype.compare("JPG") == 0)
    {
        if (fb->format != PIXFORMAT_JPEG)
        {
            bool jpeg_converted = frame2jpg(fb, CCstatus.CamConfig.ImageQuality, &buf, &buf_len);
            converted = true;

            if (!jpeg_converted)
            {
                ESP_LOGE(TAG, "JPEG compression failed");
            }
        }
        else
        {
            buf_len = fb->len;
            buf = fb->buf;
        }
    }

    FILE *fp = fopen(nm.c_str(), "wb");

    if (fp == NULL)
    {
        // If an error occurs during the file creation
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: Failed to open file " + nm);
    }
    else
    {
        fwrite(buf, sizeof(uint8_t), buf_len, fp);
        fclose(fp);
    }

    if (converted)
    {
        free(buf);
    }

    esp_camera_fb_return(fb);
    CameraDeepSleep(true);

    if (delay > 0)
    {
        LightOnOff(false); // Flash-LED off
    }

    return ESP_OK;
}

esp_err_t CCamera::CaptureToHTTP(httpd_req_t *req, int delay)
{
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();

    LEDOnOff(true); // Status-LED on

    if (delay > 0)
    {
        LightOnOff(true); // Flash-LED on
    }

    camera_fb_t *fb = NULL;
    bool _focusEnabled = false;
    bool _manualFocus = false;
    bool _reloadZoomConfig = false;
    PrecaptureCamSetup(&_focusEnabled, &_manualFocus, &_reloadZoomConfig);

    if (delay > 0)
    {
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = esp_camera_fb_get();

    ReleaseCamFocus(_focusEnabled, _reloadZoomConfig ? true : _manualFocus);

    if (!fb)
    {
        LEDOnOff(false);   // Status-LED off
        LightOnOff(false); // Flash-LED off
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: Capture Failed. "
                                                "Check camera module and/or proper electrical connection");
        httpd_resp_send_500(req);
        CameraDeepSleep(true);
        //        doReboot();

        return ESP_FAIL;
    }

    LEDOnOff(false); // Status-LED off
    res = httpd_resp_set_type(req, "image/jpeg");

    if (res == ESP_OK)
    {
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=raw.jpg");
    }

    if (res == ESP_OK)
    {
        if (CCstatus.DemoMode)
        {
            // Use images stored on SD-Card instead of camera image
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Using Demo image!");
            /* Replace Framebuffer with image from SD-Card */
            loadNextDemoImage(fb);

            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        }
        else
        {
            if (fb->format == PIXFORMAT_JPEG)
            {
                fb_len = fb->len;
                res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
            }
            else
            {
                jpg_chunking_t jchunk = {req, 0};
                res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
                httpd_resp_send_chunk(req, NULL, 0);
                fb_len = jchunk.len;
            }
        }
    }

    esp_camera_fb_return(fb);
    CameraDeepSleep(true);
    int64_t fr_end = esp_timer_get_time();

    ESP_LOGI(TAG, "JPG: %dKB %dms", (int)(fb_len / 1024), (int)((fr_end - fr_start) / 1000));

    if (delay > 0)
    {
        LightOnOff(false); // Flash-LED off
    }

    return res;
}

esp_err_t CCamera::CaptureToStream(httpd_req_t *req, bool FlashlightOn)
{
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start;
    char *part_buf[64];

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Live stream started");

    if (FlashlightOn)
    {
        LEDOnOff(true);   // Status-LED on
        LightOnOff(true); // Flash-LED on
    }

    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");  //stream is blocking web interface, only serving to local

    httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));

    bool _focusEnabled = false;
    bool _manualFocus = false;
    bool _reloadZoomConfig = false;
    PrecaptureCamSetup(&_focusEnabled, &_manualFocus, &_reloadZoomConfig);

    while (1)
    {
        fr_start = esp_timer_get_time();
        camera_fb_t *fb = esp_camera_fb_get();
        esp_camera_fb_return(fb);
        fb = esp_camera_fb_get();

        if (!fb)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToStream: Camera framebuffer not available");
            break;
        }

        fb_len = fb->len;

        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, sizeof(part_buf), _STREAM_PART, fb_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }

        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb_len);
        }

        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }

        esp_camera_fb_return(fb);

        int64_t fr_end = esp_timer_get_time();
        ESP_LOGD(TAG, "JPG: %dKB %dms", (int)(fb_len / 1024), (int)((fr_end - fr_start) / 1000));

        if (res != ESP_OK)
        {
            // Exit loop, e.g. also when closing the webpage
            break;
        }

        int64_t fr_delta_ms = (fr_end - fr_start) / 1000;

        if (CAM_LIVESTREAM_REFRESHRATE > fr_delta_ms)
        {
            const TickType_t xDelay = (CAM_LIVESTREAM_REFRESHRATE - fr_delta_ms) / portTICK_PERIOD_MS;
            ESP_LOGD(TAG, "Stream: sleep for: %ldms", (long)xDelay * 10);
            vTaskDelay(xDelay);
        }
    }

    ReleaseCamFocus(_focusEnabled, _reloadZoomConfig ? true : _manualFocus);
    CameraDeepSleep(true);

    LEDOnOff(false);   // Status-LED off
    LightOnOff(false); // Flash-LED off

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Live stream stopped");

    return res;
}

void CCamera::LightOnOff(bool status)
{
    GpioHandler *gpioHandler = gpio_handler_get();

    if ((gpioHandler != NULL) && (gpioHandler->isEnabled()))
    {
        ESP_LOGD(TAG, "Use gpioHandler to trigger flashlight");
        gpioHandler->flashLightEnable(status);
    }
    else
    {
#ifdef USE_PWM_LEDFLASH
        if (status)
        {
            ESP_LOGD(TAG, "Internal Flash-LED turn on with PWM %d", CCstatus.CamConfig.ImageLedIntensity);
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, CCstatus.CamConfig.ImageLedIntensity));
            // Update duty to apply the new value
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        }
        else
        {
            ESP_LOGD(TAG, "Internal Flash-LED turn off PWM");
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        }
#else
        // Init the GPIO
        gpio_pad_select_gpio(FLASH_GPIO);

        // Set the GPIO as a push/pull output
        gpio_set_direction(FLASH_GPIO, GPIO_MODE_OUTPUT);

        if (status)
        {
            gpio_set_level(FLASH_GPIO, 1);
        }
        else
        {
            gpio_set_level(FLASH_GPIO, 0);
        }
#endif
    }
}

void CCamera::LEDOnOff(bool status)
{
    if (xHandle_task_StatusLED == NULL)
    {
        // Init the GPIO
        gpio_pad_select_gpio(BLINK_GPIO);

        /* Set the GPIO as a push/pull output */
        gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

        if (!status)
        {
            gpio_set_level(BLINK_GPIO, 1);
        }
        else
        {
            gpio_set_level(BLINK_GPIO, 0);
        }
    }
}

void CCamera::SetImageWidthHeightFromResolution(cam_config_t *camConfig, framesize_t resol)
{
    if (resol == FRAMESIZE_QVGA)
    {
        camConfig->ImageWidth = 320;
        camConfig->ImageHeight = 240;
    }
    else if (resol == FRAMESIZE_VGA)
    {
        camConfig->ImageWidth = 640;
        camConfig->ImageHeight = 480;
    }
    else if (resol == FRAMESIZE_SVGA)
    {
        camConfig->ImageWidth = 800;
        camConfig->ImageHeight = 600;
    }
    else if (resol == FRAMESIZE_XGA)
    {
        camConfig->ImageWidth = 1024;
        camConfig->ImageHeight = 768;
    }
    else if (resol == FRAMESIZE_HD)
    {
        camConfig->ImageWidth = 1280;
        camConfig->ImageHeight = 720;
    }
    else if (resol == FRAMESIZE_SXGA)
    {
        camConfig->ImageWidth = 1280;
        camConfig->ImageHeight = 1024;
    }
    else if (resol == FRAMESIZE_UXGA)
    {
        camConfig->ImageWidth = 1600;
        camConfig->ImageHeight = 1200;
    }
    else if (resol == FRAMESIZE_QXGA)
    {
        camConfig->ImageWidth = 2048;
        camConfig->ImageHeight = 1536;
    }
    else if (resol == FRAMESIZE_WQXGA)
    {
        camConfig->ImageWidth = 2560;
        camConfig->ImageHeight = 1600;
    }
    else if (resol == FRAMESIZE_QSXGA)
    {
        camConfig->ImageWidth = 2560;
        camConfig->ImageHeight = 1920;
    }
    else
    {
        camConfig->ImageWidth = 640;
        camConfig->ImageHeight = 480;
    }
}

framesize_t CCamera::TextToFramesize(const char *_size)
{
    if (strcmp(_size, "QVGA") == 0)
    {
        return FRAMESIZE_QVGA; // 320x240
    }
    else if (strcmp(_size, "VGA") == 0)
    {
        return FRAMESIZE_VGA; // 640x480
    }
    else if (strcmp(_size, "SVGA") == 0)
    {
        return FRAMESIZE_SVGA; // 800x600
    }
    else if (strcmp(_size, "XGA") == 0)
    {
        return FRAMESIZE_XGA; // 1024x768
    }
    else if (strcmp(_size, "SXGA") == 0)
    {
        return FRAMESIZE_SXGA; // 1280x1024
    }
    else if (strcmp(_size, "UXGA") == 0)
    {
        return FRAMESIZE_UXGA; // 1600x1200
    }
    else if (strcmp(_size, "QXGA") == 0)
    {
        return FRAMESIZE_QXGA; // 2048x1536
    }
    else if (strcmp(_size, "WQXGA") == 0)
    {
        return FRAMESIZE_WQXGA; // 2560x1600
    }
    else if (strcmp(_size, "QSXGA") == 0)
    {
        return FRAMESIZE_QSXGA; // 2560x1920
    }
    else
    {
        return FRAMESIZE_VGA; // 640x480
    }

    // return CCstatus.CamConfig.ImageFrameSize;
}

std::vector<std::string> demoFiles;

void CCamera::useDemoMode(void)
{
    char line[50];

    FILE *fd = fopen("/sdcard/demo/files.txt", "r");

    if (!fd)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can not start Demo mode, the folder '/sdcard/demo/' does not contain the needed files!");
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "See Details on https://jomjol.github.io/AI-on-the-edge-device-docs/Demo-Mode!");
        return;
    }

    demoImage = (uint8_t *)malloc(DEMO_IMAGE_SIZE);

    if (demoImage == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Unable to acquire required memory for demo image!");
        return;
    }

    while (fgets(line, sizeof(line), fd) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        demoFiles.push_back(line);
    }

    fclose(fd);

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Using Demo mode (" + std::to_string(demoFiles.size()) + " files) instead of real camera image!");

    for (auto file : demoFiles)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, file);
    }

    CCstatus.DemoMode = true;
}

bool CCamera::loadNextDemoImage(camera_fb_t *fb)
{
    char filename[50];
    int readBytes;
    long fileSize;

    snprintf(filename, sizeof(filename), "/sdcard/demo/%s", demoFiles[getCountFlowRounds() % demoFiles.size()].c_str());

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Using " + std::string(filename) + " as demo image");

    /* Inject saved image */

    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to read file: " + std::string(filename) + "!");
        return false;
    }

    fileSize = GetFileSize(filename);

    if (fileSize > DEMO_IMAGE_SIZE)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "Demo Image (%d bytes) is larger than provided buffer (%d bytes)!", (int)fileSize, DEMO_IMAGE_SIZE);
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, std::string(buf));
        return false;
    }

    readBytes = fread(demoImage, 1, DEMO_IMAGE_SIZE, fp);
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "read " + std::to_string(readBytes) + " bytes");
    fclose(fp);

    fb->buf = demoImage; // Update pointer
    fb->len = readBytes;
    // ToDo do we also need to set height, width, format and timestamp?

    return true;
}

long CCamera::GetFileSize(std::string filename)
{
    struct stat stat_buf;
    long rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}
