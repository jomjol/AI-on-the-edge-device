#pragma once

#ifndef SERVEROTA_H
#define SERVEROTA_H

#include <esp_log.h>

#include <esp_http_server.h>

#include <string>

void register_server_ota_sdcard_uri(httpd_handle_t server);
void CheckOTAUpdate(void);
void doReboot(void);
void doRebootOTA(void);
void hard_restart(void);
void CheckUpdate(void);

#endif // SERVEROTA_H