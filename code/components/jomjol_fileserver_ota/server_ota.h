#include <esp_log.h>

#include <esp_http_server.h>

//#include "ClassControllCamera.h"

static const char *TAGPARTOTA = "server_ota";

void register_server_ota_sdcard_uri(httpd_handle_t server);
void CheckOTAUpdate();
void doReboot();
void hard_restart();

