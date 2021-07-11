#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "server_GPIO.h"

#include <esp_http_server.h>

extern httpd_handle_t server;

httpd_handle_t start_webserver(void);

void register_server_main_uri(httpd_handle_t server, const char *base_path);


#endif
