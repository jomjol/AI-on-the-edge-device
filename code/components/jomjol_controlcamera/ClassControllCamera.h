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
    uint16_t CamSensor_id;

    framesize_t ImageFrameSize = FRAMESIZE_VGA; // 0 - 10
    gainceiling_t ImageGainceiling;             // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)

    int ImageQuality;    // 0 - 63
    int ImageBrightness; // (-2 to 2) - set brightness
    int ImageContrast;   //-2 - 2
    int ImageSaturation; //-2 - 2
    int ImageSharpness;  //-2 - 2
    bool ImageAutoSharpness;
    int ImageSpecialEffect; // 0 - 6
    int ImageWbMode;        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    int ImageAwb;           // white balance enable (0 or 1)
    int ImageAwbGain;       // Auto White Balance enable (0 or 1)
    int ImageAec;           // auto exposure off (1 or 0)
    int ImageAec2;          // automatic exposure sensor  (0 or 1)
    int ImageAeLevel;       // auto exposure levels (-2 to 2)
    int ImageAecValue;      // set exposure manually  (0-1200)
    int ImageAgc;           // auto gain off (1 or 0)
    int ImageAgcGain;       // set gain manually (0 - 30)
    int ImageBpc;           // black pixel correction
    int ImageWpc;           // white pixel correction
    int ImageRawGma;        // (1 or 0)
    int ImageLenc;          // lens correction (1 or 0)
    int ImageHmirror;       // (0 or 1) flip horizontally
    int ImageVflip;         // Invert image (0 or 1)
    int ImageDcw;           // downsize enable (1 or 0)

    int ImageDenoiseLevel; // The OV2640 does not support it, OV3660 and OV5640 (0 to 8)

    int ImageWidth;
    int ImageHeight;

    int ImageLedIntensity;

    bool ImageZoomEnabled;
    int ImageZoomOffsetX;
    int ImageZoomOffsetY;
    int ImageZoomSize;

    int WaitBeforePicture;
    bool isImageSize;

    bool CameraInitSuccessful;
    bool changedCameraSettings;
    bool DemoMode;
    bool SaveAllFiles;
} camera_controll_config_temp_t;

extern camera_controll_config_temp_t CCstatus;

class CCamera
{
protected:
    void ledc_init(void);
    bool loadNextDemoImage(camera_fb_t *fb);
    long GetFileSize(std::string filename);
    void SetCamWindow(sensor_t *s, int frameSizeX, int frameSizeY, int xOffset, int yOffset, int xTotal, int yTotal, int xOutput, int yOutput, int imageVflip);
    void SetImageWidthHeightFromResolution(framesize_t resol);
    void SanitizeZoomParams(int imageSize, int frameSizeX, int frameSizeY, int &imageWidth, int &imageHeight, int &zoomOffsetX, int &zoomOffsetY);

public:
    CCamera(void);
    esp_err_t InitCam(void);

    void LightOnOff(bool status);
    void LEDOnOff(bool status);

    esp_err_t setSensorDatenFromCCstatus(void);
    esp_err_t getSensorDatenToCCstatus(void);

    int ov5640_set_gainceiling(sensor_t *s, gainceiling_t level);

    esp_err_t CaptureToHTTP(httpd_req_t *req, int delay = 0);
    esp_err_t CaptureToStream(httpd_req_t *req, bool FlashlightOn);

    void SetQualityZoomSize(int qual, framesize_t resol, bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize, int imageVflip);
    void SetZoomSize(bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize, int imageVflip);
    void SetCamSharpness(bool _autoSharpnessEnabled, int _sharpnessLevel);

    void SetLEDIntensity(float _intrel);
    bool testCamera(void);
    bool getCameraInitSuccessful(void);
    void useDemoMode(void);

    framesize_t TextToFramesize(const char *text);

    esp_err_t CaptureToFile(std::string nm, int delay = 0);
    esp_err_t CaptureToBasisImage(CImageBasis *_Image, int delay = 0);
};

extern CCamera Camera;
#endif