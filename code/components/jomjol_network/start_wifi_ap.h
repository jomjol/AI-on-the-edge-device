#include "defines.h"

#ifdef ENABLE_SOFTAP

#ifndef START_WIFI_AP_H
#define START_WIFI_AP_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_eth.h>
#include <esp_tls_crypto.h>
#include <esp_http_server.h>

void CheckStartAPMode();

#endif // START_WIFI_AP_H

#endif // #ifdef ENABLE_SOFTAP
