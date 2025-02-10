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
#include "camera_config.h"
#include "../../include/defines.h"

typedef struct
{
    uint16_t CamSensor_id;

    cam_config_t CamConfig;

    uint16_t CameraFocusLevel; // temporary storage for the latest focus level
    int WaitBeforePicture;
    bool isImageSize;
    bool isTempImage;

    bool CameraInitSuccessful;
    bool CameraAFInitSuccessful;
    bool CameraDeepSleepEnable;
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

    int SetCamGainceiling(sensor_t *s, gainceiling_t gainceilingLevel);
    void SetCamSharpness(bool autoSharpnessEnabled, int sharpnessLevel);
    void SetCamSpecialEffect(sensor_t *s, int specialEffect);
    void SetCamContrastBrightness(sensor_t *s, int _contrast, int _brightness);
    void SetCamWindow(sensor_t *s, int frameSizeX, int frameSizeY, int xOffset, int yOffset, int xTotal, int yTotal, int xOutput, int yOutput, int imageVflip);
    void SetImageWidthHeightFromResolution(cam_config_t *camConfig, framesize_t resol);
    void SanitizeZoomParams(cam_config_t *camConfig, int imageSize, int frameSizeX, int frameSizeY, int &imageWidth, int &imageHeight, int &zoomOffsetX, int &zoomOffsetY);
    void SetQualityZoomSize(cam_config_t *camConfig);
    void SetZoomSize(cam_config_t *camConfig, bool zoomEnabled, int zoomOffsetX, int zoomOffsetY, int imageSize, int imageVflip);
    void ResetZoomSizeOnCamera(cam_config_t *camConfig);
    int SetCamFocus(bool focusEnabled, bool manualFocus, uint16_t manualFocusLevel);
    int ReleaseCamFocus(bool focusEnabled, bool manualFocus);

    void readSensorConfig(sensor_t *s, cam_config_t *camConfig);

    int PrecaptureCamSetup(bool *focusEnabled, bool *manualFocus, bool *needReloadZoomConfig);

    bool initCameraAF(void);
    int CameraDeepSleep(bool sleep);

public:
    CCamera(void);
    esp_err_t InitCam(void);

    void LightOnOff(bool status);
    void LEDOnOff(bool status);

    esp_err_t configureSensor(cam_config_t *camConfig);
    esp_err_t setSensorDatenFromCCstatus(void);
    esp_err_t getSensorDatenToCCstatus(void);

    esp_err_t CaptureToHTTP(httpd_req_t *req, int delay = 0);
    esp_err_t CaptureToStream(httpd_req_t *req, bool FlashlightOn);

    int CalculateLEDIntensity(int _intrel);
    bool testCamera(void);
    bool getCameraInitSuccessful(void);
    bool getCameraAFInitSuccessful(void);
    void useDemoMode(void);

    framesize_t TextToFramesize(const char *text);

    esp_err_t CaptureToFile(std::string nm, int delay = 0);
    esp_err_t CaptureToBasisImage(CImageBasis *_Image, int delay = 0);
};

extern CCamera Camera;
#endif