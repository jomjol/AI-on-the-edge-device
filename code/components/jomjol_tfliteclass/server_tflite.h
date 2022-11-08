#include <esp_log.h>
#include <string>

#include <esp_http_server.h>
#include "CImageBasis.h"
#include "ClassFlowControll.h"

//#include "ClassControllCamera.h"

void register_server_tflite_uri(httpd_handle_t server);

void KillTFliteTasks();

void TFliteDoAutoStart();

bool isSetupModusActive();

std::string GetMQTTMainTopic();

int getCountFlowRounds();

esp_err_t GetJPG(std::string _filename, httpd_req_t *req);

esp_err_t GetRawJPG(httpd_req_t *req);

extern ClassFlowControll tfliteflow;
