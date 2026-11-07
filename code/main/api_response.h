#pragma once

#ifndef API_RESPONSE_H
#define API_RESPONSE_H

#include <esp_http_server.h>
#include <cJSON.h>

// Sends body as a 200 JSON response and deletes it.
// A NULL body, or one that can't be printed, means memory ran out and gives a 500.
esp_err_t api_send_json(httpd_req_t *req, cJSON *body);

// Sends {"error": {"code": ..., "message": ...}} with the given HTTP status line, e.g. "404 Not Found".
esp_err_t api_send_error(httpd_req_t *req, const char *status, const char *code, const char *message);

#endif // API_RESPONSE_H
