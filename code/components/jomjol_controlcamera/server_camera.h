#ifndef JOMJOL_CONTROLCAMERA_H
#define JOMJOL_CONTROLCAMERA_H

#include <esp_log.h>

#include <esp_http_server.h>

//#include "ClassControllCamera.h"

static const char *TAGPARTCAMERA = "server_camera";

void register_server_camera_uri(httpd_handle_t server);

void PowerResetCamera();


extern bool debug_detail_heap;

#endif