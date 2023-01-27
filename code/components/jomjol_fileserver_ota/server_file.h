#pragma once

#ifndef SERVERFILE_H
#define SERVERFILE_H

#include <esp_http_server.h>
#include <string>

void register_server_file_uri(httpd_handle_t server, const char *base_path);

void unzip(std::string _in_zip_file, std::string _target_directory);
std::string unzip_new(std::string _in_zip_file, std::string _target_zip, std::string _target_bin, std::string _main = "/sdcard/", bool _initial_setup = false);


void delete_all_in_directory(std::string _directory);

esp_err_t get_tflite_file_handler(httpd_req_t *req);
esp_err_t get_data_file_handler(httpd_req_t *req);
esp_err_t get_numbers_file_handler(httpd_req_t *req);

#endif //SERVERFILE_H