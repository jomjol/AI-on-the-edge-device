#pragma once

#ifndef SERVERHELP_H
#define SERVERHELP_H

#include <string>
#include "esp_http_server.h"

const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize);
esp_err_t send_file(httpd_req_t *req, std::string filename);
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

void delete_all_in_directory(std::string _directory);
void delete_all_file_in_directory(std::string _directory);

#endif // SERVERHELP_H
