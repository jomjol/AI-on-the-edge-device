#pragma once

#ifndef SERVERHELP_H
#define SERVERHELP_H

#include <string>
#include <string.h>
#include "esp_http_server.h"

const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize);
esp_err_t send_file(httpd_req_t *req, std::string filename);
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

void unzip_file(std::string _in_zip_file, std::string _target_directory);
std::string unzip_firmware(std::string _in_zip_file, std::string _html_tmp, std::string _html_final, std::string _target_bin, std::string _main, bool _initial_setup);

#endif // SERVERHELP_H