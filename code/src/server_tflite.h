#include <esp_log.h>

#include <esp_http_server.h>

//#include "ClassControllCamera.h"

static const char *TAGTFLITE = "server_tflite";

void register_server_tflite_uri(httpd_handle_t server);

void KillTFliteTasks();

void TFliteDoAutoStart();