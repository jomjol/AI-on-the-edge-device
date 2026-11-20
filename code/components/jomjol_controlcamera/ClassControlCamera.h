#pragma once

#ifndef CLASSCONTROLCAMERA_H
#define CLASSCONTROLCAMERA_H

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_camera.h"
#include <esp_http_server.h>
#include "CImageBasis.h"
#include "defines.h"

typedef struct
{
    int CamXclkFreqMhz = 16; // 1 to 20

    framesize_t ImageFrameSize = FRAMESIZE_VGA;      // 0 - 10
    gainceiling_t ImageGainceiling = GAINCEILING_4X; // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)

    bool ImageScale = false;         // Scale enable (0 = disable , 1 = enable)
    bool ImageBinning = false;       // Binning enable (0 = disable , 1 = enable)
    int ImageQuality = 12;           // 0 to 63
    int ImageBrightness = 0;         // -2 to 2
    int ImageContrast = 2;           // -2 to 2
    int ImageSaturation = 0;         // -2 to 2
    int ImageSharpness = 0;          // -2 to 2
    bool ImageAutoSharpness = false; // false - true
    int ImageDenoise = 0;            // The OV2640 does not support it, OV3660 and OV5640 (0 to 8)
    int ImageSpecialEffect = 0;      // 0 to 6
    int ImageWbMode = 0;             // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    int ImageAwb = 1;                // white balance enable (0 = disable , 1 = enable)
    int ImageAwbGain = 1;            // Auto White Balance enable (0 = disable , 1 = enable)
    int ImageAec = 1;                // auto exposure off (0 = disable , 1 = enable)
    int ImageAec2 = 1;               // automatic exposure sensor  (0 = disable , 1 = enable)
    int ImageAeLevel = 0;            // auto exposure levels (-2 to 2)
    int ImageAecValue = 600;         // set exposure manually  (0 to 1200)
    int ImageAgc = 1;                // auto gain off (0 = disable , 1 = enable)
    int ImageAgcGain = 8;            // set gain manually (0 to 30)
    int ImageBpc = 1;                // black pixel correction (0 = disable , 1 = enable)
    int ImageWpc = 1;                // white pixel correction (0 = disable , 1 = enable)
    int ImageRawGma = 1;             // raw gma (0 = disable , 1 = enable)
    int ImageLenc = 1;               // lens correction (0 = disable , 1 = enable)
    int ImageHmirror = 0;            // flip horizontally (0 = disable , 1 = enable)
    int ImageVflip = 0;              // invert image (0 = disable , 1 = enable)
    int ImageDcw = 1;                // downsize enable (0 = disable , 1 = enable)
    int ImageColorbar = 0;           // Colorbar enable (0 = disable , 1 = enable)

    int ImageLedIntensity = 25;

    bool ImageZoomEnabled = false;
    int ImageZoomOffsetX = 0;
    int ImageZoomOffsetY = 0;
    int ImageZoomSize = 0;

    int ImageWidth = 640;
    int ImageHeight = 480;

    int WaitBeforePicture = 5;
} camera_control_config_temp_t;

extern camera_control_config_temp_t CCstatus;
extern camera_control_config_temp_t CFstatus;

class CCamera
{
protected:
    void ledc_init(void);
    int set_led_intensity(int _intrel);

    esp_err_t check_camera_settings_changed(void);

    int set_camera_gainceiling(sensor_t *cam_sensor, gainceiling_t gainceilingLevel);
    void set_camera_sharpness(sensor_t *cam_sensor, bool autoSharpnessEnabled, int sharpnessLevel);
    void set_camera_special_effect(sensor_t *cam_sensor, int specialEffect);
    void set_camera_contrast_brightness(sensor_t *cam_sensor, int _contrast, int _brightness);

    void sanitize_zoom_params(camera_control_config_temp_t *camConfig, int imageSize, int frameSizeX, int frameSizeY, int &imageWidth, int &imageHeight, int &zoomOffsetX, int &zoomOffsetY);
    esp_err_t set_zoom_size(camera_control_config_temp_t *camConfig);

    esp_err_t set_camera_window(camera_control_config_temp_t *camConfig, int frameSizeX, int frameSizeY, int xOffset, int yOffset, int xTotal, int yTotal);

    void set_image_width_height_from_resolution(camera_control_config_temp_t *camConfig);

    bool load_next_demo_image(camera_fb_t *fb);
    long get_file_size(std::string filename);

public:
    uint16_t CamSensor_id = OV2640_PID;

    bool CameraInitSuccessful = false;
    bool changedCameraSettings = false;
    bool CamDeepSleepEnable = false;
    bool CamTempImage = false;

    int LedIntensity = 512;

    bool CaptureToBasisImageLed = false;
    bool CaptureToFileLed = false;
    bool CaptureToHTTPLed = false;
    bool CaptureToStreamLed = false;

    bool DemoMode = false;
    bool SaveAllFiles = false;

    CCamera(void);

    esp_err_t init_camera(void);
    bool test_camera(void);

    void set_flash_light(bool status);
    void set_status_led(bool status);

    void power_reset_camera(void);

    bool get_camera_init_successful(void);

    esp_err_t set_sensor_control_config(camera_control_config_temp_t *camConfig);
    esp_err_t get_sensor_control_config(camera_control_config_temp_t *camConfig);

    esp_err_t set_camera_config_from_to(camera_control_config_temp_t *camConfigFrom, camera_control_config_temp_t *camConfigTo);

    esp_err_t set_camera_deep_sleep(bool status);

    esp_err_t set_quality_zoom_size(camera_control_config_temp_t *camConfig);

    esp_err_t capture_to_basis_image(CImageBasis *_Image, int delay = 0);

    esp_err_t capture_to_file(std::string file_name, int delay = 0);
    esp_err_t capture_to_http(httpd_req_t *req, int delay = 0);
    esp_err_t capture_to_stream(httpd_req_t *req, bool FlashlightOn);

    void use_demo_mode(void);
};

extern CCamera Camera;

#endif
