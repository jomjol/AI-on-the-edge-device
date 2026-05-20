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
    cam_config_t CamConfig;

    int WaitBeforePicture;
    bool isImageSize;

    bool CameraInitSuccessful;
    bool CameraInitAFSuccessful;
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
