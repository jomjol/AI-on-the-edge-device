#ifndef WEBSOCKET_H
#define WEBSOCKET_H

esp_err_t start_websocket_server(httpd_handle_t server);

esp_err_t schedule_websocket_message(std::string message);

#endif // WEBSOCKET_H
