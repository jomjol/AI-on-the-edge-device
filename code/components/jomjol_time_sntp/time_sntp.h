#include <string>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
// #include "nvs_flash.h"
#include "esp_sntp.h"

void setup_time(void);

std::string gettimestring(const char * frm);
std::string ConvertTimeToString(time_t _time, const char * frm);

void setTimeZone(std::string _tzstring);
void reset_servername(std::string _servername);

void setBootTime();
time_t getUpTime();
