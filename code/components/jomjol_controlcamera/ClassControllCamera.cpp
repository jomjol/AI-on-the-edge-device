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
    .jpeg_quality = 6,                 // 0-63 lower number means higher quality
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

    CCstatus.ImageQuality = camera_config.jpeg_quality;
    CCstatus.ImageFrameSize = camera_config.frame_size;

    // initialize the camera
    esp_camera_deinit(); // De-init in case it was already initialized
    esp_err_t err = esp_camera_init(&camera_config);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    CCstatus.CameraInitSuccessful = true;

    // Get a reference to the sensor
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        // Dump camera module, warn for unsupported modules.
        switch (s->id.PID)
        {
        case OV2640_PID:
            ESP_LOGI(TAG, "OV2640 camera module detected");
            break;
        case OV3660_PID:
            ESP_LOGI(TAG, "OV3660 camera module detected");
            break;
        default:
            ESP_LOGE(TAG, "Camera module is unknown and not properly supported!");
            CCstatus.CameraInitSuccessful = false;
        }
    }

    if (CCstatus.CameraInitSuccessful)
    {
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

void CCamera::SetLEDIntensity(float _intrel)
{
    _intrel = min(_intrel, (float)100);
    _intrel = max(_intrel, (float)0);
    _intrel = _intrel / 100;
    CCstatus.ImageLedIntensity = (int)(_intrel * 8191);
    ESP_LOGD(TAG, "Set led_intensity to %d of 8191", CCstatus.ImageLedIntensity);
}

bool CCamera::getCameraInitSuccessful(void)
{
    return CCstatus.CameraInitSuccessful;
}

esp_err_t CCamera::setSensorDatenFromCCstatus(void)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        s->set_framesize(s, CCstatus.ImageFrameSize);
        s->set_gainceiling(s, CCstatus.ImageGainceiling); // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)

        s->set_quality(s, CCstatus.ImageQuality); // 0 - 63

        s->set_brightness(s, CCstatus.ImageBrightness); // -2 to 2
        s->set_contrast(s, CCstatus.ImageContrast);     // -2 to 2
        s->set_saturation(s, CCstatus.ImageSaturation); // -2 to 2
        // s->set_sharpness(s, CCstatus.ImageSharpness);   // auto-sharpness is not officially supported, default to 0
        SetCamSharpness(CCstatus.ImageAutoSharpness, CCstatus.ImageSharpness);

        s->set_exposure_ctrl(s, CCstatus.ImageAec);  // 0 = disable , 1 = enable
        s->set_ae_level(s, CCstatus.ImageAeLevel);   // -2 to 2
        s->set_aec_value(s, CCstatus.ImageAecValue); // 0 to 1200

        s->set_aec2(s, CCstatus.ImageAec2); // 0 = disable , 1 = enable

        s->set_gain_ctrl(s, CCstatus.ImageAgc);    // 0 = disable , 1 = enable
        s->set_agc_gain(s, CCstatus.ImageAgcGain); // 0 to 30

        s->set_bpc(s, CCstatus.ImageBpc); // 0 = disable , 1 = enable
        s->set_wpc(s, CCstatus.ImageWpc); // 0 = disable , 1 = enable

        s->set_raw_gma(s, CCstatus.ImageRawGma); // 0 = disable , 1 = enable
        s->set_lenc(s, CCstatus.ImageLenc);      // 0 = disable , 1 = enable

        s->set_hmirror(s, CCstatus.ImageHmirror); // 0 = disable , 1 = enable
        s->set_vflip(s, CCstatus.ImageVflip);     // 0 = disable , 1 = enable

        s->set_dcw(s, CCstatus.ImageDcw); // 0 = disable , 1 = enable

        s->set_wb_mode(s, CCstatus.ImageWbMode);   // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        s->set_awb_gain(s, CCstatus.ImageAwbGain); // 0 = disable , 1 = enable
        s->set_whitebal(s, CCstatus.ImageAwb);     // 0 = disable , 1 = enable

        // special_effect muß als Letztes gesetzt werden, sonst geht es nicht
        s->set_special_effect(s, CCstatus.ImageSpecialEffect); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)

        TickType_t xDelay2 = 1000 / portTICK_PERIOD_MS;
        vTaskDelay(xDelay2);

        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

esp_err_t CCamera::getSensorDatenToCCstatus(void)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        CCstatus.ImageFrameSize = (framesize_t)s->status.framesize;
        CCstatus.ImageGainceiling = (gainceiling_t)s->status.gainceiling;

        CCstatus.ImageQuality = s->status.quality;
        CCstatus.ImageBrightness = s->status.brightness;
        CCstatus.ImageContrast = s->status.contrast;
        CCstatus.ImageSaturation = s->status.saturation;
        // CCstatus.ImageSharpness = s->status.sharpness; // gibt -1 zurück, da es nicht unterstützt wird
        CCstatus.ImageWbMode = s->status.wb_mode;
        CCstatus.ImageAwb = s->status.awb;
        CCstatus.ImageAwbGain = s->status.awb_gain;
        CCstatus.ImageAec = s->status.aec;
        CCstatus.ImageAec2 = s->status.aec2;
        CCstatus.ImageAeLevel = s->status.ae_level;
        CCstatus.ImageAecValue = s->status.aec_value;
        CCstatus.ImageAgc = s->status.agc;
        CCstatus.ImageAgcGain = s->status.agc_gain;
        CCstatus.ImageBpc = s->status.bpc;
        CCstatus.ImageWpc = s->status.wpc;
        CCstatus.ImageRawGma = s->status.raw_gma;
        CCstatus.ImageLenc = s->status.lenc;
        CCstatus.ImageSpecialEffect = s->status.special_effect;
        CCstatus.ImageHmirror = s->status.hmirror;
        CCstatus.ImageVflip = s->status.vflip;
        CCstatus.ImageDcw = s->status.dcw;

        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

void CCamera::SetZoomSize(bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        if (zoomEnabled)
        {
            // ov2640_sensor_mode_t _mode = OV2640_MODE_UXGA; // 1600x1200
            // ov2640_sensor_mode_t _mode = OV2640_MODE_SVGA; // 800x600
            // ov2640_sensor_mode_t _mode = OV2640_MODE_CIF;  // 400x296
            int _mode = 0;

            int _offsetx = zoomOffsetX;
            int _offsety = zoomOffsetY;
            int _imageSize_temp = 0;
            int _maxX = 0;
            int _maxY = 0;

            if (imageSize < 29)
            {
                _imageSize_temp = (29 - imageSize);
            }

            // This works only if the aspect ratio of 4:3 is preserved in the window size.
            // use values divisible by 8 without remainder
            int _imageWidth = CCstatus.ImageWidth + (_imageSize_temp * 4 * 8);
            int _imageHeight = CCstatus.ImageHeight + (_imageSize_temp * 3 * 8);

            _maxX = 1600 - _imageWidth;
            _maxY = 1200 - _imageHeight;

            if ((abs(_offsetx) * 2) > _maxX)
            {
                if (_offsetx > 0)
                {
                    _offsetx = _maxX;
                }
                else
                {
                    _offsetx = 0;
                }
            }
            else
            {
                if (_offsetx > 0)
                {
                    // wenn der Wert von _offsetx nicht durch 8 teilbar ist,
                    // werden die Farben sehr oft vertauscht(insbesondere Rot mit Blau)
                    _offsetx = ((_maxX / 2) + _offsetx);
                }
                else
                {
                    // wenn der Wert von _offsetx nicht durch 8 teilbar ist,
                    // werden die Farben sehr oft vertauscht(insbesondere Rot mit Blau)
                    _offsetx = ((_maxX / 2) + _offsetx);
                }
            }

            if ((abs(_offsety) * 2) > _maxY)
            {
                if (_offsety > 0)
                {
                    _offsety = _maxY;
                }
                else
                {
                    _offsety = 0;
                }
            }
            else
            {
                if (_offsety > 0)
                {
                    // wenn der Wert von _offsety nicht durch 8 teilbar ist,
                    // werden die Farben sehr oft vertauscht(insbesondere Rot mit Blau)
                    _offsety = ((_maxY / 2) + _offsety);
                }
                else
                {
                    // wenn der Wert von _offsety nicht durch 8 teilbar ist,
                    // werden die Farben sehr oft vertauscht(insbesondere Rot mit Blau)
                    _offsety = ((_maxY / 2) + _offsety);
                }
            }

            // _mode sets the sensor resolution (3 options available),
            // _offsetx and _offsety set the start of the ROI,
            // _imageWidth and _imageHeight set the size of the ROI,
            // CCstatus.ImageWidth and CCstatus.ImageHeight set the output window size.
            SetCamWindow(s, _mode, _offsetx, _offsety, _imageWidth, _imageHeight, CCstatus.ImageWidth, CCstatus.ImageHeight);
        }
        else
        {
            s->set_framesize(s, CCstatus.ImageFrameSize);
        }
    }
}

void CCamera::SetQualityZoomSize(int qual, framesize_t resol, bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize)
{
    qual = min(63, max(8, qual)); // Limit quality from 8..63 (values lower than 8 tent to be unstable)

    SetImageWidthHeightFromResolution(resol);

    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        s->set_quality(s, qual);
        SetZoomSize(zoomEnabled, zoomOffsetX, zoomOffsetY, imageSize);
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SetQualityZoomSize, Failed to get Cam control structure");
    }
}

void CCamera::SetCamSharpness(bool _autoSharpnessEnabled, int _sharpnessLevel)
{
    _sharpnessLevel = min(2, max(-2, _sharpnessLevel));

    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        // post processing
        if (_autoSharpnessEnabled)
        {
            s->set_sharpness(s, 0);
            ov2640_enable_auto_sharpness(s);
        }
        else
        {
            ov2640_set_sharpness(s, _sharpnessLevel);
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SetCamSharpness, Failed to get Cam control structure");
    }
}

/*
 * resolution = 0 \\ OV2640_MODE_UXGA -> 1600 x 1200
 * resolution = 1 \\ OV2640_MODE_SVGA -> 800  x 600
 * resolution = 2 \\ OV2640_MODE_CIF  -> 400  x 296
 * resolution = 3 \\ OV2640_MODE_MAX
 */
void CCamera::SetCamWindow(sensor_t *s, int resolution, int xOffset, int yOffset, int xTotal, int yTotal, int xOutput, int yOutput)
{
    // - (xOffset,yOffset) is the origin of the window in pixels and (xLength,yLength) is the size of the window in pixels.
    // - (xOffset,yOffset) ist der Ursprung des Fensters in Pixel und (xLength,yLength) ist die Größe des Fensters in Pixel.

    // - Be aware that changing the resolution will effectively overwrite these settings.
    // - Beachten Sie, dass eine Änderung der Auflösung diese Einstellungen effektiv überschreibt.

    // - This works only if the aspect ratio of 4:3 is preserved in the window size.
    // - Dies funktioniert nur, wenn das Seitenverhältnis von 4:3 in der Fenstergröße beibehalten wird.

    // - total_x and total_y defines the size on the sensor
    // - total_x und total_y definieren die Größe des Sensors

    // - width and height defines the resulting image(may be smaller than the size on the sensor)
    // - width und height definieren das resultierende Bild (kann kleiner sein als die Größe des Sensor)

    // - keep the aspect total_x : total_y == width : height
    // - Behalten Sie den Aspekt total_x : total_y == width : height bei

    // - use values divisible by 8 without remainder
    // - Verwenden Sie Werte, die ohne Rest durch 8 teilbar sind

    // - start with total_x = width and total_y = height, reduce both values by eg.32 pixels
    // - Beginnen Sie mit total_x = width und total_y = height und reduzieren Sie beide Werte um z.B.32 Pixel

    // - next try moving with offset_x or offset_y by 8 pixels
    // - Versuchen Sie als Nächstes, mit offset_x oder offset_y um 8 Pixel zu verschieben

    // set_res_raw(sensor_t *sensor, int startX, int startY, int endX, int endY, int offsetX, int offsetY, int totalX, int totalY, int outputX, int outputY, bool scale, bool binning)
    // set_window(sensor, (ov2640_sensor_mode_t)startX, offsetX, offsetY, totalX, totalY, outputX, outputY);
    // set_window(sensor, mode, offset_x, offset_y, max_x, max_y, w, h);

    int unused = 0;
    s->set_res_raw(s, resolution, unused, unused, unused, xOffset, yOffset, xTotal, yTotal, xOutput, yOutput, unused, unused);
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
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CaptureToBasisImage - After LightOn");
#endif

    camera_fb_t *fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = esp_camera_fb_get();

    if (!fb)
    {
        LEDOnOff(false);   // Status-LED off
        LightOnOff(false); // Flash-LED off

        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "is not working anymore (CaptureToBasisImage) - most probably caused "
                                                "by a hardware problem (instablility, ...). System will reboot.");
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
    int width = CCstatus.ImageWidth;
    int height = CCstatus.ImageHeight;

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
    string ftype;

    LEDOnOff(true); // Status-LED on

    if (delay > 0)
    {
        LightOnOff(true); // Flash-LED on
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

    camera_fb_t *fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = esp_camera_fb_get();

    if (!fb)
    {
        LEDOnOff(false);   // Status-LED off
        LightOnOff(false); // Flash-LED off
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: Capture Failed. "
                                                "Check camera module and/or proper electrical connection");
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

    ftype = toUpper(getFileType(nm));

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
            bool jpeg_converted = frame2jpg(fb, CCstatus.ImageQuality, &buf, &buf_len);
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
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

    camera_fb_t *fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = esp_camera_fb_get();

    if (!fb)
    {
        LEDOnOff(false);   // Status-LED off
        LightOnOff(false); // Flash-LED off
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CaptureToFile: Capture Failed. "
                                                "Check camera module and/or proper electrical connection");
        httpd_resp_send_500(req);
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

    // wenn die Kameraeinstellungen durch Erstellen eines neuen Referenzbildes verändert wurden, müssen sie neu gesetzt werden
    if (CFstatus.changedCameraSettings)
    {
        Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
        Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize);
        CFstatus.changedCameraSettings = false;
    }

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Live stream started");

    if (FlashlightOn)
    {
        LEDOnOff(true);   // Status-LED on
        LightOnOff(true); // Flash-LED on
    }

    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");  //stream is blocking web interface, only serving to local

    httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));

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
            ESP_LOGD(TAG, "Internal Flash-LED turn on with PWM %d", CCstatus.ImageLedIntensity);
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, CCstatus.ImageLedIntensity));
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

void CCamera::SetImageWidthHeightFromResolution(framesize_t resol)
{
    if (resol == FRAMESIZE_QVGA)
    {
        CCstatus.ImageHeight = 240;
        CCstatus.ImageWidth = 320;
    }
    else if (resol == FRAMESIZE_VGA)
    {
        CCstatus.ImageHeight = 480;
        CCstatus.ImageWidth = 640;
    }
    else if (resol == FRAMESIZE_SVGA)
    {
        CCstatus.ImageHeight = 600;
        CCstatus.ImageWidth = 800;
    }
    else if (resol == FRAMESIZE_XGA)
    {
        CCstatus.ImageHeight = 768;
        CCstatus.ImageWidth = 1024;
    }
    else if (resol == FRAMESIZE_HD)
    {
        CCstatus.ImageHeight = 720;
        CCstatus.ImageWidth = 1280;
    }
    else if (resol == FRAMESIZE_SXGA)
    {
        CCstatus.ImageHeight = 1024;
        CCstatus.ImageWidth = 1280;
    }
    else if (resol == FRAMESIZE_UXGA)
    {
        CCstatus.ImageHeight = 1200;
        CCstatus.ImageWidth = 1600;
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

    return CCstatus.ImageFrameSize;
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
