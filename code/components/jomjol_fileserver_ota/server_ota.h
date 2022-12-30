#pragma once

#ifndef SERVEROTA_H
#define SERVEROTA_H

#include <esp_log.h>

#include <esp_http_server.h>

#include <string>


void register_server_ota_sdcard_uri(httpd_handle_t server);
void CheckOTAUpdate();
void doReboot();
void doRebootOTA();
void hard_restart();
void CheckUpdate();

#endif //SERVEROTA_H