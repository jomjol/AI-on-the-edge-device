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


#define CAMERA_MODEL_AI_THINKER


static const char *TAGCAMERACLASS = "server_part_camera";


class CCamera {
    protected:
        int ActualQuality;
        framesize_t ActualResolution;

    public:
        CCamera();
        esp_err_t InitCam();

        void LightOnOff(bool status);
        esp_err_t CaptureToHTTP(httpd_req_t *req, int delay = 0);
        void SetQualitySize(int qual, framesize_t resol);
        void GetCameraParameter(httpd_req_t *req, int &qual, framesize_t &resol);

        framesize_t TextToFramesize(const char * text);


        esp_err_t CaptureToFile(std::string nm, int delay = 0);
        
};


extern CCamera Camera;


#endif