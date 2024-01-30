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

class CCamera {
    protected:
        int ActualQuality;
        framesize_t ActualResolution;
        int brightness, contrast, saturation, autoExposureLevel;
        bool isFixedExposure;
        int waitbeforepicture_org;
        int led_intensity = 4095;

        void ledc_init(void);
        bool CameraInitSuccessful = false;
        bool demoMode = false;

        bool loadNextDemoImage(camera_fb_t *fb);
        long GetFileSize(std::string filename);

        void SetCamWindow(sensor_t *s, int resolution, int xOffset, int yOffset, int xLength, int yLength);
        void SetImageWidthHeightFromResolution(framesize_t resol);

    public:
        int image_height, image_width;
        bool imageZoomEnabled = false;
        int imageZoomMode = 0;
        int imageZoomOffsetX = 0;
        int imageZoomOffsetY = 0;
        bool imageNegative = false;
        bool imageAec2 = false;
        bool imageAutoSharpness = false;
        int imageSharpnessLevel = 0;
    #ifdef GRAYSCALE_AS_DEFAULT
        bool imageGrayscale = true;
    #else
        bool imageGrayscale = false;
    #endif
        
        CCamera();
        esp_err_t InitCam();

        void LightOnOff(bool status);
        void LEDOnOff(bool status);
        esp_err_t CaptureToHTTP(httpd_req_t *req, int delay = 0);
        esp_err_t CaptureToStream(httpd_req_t *req, bool FlashlightOn);
        void SetQualitySize(int qual, framesize_t resol, bool zoomEnabled, int zoomMode, int zoomOffsetX, int zoomOffsetY);
        bool SetBrightnessContrastSaturation(int _brightness, int _contrast, int _saturation, int _autoExposureLevel, bool _grayscale, bool _negative, bool _aec2, int _sharpnessLevel);
        void SetZoom(bool zoomEnabled, int zoomMode, int zoomOffsetX, int zoomOffsetY);
        void GetCameraParameter(httpd_req_t *req, int &qual, framesize_t &resol, bool &zoomEnabled, int &zoomMode, int &zoomOffsetX, int &zoomOffsetY);
        void SetLEDIntensity(float _intrel);
        bool testCamera(void);
        void EnableAutoExposure(int flash_duration);
        bool getCameraInitSuccessful();
        void useDemoMode(void);
       

        framesize_t TextToFramesize(const char * text);

        esp_err_t CaptureToFile(std::string nm, int delay = 0);
        esp_err_t CaptureToBasisImage(CImageBasis *_Image, int delay = 0);
};


extern CCamera Camera;

#endif