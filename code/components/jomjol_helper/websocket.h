#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "esp_http_server.h"

esp_err_t start_websocket_server(httpd_handle_t server);

esp_err_t schedule_websocket_message(std::string message);

#endif // WEBSOCKET_H
