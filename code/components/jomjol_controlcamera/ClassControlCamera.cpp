#include "ClassControlCamera.h"
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

#include "defines.h"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <limits.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "server_camera.h"
#include "esp_camera.h"

#include "driver/ledc.h"
#include "MainFlowControl.h"

#include "ov2640_sharpness.h"
#include "ov2640_specialEffect.h"
#include "ov2640_contrast_brightness.h"

static const char *TAG = "CAM";

CCamera Camera;

camera_control_config_temp_t CCstatus;
camera_control_config_temp_t CFstatus;

/* Camera live stream */
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

uint8_t *demoImage = NULL;    // Buffer holding the demo image in bytes
#define DEMO_IMAGE_SIZE 30000 // Max size of demo image in bytes
std::vector<std::string> demoFiles;

// Camera module bus communications frequency.
// Originally: config.xclk_freq_mhz = 20000000, but this lead to visual artifacts on many modules.
// See https://github.com/espressif/esp32-camera/issues/150#issuecomment-726473652 et al.
#if !defined(XCLK_FREQ_MHZ)
int xclk = 16;
// int xclk = 20; // Orginal value
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
    .ledc_timer = CAM_XCLK_TIMER,     // LEDC timer to be used for generating XCLK
    .ledc_channel = CAM_XCLK_CHANNEL, // LEDC channel to be used for generating XCLK

    .pixel_format = PIXFORMAT_JPEG,    // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_VGA,       // QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    .jpeg_quality = 12,                // 0-63 lower number means higher quality
    .fb_count = 1,                     // if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM, /*!< The location where the frame buffer will be allocated */
    .grab_mode = CAMERA_GRAB_LATEST,   // only from new esp32cam version
    .sccb_i2c_port = -1,
};

typedef struct
{
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

CCamera::CCamera(void)
{
    CCstatus.ImageQuality = camera_config.jpeg_quality;
    CCstatus.ImageFrameSize = camera_config.frame_size;

    CCstatus.CamXclkFreqMhz = xclk;

    set_led_intensity(LedIntensity);

    CCstatus.WaitBeforePicture = 5;

    ledc_init();
}

esp_err_t CCamera::init_camera(void)
{
    ESP_LOGD(TAG, "Init Camera");

    // De-init in case it was already initialized
    power_reset_camera();
    vTaskDelay(pdMS_TO_TICKS(200));

    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    CameraInitSuccessful = true;

    // Get a reference to the sensor
    sensor_t *cam_sensor = esp_camera_sensor_get();
    if (cam_sensor != NULL)
    {
        CamSensor_id = cam_sensor->id.PID;

        // Dump camera module, warn for unsupported modules.
        switch (CamSensor_id)
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
            CameraInitSuccessful = false;
        }
    }

    if (CameraInitSuccessful)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

bool CCamera::test_camera(void)
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

void CCamera::power_reset_camera(void)
{
    // Use reset only if pin is available
    if (CAM_PIN_PWDN == GPIO_NUM_NC)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "No power down pin availbale to reset camera");
    }
    else
    {
        ESP_LOGD(TAG, "Resetting camera by power down line");
        gpio_config_t conf = {
            .pin_bit_mask = 1ULL << CAM_PIN_PWDN,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&conf);

        // carefull, logic is inverted compared to reset pin
        ESP_LOGD(TAG, "Camera power down");
        gpio_set_level(CAM_PIN_PWDN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        ESP_LOGD(TAG, "Camera power up");
        gpio_set_level(CAM_PIN_PWDN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
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

int CCamera::set_led_intensity(int _intrel)
{
    Camera.LedIntensity = (int)((float)(std::min(std::max(0, _intrel), 100)) / 100 * 8191);
    ESP_LOGD(TAG, "Set led_intensity to %i of 8191", Camera.LedIntensity);

    return Camera.LedIntensity;
}

bool CCamera::get_camera_init_successful(void)
{
    return CameraInitSuccessful;
}

esp_err_t CCamera::set_sensor_control_config(camera_control_config_temp_t *camConfig)
{
    sensor_t *cam_sensor = esp_camera_sensor_get();
    if (cam_sensor != NULL)
    {
        cam_sensor->set_xclk(cam_sensor, CAM_XCLK_TIMER, camConfig->CamXclkFreqMhz);

        cam_sensor->set_framesize(cam_sensor, camConfig->ImageFrameSize);

        // cam_sensor->set_contrast(cam_sensor, camConfig->ImageContrast);     // -2 to 2
        // cam_sensor->set_brightness(cam_sensor, camConfig->ImageBrightness); // -2 to 2
        set_camera_contrast_brightness(cam_sensor, camConfig->ImageContrast, camConfig->ImageBrightness);

        cam_sensor->set_saturation(cam_sensor, camConfig->ImageSaturation); // -2 to 2

        cam_sensor->set_quality(cam_sensor, camConfig->ImageQuality); // 0 - 63

        // cam_sensor->set_gainceiling(cam_sensor, camConfig->ImageGainceiling); // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
        set_camera_gainceiling(cam_sensor, camConfig->ImageGainceiling);

        cam_sensor->set_gain_ctrl(cam_sensor, camConfig->ImageAgc);     // 0 = disable , 1 = enable
        cam_sensor->set_exposure_ctrl(cam_sensor, camConfig->ImageAec); // 0 = disable , 1 = enable
        cam_sensor->set_hmirror(cam_sensor, camConfig->ImageHmirror);   // 0 = disable , 1 = enable
        cam_sensor->set_vflip(cam_sensor, camConfig->ImageVflip);       // 0 = disable , 1 = enable

        cam_sensor->set_whitebal(cam_sensor, camConfig->ImageAwb);       // 0 = disable , 1 = enable
        cam_sensor->set_aec2(cam_sensor, camConfig->ImageAec2);          // 0 = disable , 1 = enable
        cam_sensor->set_aec_value(cam_sensor, camConfig->ImageAecValue); // 0 to 1200
        // cam_sensor->set_special_effect(cam_sensor, camConfig->ImageSpecialEffect); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        set_camera_special_effect(cam_sensor, camConfig->ImageSpecialEffect);
        cam_sensor->set_wb_mode(cam_sensor, camConfig->ImageWbMode);   // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        cam_sensor->set_ae_level(cam_sensor, camConfig->ImageAeLevel); // -2 to 2

        cam_sensor->set_dcw(cam_sensor, camConfig->ImageDcw);          // 0 = disable , 1 = enable
        cam_sensor->set_bpc(cam_sensor, camConfig->ImageBpc);          // 0 = disable , 1 = enable
        cam_sensor->set_wpc(cam_sensor, camConfig->ImageWpc);          // 0 = disable , 1 = enable
        cam_sensor->set_awb_gain(cam_sensor, camConfig->ImageAwbGain); // 0 = disable , 1 = enable
        cam_sensor->set_agc_gain(cam_sensor, camConfig->ImageAgcGain); // 0 to 30

        cam_sensor->set_raw_gma(cam_sensor, camConfig->ImageRawGma); // 0 = disable , 1 = enable
        cam_sensor->set_lenc(cam_sensor, camConfig->ImageLenc);      // 0 = disable , 1 = enable

        // cam_sensor->set_sharpness(cam_sensor, camConfig->ImageSharpness);   // auto-sharpness is not officially supported, default to 0
        set_camera_sharpness(cam_sensor, camConfig->ImageAutoSharpness, camConfig->ImageSharpness);
        cam_sensor->set_denoise(cam_sensor, camConfig->ImageDenoise); // The OV2640 does not support it, OV3660 and OV5640 (0 to 8)

        vTaskDelay(100 / portTICK_PERIOD_MS);

        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

esp_err_t CCamera::get_sensor_control_config(camera_control_config_temp_t *camConfig)
{
    sensor_t *cam_sensor = esp_camera_sensor_get();
    if (cam_sensor != NULL)
    {
        // Since there are always errors when reading out the parameters
        // and therefore problems when initializing the camera with the wrong parameters,
        // the parameters are checked and, if necessary, set with a default value.
        // note: The read value is usually "uint8_t" (see:
        // https://github.com/espressif/esp32-camera/blob/dba8da9898928d9808d57a0b0cdcde9f130ed8fe/driver/include/sensor.h#L174#L203)
        // However, the one to be set is usually "int" (see:
        // https://github.com/espressif/esp32-camera/blob/dba8da9898928d9808d57a0b0cdcde9f130ed8fe/driver/include/sensor.h#L206#L253)

        CamSensor_id = cam_sensor->id.PID;

        camConfig->CamXclkFreqMhz = get_value_or_default_int((int)(cam_sensor->xclk_freq_hz / 1000000), 20, 1, 16);

        camConfig->ImageFrameSize = (framesize_t)get_value_or_default_uint((int)cam_sensor->status.framesize, 10, 10);

        camConfig->ImageContrast = get_value_or_default_int((int)cam_sensor->status.contrast, 2, -2, 0);
        camConfig->ImageBrightness = get_value_or_default_int((int)cam_sensor->status.brightness, 2, -2, 0);
        camConfig->ImageSaturation = get_value_or_default_int((int)cam_sensor->status.saturation, 2, -2, 0);

        camConfig->ImageQuality = get_value_or_default_int((int)cam_sensor->status.quality, 63, 6, 12);

        camConfig->ImageGainceiling = (gainceiling_t)get_value_or_default_uint((int)cam_sensor->status.gainceiling, 6, 1);

        camConfig->ImageAgc = get_value_or_default_uint((int)cam_sensor->status.agc, 1, 1);
        camConfig->ImageAec = get_value_or_default_uint((int)cam_sensor->status.aec, 1, 1);
        camConfig->ImageHmirror = get_value_or_default_uint((int)cam_sensor->status.hmirror, 1, 0);
        camConfig->ImageVflip = get_value_or_default_uint((int)cam_sensor->status.vflip, 1, 0);

        camConfig->ImageAwb = get_value_or_default_uint((int)cam_sensor->status.awb, 1, 1);
        camConfig->ImageAec2 = get_value_or_default_uint((int)cam_sensor->status.aec2, 1, 1);
        camConfig->ImageAecValue = get_value_or_default_uint((int)cam_sensor->status.aec_value, 1200, 600);
        camConfig->ImageSpecialEffect = get_value_or_default_uint((int)cam_sensor->status.special_effect, 6, 0);
        camConfig->ImageWbMode = get_value_or_default_uint((int)cam_sensor->status.wb_mode, 4, 0);
        camConfig->ImageAeLevel = get_value_or_default_int((int)cam_sensor->status.ae_level, 5, -5, 0);

        camConfig->ImageDcw = get_value_or_default_uint((int)cam_sensor->status.dcw, 1, 1);
        camConfig->ImageBpc = get_value_or_default_uint((int)cam_sensor->status.bpc, 1, 1);
        camConfig->ImageWpc = get_value_or_default_uint((int)cam_sensor->status.wpc, 1, 1);
        camConfig->ImageAwbGain = get_value_or_default_uint((int)cam_sensor->status.awb_gain, 1, 1);
        camConfig->ImageAgcGain = get_value_or_default_uint((int)cam_sensor->status.agc_gain, 30, 8);

        camConfig->ImageRawGma = get_value_or_default_uint((int)cam_sensor->status.raw_gma, 1, 1);
        camConfig->ImageLenc = get_value_or_default_uint((int)cam_sensor->status.lenc, 1, 1);

        camConfig->ImageSharpness = get_value_or_default_int((int)cam_sensor->status.sharpness, 2, -2, 0);
        camConfig->ImageDenoise = get_value_or_default_uint((int)cam_sensor->status.denoise, 8, 0);

        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

esp_err_t CCamera::set_camera_config_from_to(camera_control_config_temp_t *camConfigFrom, camera_control_config_temp_t *camConfigTo)
{
    camConfigTo->CamXclkFreqMhz = camConfigFrom->CamXclkFreqMhz;

    camConfigTo->ImageFrameSize = camConfigFrom->ImageFrameSize;

    camConfigTo->ImageContrast = camConfigFrom->ImageContrast;
    camConfigTo->ImageBrightness = camConfigFrom->ImageBrightness;
    camConfigTo->ImageSaturation = camConfigFrom->ImageSaturation;

    camConfigTo->ImageQuality = camConfigFrom->ImageQuality;

    camConfigTo->ImageGainceiling = camConfigFrom->ImageGainceiling;

    camConfigTo->ImageAgc = camConfigFrom->ImageAgc;
    camConfigTo->ImageAec = camConfigFrom->ImageAec;
    camConfigTo->ImageHmirror = camConfigFrom->ImageHmirror;
    camConfigTo->ImageVflip = camConfigFrom->ImageVflip;

    camConfigTo->ImageAwb = camConfigFrom->ImageAwb;
    camConfigTo->ImageAec2 = camConfigFrom->ImageAec2;
    camConfigTo->ImageAecValue = camConfigFrom->ImageAecValue;
    camConfigTo->ImageSpecialEffect = camConfigFrom->ImageSpecialEffect;
    camConfigTo->ImageWbMode = camConfigFrom->ImageWbMode;
    camConfigTo->ImageAeLevel = camConfigFrom->ImageAeLevel;

    camConfigTo->ImageDcw = camConfigFrom->ImageDcw;
    camConfigTo->ImageBpc = camConfigFrom->ImageBpc;
    camConfigTo->ImageWpc = camConfigFrom->ImageWpc;
    camConfigTo->ImageAwbGain = camConfigFrom->ImageAwbGain;
    camConfigTo->ImageAgcGain = camConfigFrom->ImageAgcGain;

    camConfigTo->ImageRawGma = camConfigFrom->ImageRawGma;
    camConfigTo->ImageLenc = camConfigFrom->ImageLenc;

    camConfigTo->ImageSharpness = camConfigFrom->ImageSharpness;
    camConfigTo->ImageAutoSharpness = camConfigFrom->ImageAutoSharpness;

    camConfigTo->ImageDenoise = camConfigFrom->ImageDenoise;

    camConfigTo->ImageLedIntensity = camConfigFrom->ImageLedIntensity;

    camConfigTo->ImageZoomEnabled = camConfigFrom->ImageZoomEnabled;
    camConfigTo->ImageZoomOffsetX = camConfigFrom->ImageZoomOffsetX;
    camConfigTo->ImageZoomOffsetY = camConfigFrom->ImageZoomOffsetY;
    camConfigTo->ImageZoomSize = camConfigFrom->ImageZoomSize;

    camConfigTo->WaitBeforePicture = camConfigFrom->WaitBeforePicture;

    return ESP_OK;
}

esp_err_t CCamera::check_camera_settings_changed(void)
{
    if (set_camera_deep_sleep(false) != ESP_OK)
    {
        return ESP_FAIL;
    }

    // If the camera settings were changed by creating a new reference image, they must be reset
    if (changedCameraSettings)
    {
        if (CamTempImage)
        {
            ESP_LOGD(TAG, "check_camera_settings_changed(): CFstatus");
            set_sensor_control_config(&CFstatus); // CFstatus >>> Kamera
            set_quality_zoom_size(&CFstatus);
            set_led_intensity(CFstatus.ImageLedIntensity);
            CamTempImage = false;
        }
        else
        {
            ESP_LOGD(TAG, "check_camera_settings_changed(): CCstatus");
            set_sensor_control_config(&CCstatus); // CCstatus >>> Kamera
            set_quality_zoom_size(&CCstatus);
            set_led_intensity(CCstatus.ImageLedIntensity);
            changedCameraSettings = false;
        }
    }

    return ESP_OK;
}

// only available on OV3660 and OV5640
// https://github.com/espressif/esp32-camera/issues/672
esp_err_t CCamera::set_camera_deep_sleep(bool status)
{
    if (CamDeepSleepEnable != status)
    {
        CamDeepSleepEnable = status;

        sensor_t *cam_sensor = esp_camera_sensor_get();
        if (cam_sensor != NULL)
        {
            std::string temp_status = "unsupported";
            if (CamSensor_id == OV2640_PID)
            {
                // OV2640 Standby mode
                uint8_t reg = cam_sensor->get_reg(cam_sensor, 0x09, 0xFF);
                cam_sensor->set_reg(cam_sensor, 0x09, 0xFF, status ? (reg |= 0x10) : (reg &= ~0x10));
                temp_status = status ? "enabled" : "disabled";
            }
            else if ((CamSensor_id == OV3660_PID) || (CamSensor_id == OV5640_PID))
            {
                // OV3660/OV5640 DeepSleep mode
                cam_sensor->set_reg(cam_sensor, 0x3008, 0x42, status ? 0x42 : 0x02);
                temp_status = status ? "enabled" : "disabled";
            }

            ESP_LOGD(TAG, "DeepSleep: %s", temp_status.c_str());
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else
        {
            ESP_LOGE(TAG, "set_camera_deep_sleep(), Failed to get Cam control structure");
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}

// on the OV5640, gainceiling must be set with the real value (x2>>>gainceilingLevel = 2, .... x128>>>gainceilingLevel = 128)
int CCamera::set_camera_gainceiling(sensor_t *cam_sensor, gainceiling_t gainceilingLevel)
{
    int ret = 0;

    if (CamSensor_id == OV2640_PID)
    {
        ret = cam_sensor->set_gainceiling(cam_sensor, gainceilingLevel); // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
    }
    else
    {
        int _level = (1 << ((int)gainceilingLevel + 1));

        ret = cam_sensor->set_reg(cam_sensor, 0x3A18, 0xFF, (_level >> 8) & 3) || cam_sensor->set_reg(cam_sensor, 0x3A19, 0xFF, _level & 0xFF);

        if (ret == 0)
        {
            // ESP_LOGD(TAG, "Set gainceiling to: %d", gainceilingLevel);
            cam_sensor->status.gainceiling = gainceilingLevel;
        }
    }

    return ret;
}

void CCamera::set_camera_sharpness(sensor_t *cam_sensor, bool autoSharpnessEnabled, int sharpnessLevel)
{
    if (CamSensor_id == OV2640_PID)
    {
        sharpnessLevel = min(2, max(-2, sharpnessLevel));
        // The OV2640 does not officially support sharpness, so the detour is made with the ov2640_sharpness.cpp.
        if (autoSharpnessEnabled)
        {
            ov2640_enable_auto_sharpness(cam_sensor);
        }
        else
        {
            ov2640_set_sharpness(cam_sensor, sharpnessLevel);
        }
    }
    else
    {
        sharpnessLevel = min(3, max(-3, sharpnessLevel));
        // for CAMERA_OV5640 and CAMERA_OV3660
        if (autoSharpnessEnabled)
        {
            // autoSharpness is not supported, default to zero
            cam_sensor->set_sharpness(cam_sensor, 0);
        }
        else
        {
            cam_sensor->set_sharpness(cam_sensor, sharpnessLevel);
        }
    }
}

void CCamera::set_camera_special_effect(sensor_t *cam_sensor, int specialEffect)
{
    if (CamSensor_id == OV2640_PID)
    {
        ov2640_set_special_effect(cam_sensor, specialEffect);
    }
    else
    {
        cam_sensor->set_special_effect(cam_sensor, specialEffect);
    }
}

void CCamera::set_camera_contrast_brightness(sensor_t *cam_sensor, int _contrast, int _brightness)
{
    if (CamSensor_id == OV2640_PID)
    {
        ov2640_set_contrast_brightness(cam_sensor, _contrast, _brightness);
    }
    else
    {
        cam_sensor->set_contrast(cam_sensor, _contrast);     // -2 to 2
        cam_sensor->set_brightness(cam_sensor, _brightness); // -2 to 2
    }
}

// - It always zooms to the image center when offsets are zero
// - if imageSize = 0 then the image is not zoomed
// - if imageSize = max value, then the image is fully zoomed in
// - a zoom step is >>> Width + 32 px / Height + 24 px
void CCamera::sanitize_zoom_params(camera_control_config_temp_t *camConfig, int imageSize, int frameSizeX, int frameSizeY, int &imageWidth, int &imageHeight, int &zoomOffsetX, int &zoomOffsetY)
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

esp_err_t CCamera::set_zoom_size(camera_control_config_temp_t *camConfig)
{
    sensor_t *cam_sensor = esp_camera_sensor_get();
    if (cam_sensor != NULL)
    {
        if (camConfig->ImageZoomEnabled)
        {
            int _imageSize_temp = 0;

            int _imageWidth = camConfig->ImageWidth;
            int _imageHeight = camConfig->ImageHeight;

            int _offsetx = camConfig->ImageZoomOffsetX;
            int _offsety = camConfig->ImageZoomOffsetY;

            int frameSizeX = 1600;
            int frameSizeY = 1200;

            switch (CamSensor_id)
            {
            case OV5640_PID:
                frameSizeX = 2592;
                frameSizeY = 1944;
                // max imageSize = ((frameSizeX - camConfig->ImageWidth) / 8 / 4) - 1
                // 59 = ((2560 - 640) / 8 / 4) - 1
                if (camConfig->ImageZoomSize < 59)
                {
                    _imageSize_temp = (59 - camConfig->ImageZoomSize);
                }
                break;

            case OV3660_PID:
                frameSizeX = 2048;
                frameSizeY = 1536;
                // max imageSize = ((frameSizeX - camConfig->ImageWidth) / 8 / 4) -1
                // 43 = ((2048 - 640) / 8 / 4) - 1
                if (camConfig->ImageZoomSize < 43)
                {
                    _imageSize_temp = (43 - camConfig->ImageZoomSize);
                }
                break;

            case OV2640_PID:
                frameSizeX = 1600;
                frameSizeY = 1200;
                // max imageSize = ((frameSizeX - camConfig->ImageWidth) / 8 / 4) -1
                // 29 = ((1600 - 640) / 8 / 4) - 1
                if (camConfig->ImageZoomSize < 29)
                {
                    _imageSize_temp = (29 - camConfig->ImageZoomSize);
                }
                break;

            default:
                // do nothing
                break;
            }

            sanitize_zoom_params(camConfig, _imageSize_temp, frameSizeX, frameSizeY, _imageWidth, _imageHeight, _offsetx, _offsety);
            set_camera_window(camConfig, frameSizeX, frameSizeY, _offsetx, _offsety, _imageWidth, _imageHeight);
        }
        else
        {
            cam_sensor->set_framesize(cam_sensor, camConfig->ImageFrameSize);
        }
    }
    else
    {
        ESP_LOGE(TAG, "set_zoom_size(), Failed to get Cam control structure");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t CCamera::set_quality_zoom_size(camera_control_config_temp_t *camConfig)
{
    sensor_t *cam_sensor = esp_camera_sensor_get();
    if (cam_sensor != NULL)
    {
        // OV2640 has no lower limit on jpeg quality
        if (CamSensor_id == OV5640_PID)
        {
            camConfig->ImageQuality = min(63, max(8, camConfig->ImageQuality));
        }

        set_image_width_height_from_resolution(camConfig);

        cam_sensor->set_quality(cam_sensor, camConfig->ImageQuality);
        set_zoom_size(camConfig);
    }
    else
    {
        ESP_LOGE(TAG, "set_quality_zoom_size(), Failed to get Cam control structure");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t CCamera::set_camera_window(camera_control_config_temp_t *camConfig, int frameSizeX, int frameSizeY, int xOffset, int yOffset, int xTotal, int yTotal)
{
    sensor_t *cam_sensor = esp_camera_sensor_get();
    if (cam_sensor != NULL)
    {
        if (CamSensor_id == OV2640_PID)
        {
            cam_sensor->set_res_raw(cam_sensor, 0, 0, 0, 0, xOffset, yOffset, xTotal, yTotal, camConfig->ImageWidth, camConfig->ImageHeight, false, false);
        }
        else
        {
            // for CAMERA_OV5640 and CAMERA_OV3660
            bool scale = !(camConfig->ImageWidth == xTotal && camConfig->ImageHeight == yTotal);
            bool binning = (xTotal >= (frameSizeX >> 1));

            if (camConfig->ImageVflip == 1)
            {
                cam_sensor->set_res_raw(cam_sensor, xOffset, yOffset, xOffset + xTotal - 1, yOffset + yTotal - 1, 0, 0, frameSizeX, frameSizeY, camConfig->ImageWidth, camConfig->ImageHeight, scale, binning);
            }
            else
            {
                cam_sensor->set_res_raw(cam_sensor, xOffset, yOffset, xOffset + xTotal, yOffset + yTotal, 0, 0, frameSizeX, frameSizeY, camConfig->ImageWidth, camConfig->ImageHeight, scale, binning);
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "set_camera_window(), Failed to get Cam control structure");
        return ESP_FAIL;
    }

    return ESP_OK;
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

// wird von ClassFlowTakeImage::doFlow() verwendet
esp_err_t CCamera::capture_to_basis_image(CImageBasis *tempImageBasis, int flash_duration)
{
    if (tempImageBasis == nullptr || !tempImageBasis->ImageOkay())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: target image is not allocated");
        return ESP_ERR_INVALID_ARG;
    }

    // tempImageBasis->EmptyImage(); // Delete previous stored raw image -> black image

    check_camera_settings_changed();
    set_status_led(true); // Status-LED on
    bool flashUsed = (flash_duration > 0);

    if (flashUsed)
    {
        CaptureToBasisImageLed = true;
        set_flash_light(true); // Flash-LED on
        vTaskDelay(pdMS_TO_TICKS(flash_duration));
    }

    camera_fb_t *warmupFrame = esp_camera_fb_get();
    if (warmupFrame != nullptr)
    {
        esp_camera_fb_return(warmupFrame);
    }

    camera_fb_t *temp_frame_buffer = esp_camera_fb_get();

    if (flashUsed)
    {
        CaptureToBasisImageLed = false;
        if (!CaptureToFileLed && !CaptureToHTTPLed && !CaptureToStreamLed)
        {
            if (flashUsed)
            {
                set_flash_light(false); // Flash-LED off
            }
        }
    }

    if (temp_frame_buffer == nullptr)
    {
        if (!CaptureToFileLed && !CaptureToHTTPLed && !CaptureToStreamLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
        }

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: camera framebuffer unavailable");
        return ESP_FAIL;
    }

    if (temp_frame_buffer->buf == nullptr || temp_frame_buffer->len == 0)
    {
        esp_camera_fb_return(temp_frame_buffer);

        if (!CaptureToFileLed && !CaptureToHTTPLed && !CaptureToStreamLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
        }

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: empty camera framebuffer");
        return ESP_FAIL;
    }

    if (Camera.DemoMode && !load_next_demo_image(temp_frame_buffer))
    {
        esp_camera_fb_return(temp_frame_buffer);

        if (!CaptureToFileLed && !CaptureToHTTPLed && !CaptureToStreamLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
        }

        return ESP_FAIL;
    }

    CImageBasis *tmpImage = new (std::nothrow) CImageBasis("tmpImage");

    if (tmpImage)
    {
        if (temp_frame_buffer->len > static_cast<size_t>(INT_MAX))
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: camera frame is too large to decode");
        }
        else
        {
            tmpImage->LoadFromMemory(temp_frame_buffer->buf, static_cast<int>(temp_frame_buffer->len));
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: Can't allocate tmpImage");
    }

    esp_camera_fb_return(temp_frame_buffer);

    if (!CaptureToFileLed && !CaptureToHTTPLed && !CaptureToStreamLed)
    {
        set_status_led(false); // Status-LED off
        set_camera_deep_sleep(true);
    }

    if (tmpImage == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    if (!tmpImage->ImageOkay())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: STBI could not decode the camera frame");
        delete tmpImage;

        return ESP_FAIL;
    }

    int channels = 3;
    int width = CCstatus.ImageWidth;
    int height = CCstatus.ImageHeight;

    // Both images must be packed RGB888 with exactly the configured dimensions.
    // Never copy using camera settings alone: the decoded frame may differ.
    if (tempImageBasis->rgb_image == nullptr || tmpImage->width != width || tmpImage->height != height ||
        tempImageBasis->width != width || tempImageBasis->height != height || tmpImage->channels != channels || tempImageBasis->channels != channels)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: decoded frame and target image dimensions do not match");
        delete tmpImage;

        return ESP_FAIL;
    }

    if (width <= 0 || height <= 0 || static_cast<size_t>(width) > SIZE_MAX / static_cast<size_t>(height) ||
        static_cast<size_t>(width) * static_cast<size_t>(height) > SIZE_MAX / static_cast<size_t>(channels))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToBasisImage: invalid frame size");
        delete tmpImage;

        return ESP_ERR_INVALID_SIZE;
    }

    const size_t imageBytes = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
    uint8_t *targetData = tempImageBasis->RGBImageLock();

    if (targetData == nullptr)
    {
        tempImageBasis->RGBImageRelease();

        delete tmpImage;

        return ESP_ERR_INVALID_STATE;
    }

    uint8_t *sourceData = tmpImage->RGBImageLock();

    if (sourceData == nullptr)
    {
        tmpImage->RGBImageRelease();
        tempImageBasis->RGBImageRelease();

        delete tmpImage;

        return ESP_ERR_INVALID_STATE;
    }

    // Both images are packed RGB888: copy the whole frame at once.
    memcpy(targetData, sourceData, imageBytes);

    tmpImage->RGBImageRelease();
    tempImageBasis->RGBImageRelease();

    delete tmpImage;

    return ESP_OK;
}

// wird von ClassFlowTakeImage::camera_capture() und server_camera.cpp->handler_capture_save_to_file() verwendet
esp_err_t CCamera::capture_to_file(std::string filename, int flash_duration)
{
    if (filename.empty())
    {
        return ESP_ERR_INVALID_ARG;
    }

    check_camera_settings_changed();
    set_status_led(true); // Status-LED on
    bool flashUsed = (flash_duration > 0);

    if (flashUsed)
    {
        CaptureToFileLed = true;
        set_flash_light(true); // Flash-LED on
        vTaskDelay(pdMS_TO_TICKS(flash_duration));
    }

    camera_fb_t *warmupFrame = esp_camera_fb_get();
    if (warmupFrame != nullptr)
    {
        esp_camera_fb_return(warmupFrame);
    }

    camera_fb_t *temp_frame_buffer = esp_camera_fb_get();

    if (flashUsed)
    {
        CaptureToFileLed = false;
        if (!CaptureToBasisImageLed && !CaptureToHTTPLed && !CaptureToStreamLed)
        {
            if (flashUsed)
            {
                set_flash_light(false); // Flash-LED off
            }
        }
    }

    if (temp_frame_buffer == nullptr)
    {
        if (!CaptureToBasisImageLed && !CaptureToHTTPLed && !CaptureToStreamLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
        }

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: camera framebuffer unavailable");
        return ESP_FAIL;
    }

    if (temp_frame_buffer->buf == nullptr || temp_frame_buffer->len == 0)
    {
        esp_camera_fb_return(temp_frame_buffer);

        if (!CaptureToBasisImageLed && !CaptureToHTTPLed && !CaptureToStreamLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
        }

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: empty camera framebuffer");
        return ESP_FAIL;
    }

    uint8_t *buf = nullptr;
    size_t buf_len = 0;

    bool converted = false;

    filename = FormatFileName(filename);
    const string ftype = toUpper(getFileType(filename));

    if (ftype == "BMP")
    {
        converted = true;

        if (!frame2bmp(temp_frame_buffer, &buf, &buf_len))
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: BMP conversion failed");
        }
    }
    else if (ftype == "JPG")
    {
        if (temp_frame_buffer->format != PIXFORMAT_JPEG)
        {
            converted = true;

            if (!frame2jpg(temp_frame_buffer, CCstatus.ImageQuality, &buf, &buf_len))
            {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: JPEG conversion failed");
            }
        }
        else
        {
            buf = temp_frame_buffer->buf;
            buf_len = temp_frame_buffer->len;
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: unsupported output format " + ftype);
    }

    esp_camera_fb_return(temp_frame_buffer);

    if (!CaptureToBasisImageLed && !CaptureToHTTPLed && !CaptureToStreamLed)
    {
        set_status_led(false); // Status-LED off
        set_camera_deep_sleep(true);
    }

    esp_err_t result = ESP_FAIL;

    if (buf != nullptr && buf_len > 0)
    {
        FILE *pFile = fopen(filename.c_str(), "wb");
        if (pFile == nullptr)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: failed to open file " + filename);
        }
        else
        {
            const size_t written = fwrite(buf, 1, buf_len, pFile);
            const int closeResult = fclose(pFile);

            if (written == buf_len && closeResult == 0)
            {
                result = ESP_OK;
            }
            else
            {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: incomplete write to " + filename);
            }
        }
    }

    if (converted && buf != nullptr)
    {
        free(buf);
    }

    return result;
}

// wird von ClassFlowTakeImage::SendRawJPG() und server_camera.cpp->handler_capture() verwendet
esp_err_t CCamera::capture_to_http(httpd_req_t *req, int flash_duration)
{
    if (req == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }

    const int64_t fr_start = esp_timer_get_time();

    check_camera_settings_changed();
    set_status_led(true); // Status-LED on
    bool flashUsed = (flash_duration > 0);

    if (flashUsed)
    {
        CaptureToHTTPLed = true;
        set_flash_light(true); // Flash-LED on
        vTaskDelay(pdMS_TO_TICKS(flash_duration));
    }

    camera_fb_t *warmupFrame = esp_camera_fb_get();
    if (warmupFrame != nullptr)
    {
        esp_camera_fb_return(warmupFrame);
    }

    camera_fb_t *temp_frame_buffer = esp_camera_fb_get();

    if (flashUsed)
    {
        CaptureToHTTPLed = false;
        if (!CaptureToBasisImageLed && !CaptureToFileLed && !CaptureToStreamLed)
        {
            if (flashUsed)
            {
                set_flash_light(false); // Flash-LED off
            }
        }
    }

    if (temp_frame_buffer == nullptr)
    {
        if (!CaptureToBasisImageLed && !CaptureToFileLed && !CaptureToStreamLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
        }

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToHTTP: camera framebuffer unavailable");
        httpd_resp_send_500(req);

        return ESP_FAIL;
    }

    if (temp_frame_buffer->buf == nullptr || temp_frame_buffer->len == 0)
    {
        esp_camera_fb_return(temp_frame_buffer);

        if (!CaptureToBasisImageLed && !CaptureToFileLed && !CaptureToStreamLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
        }

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToHTTP: empty camera framebuffer");
        httpd_resp_send_500(req);

        return ESP_FAIL;
    }

    esp_err_t res = httpd_resp_set_type(req, "image/jpeg");
    if (res == ESP_OK)
    {
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=raw.jpg");
    }

    size_t temp_frame_buffer_len = 0;

    if (res == ESP_OK)
    {
        if (Camera.DemoMode)
        {
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Using Demo image!");

            if (!load_next_demo_image(temp_frame_buffer))
            {
                res = ESP_FAIL;
            }

            if (res != ESP_OK || temp_frame_buffer->buf == nullptr || temp_frame_buffer->len == 0)
            {
                res = ESP_FAIL;
            }
            else
            {
                temp_frame_buffer_len = temp_frame_buffer->len;
                res = httpd_resp_send(req, (const char *)temp_frame_buffer->buf, temp_frame_buffer_len);
            }
        }
        else if (temp_frame_buffer->format == PIXFORMAT_JPEG)
        {
            temp_frame_buffer_len = temp_frame_buffer->len;
            res = httpd_resp_send(req, (const char *)temp_frame_buffer->buf, temp_frame_buffer->len);
        }
        else
        {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(temp_frame_buffer, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
            temp_frame_buffer_len = jchunk.len;

            if (res == ESP_OK)
            {
                res = httpd_resp_send_chunk(req, NULL, 0);
            }
        }
    }

    esp_camera_fb_return(temp_frame_buffer);

    if (!CaptureToBasisImageLed && !CaptureToFileLed && !CaptureToStreamLed)
    {
        set_status_led(false); // Status-LED off
        set_camera_deep_sleep(true);
    }

    const int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %dKB %dms", (int)(temp_frame_buffer_len / 1024), (int)((fr_end - fr_start) / 1000));

    return res;
}

// wird von MainFlowControl.cpp->handler_stream() verwendet
esp_err_t CCamera::capture_to_stream(httpd_req_t *req, bool FlashlightOn)
{
    if (req == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }

    check_camera_settings_changed();
    if (FlashlightOn)
    {
        set_status_led(true);  // Status-LED on
        set_flash_light(true); // Flash-LED on
    }

    camera_fb_t *warmupFrame = esp_camera_fb_get();
    if (warmupFrame != nullptr)
    {
        esp_camera_fb_return(warmupFrame);
    }

    camera_fb_t *temp_frame_buffer = esp_camera_fb_get();
    if (temp_frame_buffer == NULL)
    {
        CaptureToStreamLed = false;
        if (!CaptureToBasisImageLed && !CaptureToFileLed && !CaptureToHTTPLed)
        {
            set_status_led(false); // Status-LED off
            set_camera_deep_sleep(true);
            if (FlashlightOn)
            {
                set_flash_light(false); // Flash-LED off
            }
        }

        ESP_LOGE(TAG, "capture_to_stream(): Capture Failed. Check camera module and/or proper electrical connection");
        httpd_resp_send_500(req);

        esp_camera_fb_return(temp_frame_buffer);

        return ESP_FAIL;
    }

    esp_camera_fb_return(temp_frame_buffer);

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Live stream started");

    esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res == ESP_OK)
    {
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }

    size_t temp_frame_buffer_len = 0;
    int64_t fr_start;
    char part_buf[64];

    while (res == ESP_OK)
    {
        fr_start = esp_timer_get_time();

        temp_frame_buffer = esp_camera_fb_get();

        if (!temp_frame_buffer)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToStream: Camera framebuffer not available");
            res = ESP_FAIL;
            break;
        }

        if (temp_frame_buffer->buf == nullptr || temp_frame_buffer->len == 0)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToStream: empty camera framebuffer");
            esp_camera_fb_return(temp_frame_buffer);
            res = ESP_FAIL;
            break;
        }

        temp_frame_buffer_len = temp_frame_buffer->len;

        if (res == ESP_OK)
        {
            const int headerLength = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, temp_frame_buffer_len);

            if (headerLength < 0 || static_cast<size_t>(headerLength) >= sizeof(part_buf))
            {
                res = ESP_FAIL;
            }
            else
            {
                res = httpd_resp_send_chunk(req, part_buf, static_cast<size_t>(headerLength));
            }
        }

        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)temp_frame_buffer->buf, temp_frame_buffer_len);
        }

        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }

        esp_camera_fb_return(temp_frame_buffer);

        int64_t fr_end = esp_timer_get_time();

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

    esp_camera_fb_return(temp_frame_buffer);

    CaptureToStreamLed = false;
    if (!CaptureToBasisImageLed && !CaptureToFileLed && !CaptureToHTTPLed)
    {
        set_status_led(false); // Status-LED off
        set_camera_deep_sleep(true);
        if (FlashlightOn)
        {
            set_flash_light(false); // Flash-LED off
        }
    }

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Live stream stopped");

    return res;
}

void CCamera::set_flash_light(bool status)
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
            ESP_LOGD(TAG, "Internal Flash-LED turn on with PWM %d", Camera.LedIntensity);
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, Camera.LedIntensity));
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

void CCamera::set_status_led(bool status)
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

void CCamera::set_image_width_height_from_resolution(camera_control_config_temp_t *camConfig)
{
    if (camConfig->ImageFrameSize == FRAMESIZE_QVGA)
    {
        camConfig->ImageWidth = 320;
        camConfig->ImageHeight = 240;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_VGA)
    {
        camConfig->ImageWidth = 640;
        camConfig->ImageHeight = 480;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_SVGA)
    {
        camConfig->ImageWidth = 800;
        camConfig->ImageHeight = 600;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_XGA)
    {
        camConfig->ImageWidth = 1024;
        camConfig->ImageHeight = 768;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_HD)
    {
        camConfig->ImageWidth = 1280;
        camConfig->ImageHeight = 720;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_SXGA)
    {
        camConfig->ImageWidth = 1280;
        camConfig->ImageHeight = 1024;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_UXGA)
    {
        camConfig->ImageWidth = 1600;
        camConfig->ImageHeight = 1200;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_QXGA)
    {
        camConfig->ImageWidth = 2048;
        camConfig->ImageHeight = 1536;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_WQXGA)
    {
        camConfig->ImageWidth = 2560;
        camConfig->ImageHeight = 1600;
    }
    else if (camConfig->ImageFrameSize == FRAMESIZE_QSXGA)
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

void CCamera::use_demo_mode(void)
{
    FILE *pFile = fopen("/sdcard/demo/files.txt", "r");

    if (pFile == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can not start Demo mode, the folder '/sdcard/demo/' does not contain the needed files!");
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "See Details on https://jomjol.github.io/AI-on-the-edge-device-docs/Demo-Mode!");
        return;
    }

    if (demoImage == nullptr)
    {
        demoImage = static_cast<uint8_t *>(malloc(DEMO_IMAGE_SIZE));
    }

    if (demoImage == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Unable to acquire required memory for demo image!");
        fclose(pFile);
        return;
    }

    demoFiles.clear();

    char line[50];

    while (fgets(line, sizeof(line), pFile) != nullptr)
    {
        // Remove either LF or CRLF without underflow on empty lines.
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] != '\0')
        {
            demoFiles.emplace_back(line);
        }
    }
    fclose(pFile);

    if (demoFiles.empty())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Demo image list is empty");
        return;
    }

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Using Demo mode (" + std::to_string(demoFiles.size()) + " files) instead of real camera image!");
    for (const auto &file : demoFiles)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, file);
    }

    Camera.DemoMode = true;
}

bool CCamera::load_next_demo_image(camera_fb_t *fb)
{
    if (fb == nullptr || demoImage == nullptr || demoFiles.empty())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Demo mode is not initialized");
        return false;
    }

    const size_t fileIndex = static_cast<size_t>(getCountFlowRounds()) % demoFiles.size();
    const std::string filename = "/sdcard/demo/" + demoFiles[fileIndex];
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Using " + filename + " as demo image");

    FILE *pFile = fopen(filename.c_str(), "rb");
    if (pFile == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to read file: " + filename + "!");
        return false;
    }

    const long fileSize = get_file_size(filename);
    if (fileSize <= 0 || static_cast<unsigned long>(fileSize) > DEMO_IMAGE_SIZE)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Demo image has invalid size: " + std::to_string(fileSize));
        fclose(pFile);
        return false;
    }

    const size_t bytesRead = fread(demoImage, 1, static_cast<size_t>(fileSize), pFile);
    const bool readOk = bytesRead == static_cast<size_t>(fileSize) && ferror(pFile) == 0;

    fclose(pFile);

    if (!readOk)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Short read while loading demo image: " + filename);
        return false;
    }

    fb->buf = demoImage;
    fb->len = bytesRead;

    return true;
}

long CCamera::get_file_size(std::string filename)
{
    struct stat stat_buf;
    long rc = stat(filename.c_str(), &stat_buf);

    return rc == 0 ? stat_buf.st_size : -1;
}
