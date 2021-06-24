#ifndef JOMJOL_CONTROLCAMERA_H
#define JOMJOL_CONTROLCAMERA_H

#include <esp_log.h>

#include <esp_http_server.h>

//#include "ClassControllCamera.h"

void register_server_camera_uri(httpd_handle_t server);

void PowerResetCamera();

#endif