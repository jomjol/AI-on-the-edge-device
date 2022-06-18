#include <esp_http_server.h>
#include <string>

void register_server_file_uri(httpd_handle_t server, const char *base_path);

void unzip(std::string _in_zip_file, std::string _target_directory);

void delete_all_in_directory(std::string _directory);

esp_err_t get_tflite_file_handler(httpd_req_t *req);
