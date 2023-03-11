#pragma once

#ifndef SERVERTFLITE_H
#define SERVERTFLITE_H

#include <esp_log.h>
#include <string>

#include <esp_http_server.h>
#include "CImageBasis.h"
#include "ClassFlowControll.h"

extern ClassFlowControll tfliteflow;
void register_server_tflite_uri(httpd_handle_t server);

void KillTFliteTasks();
void TFliteDoAutoStart();
bool isSetupModusActive();
int getCountFlowRounds();

void CheckIsPlannedReboot();
bool getIsPlannedReboot();
void setTaskAutoFlowState(uint8_t _value);

esp_err_t GetJPG(std::string _filename, httpd_req_t *req);
esp_err_t GetRawJPG(httpd_req_t *req);

#ifdef ENABLE_MQTT
std::string GetMQTTMainTopic();
esp_err_t MQTTCtrlFlowStart(std::string);
#endif //ENABLE_MQTT

#endif //SERVERTFLITE_H
