#pragma once

#ifndef API_INFO_H
#define API_INFO_H

#include <esp_http_server.h>

// GET /api/v1/info: everything /info?type=... and /sysinfo report, except the flow state, as one JSON object.
esp_err_t api_info_get_handler(httpd_req_t *req);

#endif // API_INFO_H
