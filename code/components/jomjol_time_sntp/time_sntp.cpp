#include "time_sntp.h"

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
#include "esp_sntp.h"

#include "ClassLogFile.h"

static const char *TAG = "SNTP";

time_t bootTime;

static void obtain_time(void);
static void initialize_sntp(void);
static void logNtpStatus(sntp_sync_status_t status);

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

std::string ConvertTimeToString(time_t _time, const char * frm)
{
    struct tm timeinfo;
    char strftime_buf[64];
    localtime_r(&_time, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), frm, &timeinfo);

    std::string result(strftime_buf);
    return result;
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
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char strftime_buf[64];

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (!getTimeIsSet()) {
        ESP_LOGI(TAG, "Time is not set yet. Getting time over NTP.");
        initialize_sntp();
        obtain_time();
        // update 'now' variable with current time
        time(&now);

        setTimeZone("CET-1CEST,M3.5.0,M10.5.0/3");

        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d_%H:%M:%S", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);
    }
    else {
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d_%H:%M:%S", &timeinfo);
        ESP_LOGI(TAG, "Time is already set (%s)", strftime_buf);
    }
}

void setTimeZone(std::string _tzstring)
{
    setenv("TZ", _tzstring.c_str(), 1);
    tzset();    
    _tzstring = "Time zone set to " + _tzstring;
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, _tzstring);
}

static void obtain_time(void)
{
    time_t now = 0;
    struct tm timeinfo = {};
    int retry = 0;
    const int retry_count = 10;
    time(&now);
    localtime_r(&now, &timeinfo);    

    ESP_LOGI(TAG, "Waiting until we get a time from the NTP server...");
    while (true) {
        retry++;

        if (retry == retry_count) {
            ESP_LOGW(TAG, "NTP time fetching seems to take longer, will check again on next round!", retry); // The NTP client will automatically retry periodically!
            break;
        }

        sntp_sync_status_t status = sntp_get_sync_status();
        logNtpStatus(status);
        if (status == SNTP_SYNC_STATUS_COMPLETED) {
            ESP_LOGI(TAG, "Time is synced with NTP Server");
            break;
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    time(&now);
    localtime_r(&now, &timeinfo);
}


void logNtpStatus(sntp_sync_status_t status) {
    if (status == SNTP_SYNC_STATUS_COMPLETED) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "NTP Status OK");
    }
    else if (status == SNTP_SYNC_STATUS_IN_PROGRESS) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "NTP Status: In Progress");
    }
    else { // SNTP_SYNC_STATUS_RESET
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "NTP Status: Reset");
    }
}


void reset_servername(std::string _servername)
{
    ESP_LOGD(TAG, "Set SNTP-Server: %s", _servername.c_str());
    sntp_stop();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, _servername.c_str());
    sntp_init();
    obtain_time();
    std::string zw = gettimestring("%Y%m%d-%H%M%S");
    ESP_LOGD(TAG, "Time ist %s", zw.c_str());
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
//    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

void setBootTime() 
{
    time(&bootTime);
}

time_t getUpTime() 
{
    time_t now;
    time(&now);

    return now - bootTime;
}

bool getTimeIsSet(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char strftime_buf[64];

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d_%H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);

    // Is time set? If not, tm_year will be (1970 - 1900).
    if ((timeinfo.tm_year < (2022 - 1900))) {
        return false;
    }
    else {
        return true;
    }
}

void restartNtpClient(void) {
    sntp_restart();
    obtain_time();
}
