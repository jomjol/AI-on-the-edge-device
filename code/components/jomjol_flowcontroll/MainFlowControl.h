#pragma once

#ifndef MAINFLOWCONTROL_H
#define MAINFLOWCONTROL_H

#include <esp_log.h>
#include <string>

#include <esp_http_server.h>
#include "CImageBasis.h"
#include "ClassFlowControll.h"

extern ClassFlowControll flowctrl;


void register_server_main_flow_task_uri(httpd_handle_t server);

void CheckIsPlannedReboot();
bool getIsPlannedReboot();

void InitializeFlowTask();
void DeleteMainFlowTask();
bool isSetupModusActive();

int getCountFlowRounds();

#ifdef ENABLE_MQTT
esp_err_t MQTTCtrlFlowStart(std::string _topic);
#endif //ENABLE_MQTT

esp_err_t GetRawJPG(httpd_req_t *req);
esp_err_t GetJPG(std::string _filename, httpd_req_t *req);

#endif //MAINFLOWCONTROL_H
