#include <esp_log.h>

#include <esp_http_server.h>

//#include "ClassControllCamera.h"

static const char *TAGPARTGPIO = "server_GPIO";

void register_server_GPIO_uri(httpd_handle_t server);


extern bool debug_detail_heap;