#pragma once

#include <esp_http_server.h>

void init_basic_auth();
esp_err_t basic_auth_request_filter(httpd_req_t *req, esp_err_t original_handler(httpd_req_t *));

#define APPLY_BASIC_AUTH_FILTER(method) [](httpd_req_t *req){ return basic_auth_request_filter(req, method); }
