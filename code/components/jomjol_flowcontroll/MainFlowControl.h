#pragma once

#ifndef MAINFLOWCONTROL_H
#define MAINFLOWCONTROL_H

#include <esp_log.h>
#include <string>

#include <esp_http_server.h>
#include "CImageBasis.h"
#include "ClassFlowControll.h"
#include "openmetrics.h"

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
} camera_flow_config_temp_t;

extern camera_flow_config_temp_t CFstatus;
extern ClassFlowControll flowctrl;

esp_err_t setCCstatusToCFstatus(void); // CCstatus >>> CFstatus
esp_err_t setCFstatusToCCstatus(void); // CFstatus >>> CCstatus
esp_err_t setCFstatusToCam(void);      // CFstatus >>> Kamera

void register_server_main_flow_task_uri(httpd_handle_t server);

void CheckIsPlannedReboot(void);
bool getIsPlannedReboot(void);

void InitializeFlowTask(void);
void DeleteMainFlowTask(void);
bool isSetupModusActive(void);

int getCountFlowRounds(void);

#ifdef ENABLE_MQTT
esp_err_t MQTTCtrlFlowStart(std::string _topic);
#endif // ENABLE_MQTT

esp_err_t GetRawJPG(httpd_req_t *req);
esp_err_t GetJPG(std::string _filename, httpd_req_t *req);

#endif // MAINFLOWCONTROL_H
