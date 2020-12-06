#include "time_sntp.h"

/* LwIP SNTP example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
// #include "protocol_examples_common.h"
#include "esp_sntp.h"

#include "ClassLogFile.h"

static const char *TAG = "sntp";

RTC_DATA_ATTR int boot_count = 0;

bool setTimeAlwaysOnReboot = true;

static void obtain_time(void);
static void initialize_sntp(void);


void time_sync_notification_cb(struct timeval *tv)
{
//    LogFile.WriteToFile("Notification of a time synchronization event");
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

std::string gettimestring(const char * frm)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    char strftime_buf[64];
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), frm, &timeinfo);

    std::string result(strftime_buf);
    return result;
}

void setup_time()
{
    ++boot_count;
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if ((timeinfo.tm_year < (2016 - 1900)) || setTimeAlwaysOnReboot) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        initialize_sntp();
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    char strftime_buf[64];

    setTimeZone("CET-1CEST,M3.5.0,M10.5.0/3");
//    setTimeZone("Europe/Berlin");
//    setTimeZone("Asia/Tokyo");

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);

    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d_%H:%M", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);

    std::string zw = gettimestring("%Y%m%d-%H%M%S");
    printf("timeist %s\n", zw.c_str());
}

void setTimeZone(std::string _tzstring)
{
    setenv("TZ", _tzstring.c_str(), 1);
    tzset();    
    printf("TimeZone set to %s\n", _tzstring.c_str());
    _tzstring = "Time zone set to " + _tzstring;
    LogFile.WriteToFile(_tzstring);
}

static void obtain_time(void)
{
//    initialize_sntp();
 
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {};
    int retry = 0;
    const int retry_count = 10;
    time(&now);
    localtime_r(&now, &timeinfo);    
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if (retry == retry_count) {
//        LogFile.WriteToFile("Time Synchzronisation nicht erfolgreich ...");
    }
    else
    {
//        LogFile.WriteToFile("Time erfolgreich ...");
    }
    
    time(&now);
    localtime_r(&now, &timeinfo);
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
//    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}