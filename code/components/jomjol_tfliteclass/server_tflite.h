#include <esp_log.h>

#include <esp_http_server.h>
#include "CImageBasis.h"

//#include "ClassControllCamera.h"

static const char *TAGTFLITE = "server_tflite";

void register_server_tflite_uri(httpd_handle_t server);

void KillTFliteTasks();

void TFliteDoAutoStart();

bool isSetupModusActive();

esp_err_t GetJPG(std::string _filename, httpd_req_t *req);

esp_err_t GetRawJPG(httpd_req_t *req);