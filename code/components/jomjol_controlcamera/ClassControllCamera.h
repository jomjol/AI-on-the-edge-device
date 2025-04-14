#pragma once

#ifndef CLASSCONTROLLCAMERA_H
#define CLASSCONTROLLCAMERA_H

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_camera.h"
#include <string>
#include <esp_http_server.h>
#include "CImageBasis.h"
#include "../../include/defines.h"

typedef struct
{
    framesize_t ImageFrameSize = FRAMESIZE_VGA;       // 0 - 10
    int ImageGainceiling = 1;         // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)

    int ImageQuality = 12;            // 0 - 63
    int ImageBrightness = 0;          // (-2 to 2) - set brightness
    int ImageContrast = 0;            //-2 - 2
    int ImageSaturation = 0;          //-2 - 2
    int ImageSharpness = 0;           //-2 - 2
    bool ImageAutoSharpness = false;
    int ImageSpecialEffect = 0;       // 0 - 6
    int ImageWbMode = 0;              // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    int ImageAwb = 1;                 // white balance enable (0 or 1)
    int ImageAwbGain = 1;             // Auto White Balance enable (0 or 1)
    int ImageAec = 1;                 // auto exposure off (1 or 0)
    int ImageAec2 = 1;                // automatic exposure sensor  (0 or 1)
    int ImageAeLevel = 0;             // auto exposure levels (-2 to 2)
    int ImageAecValue = 160;          // set exposure manually  (0-1200)
    int ImageAgc = 1;                 // auto gain off (1 or 0)
    int ImageAgcGain = 15;            // set gain manually (0 - 30)
    int ImageBpc = 1;                 // black pixel correction
    int ImageWpc = 1;                 // white pixel correction
    int ImageRawGma = 1;              // (1 or 0)
    int ImageLenc = 1;                // lens correction (1 or 0)
    int ImageHmirror = 0;             // (0 or 1) flip horizontally
    int ImageVflip = 0;               // Invert image (0 or 1)
    int ImageDcw = 1;                 // downsize enable (1 or 0)

    int ImageDenoiseLevel = 0;        // The OV2640 does not support it, OV3660 and OV5640 (0 to 8)

    int ImageWidth = 640;
    int ImageHeight = 480;

    int ImageLedIntensity = 50;

    bool ImageZoomEnabled = false;
    int ImageZoomOffsetX = 0;
    int ImageZoomOffsetY = 0;
    int ImageZoomSize = 0;

    int WaitBeforePicture = 5;

    bool DemoMode = false;
    bool SaveAllFiles = false;
} camera_controll_config_temp_t;

extern camera_controll_config_temp_t CCstatus;

class CCamera
{
protected:
    void ledc_init(void);
    void CheckCamSettingsChanged(void);
    bool loadNextDemoImage(camera_fb_t *fb);
    long GetFileSize(std::string filename);
    void SetCamWindow(sensor_t *sensor, int frameSizeX, int frameSizeY, int xOffset, int yOffset, int xTotal, int yTotal, int xOutput, int yOutput, int imageVflip);
    void SetImageWidthHeightFromResolution(framesize_t resol);
    void SanitizeZoomParams(int imageSize, int frameSizeX, int frameSizeY, int &imageWidth, int &imageHeight, int &zoomOffsetX, int &zoomOffsetY);

public:
    int LedIntensity = 4096;

    bool CaptureToBasisImageLed = false;
    bool CaptureToFileLed = false;
    bool CaptureToHTTPLed = false;
    bool CaptureToStreamLed = false;
	
    uint16_t CamSensor_id;
    bool CamInitSuccessful = false;
    bool CamSettingsChanged = false;
    bool CamTempImage = false;

    CCamera(void);
    esp_err_t InitCam(void);

    void FlashLightOnOff(bool status);
    void StatusLEDOnOff(bool status);

    esp_err_t setSensorDatenFromCCstatus(void);
    esp_err_t getSensorDatenToCCstatus(void);

    int SetCamGainceiling(sensor_t *sensor, int gainceilingLevel);
    void SetCamSharpness(bool autoSharpnessEnabled, int sharpnessLevel);
    void SetCamSpecialEffect(sensor_t *sensor, int specialEffect);
    void SetCamContrastBrightness(sensor_t *sensor, int _contrast, int _brightness);

    esp_err_t CaptureToHTTP(httpd_req_t *req, int delay = 0);
    esp_err_t CaptureToStream(httpd_req_t *req, bool FlashlightOn);

    void SetQualityZoomSize(int qual, framesize_t resol, bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize, int imageVflip);
    void SetZoomSize(bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize, int imageVflip);

    int SetLEDIntensity(int _intrel);
    bool testCamera(void);
    bool getCameraInitSuccessful(void);
    void useDemoMode(void);

    framesize_t TextToFramesize(const char *text);

    esp_err_t CaptureToFile(std::string nm, int delay = 0);
    esp_err_t CaptureToBasisImage(CImageBasis *_Image, int delay = 0);
};

extern CCamera Camera;
#endif
