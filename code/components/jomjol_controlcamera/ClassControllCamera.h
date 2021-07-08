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


#define CAMERA_MODEL_AI_THINKER


class CCamera {
    protected:
        int ActualQuality;
        framesize_t ActualResolution;
        int brightness, contrast, saturation;
        bool isFixedExposure;
        int waitbeforepicture_org;

    public:
        int image_height, image_width;
        
        CCamera();
        esp_err_t InitCam();

        void LightOnOff(bool status);
        void LEDOnOff(bool status);
        esp_err_t CaptureToHTTP(httpd_req_t *req, int delay = 0);
        void SetQualitySize(int qual, framesize_t resol);
        bool SetBrightnessContrastSaturation(int _brightness, int _contrast, int _saturation);
        void GetCameraParameter(httpd_req_t *req, int &qual, framesize_t &resol);

        void EnableAutoExposure(int flashdauer);
        

        framesize_t TextToFramesize(const char * text);

        esp_err_t CaptureToFile(std::string nm, int delay = 0);
        esp_err_t CaptureToBasisImage(CImageBasis *_Image, int delay = 0);
};


extern CCamera Camera;

#endif