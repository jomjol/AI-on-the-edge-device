#include "time_sntp.h"

#include <string>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_netif_sntp.h"

#include "../../include/defines.h"

#include "ClassLogFile.h"

#include "configFile.h"
#include "Helper.h"


static const char *TAG = "SNTP";

static std::string timeZone = "";
static std::string timeServer = "undefined";
static bool useNtp = true;
static bool timeWasNotSetAtBoot = false;
static bool timeWasNotSetAtBoot_PrintStartBlock = false;

std::string getNtpStatusText(sntp_sync_status_t status);
static void setTimeZone(std::string _tzstring);
static std::string getServerName(void);

int LocalTimeToUTCOffsetSeconds;



std::string ConvertTimeToString(time_t _time, const char * frm)
{
    struct tm timeinfo;
    char strftime_buf[64];
    localtime_r(&_time, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), frm, &timeinfo);

    std::string result(strftime_buf);
    return result;
}


std::string getCurrentTimeString(const char * frm)
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


void time_sync_notification_cb(struct timeval *tv)
{
    if (timeWasNotSetAtBoot_PrintStartBlock) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================================");
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "==================== Start ======================");
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "== Logs before time sync -> log_1970-01-01.txt ==");
        timeWasNotSetAtBoot_PrintStartBlock = false;
    }
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Time is synced with NTP Server " +
            getServerName() + ": " + getCurrentTimeString("%Y-%m-%d %H:%M:%S"));
}


bool time_manual_reset_sync(void)
{
    sntp_restart();
//    sntp_init();
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Waiting for system time to be set... " + std::to_string(retry) + "/" + std::to_string(retry_count));
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if (retry >= retry_count)
        return false;

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Waiting for system time successfull with " + std::to_string(retry) + "/" + std::to_string(retry_count));
    return true;
}


int getUTCOffsetSeconds(std::string &zeitzone)
{
    int offset = 0;
    int vorzeichen = 1;
    int minuten = 0;
    int stunden = 0;
    time_t now;
    struct tm timeinfo;

    time (&now);
    localtime_r(&now, &timeinfo);
    char buffer[80];
    strftime(buffer, 80, "%z", &timeinfo);
    zeitzone = std::string(buffer);

    if (zeitzone.length() == 5)
    {
        if (zeitzone[0] == '-')
            vorzeichen = -1; 

        stunden = stoi(zeitzone.substr(1, 2));
        minuten = stoi(zeitzone.substr(3, 2));

        offset = ((stunden * 60) + minuten) * 60;
    }
    return offset;
}


void setTimeZone(std::string _tzstring)
{
    setenv("TZ", _tzstring.c_str(), 1);
    tzset();    

    _tzstring = "Time zone set to " + _tzstring;
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, _tzstring);

    std::string zeitzone;
    LocalTimeToUTCOffsetSeconds = getUTCOffsetSeconds(zeitzone);
//    std::string zw = std::to_string(LocalTimeToUTCOffsetSeconds);
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "time zone: " + zeitzone + " Delta to UTC: " + std::to_string(LocalTimeToUTCOffsetSeconds) + " seconds");
}



std::string getNtpStatusText(sntp_sync_status_t status) {
    if (status == SNTP_SYNC_STATUS_COMPLETED) {
        return "Synchronized";
    }
    else if (status == SNTP_SYNC_STATUS_IN_PROGRESS) {
        return "In Progress";
    }
    else { // SNTP_SYNC_STATUS_RESET
        return "Reset";
    }
}


bool getTimeIsSet(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Is time set? If not, tm_year will be (1970 - 1900).
    if ((timeinfo.tm_year < (2022 - 1900))) {
        return false;
    }
    else {
        return true;
    }
}

/*void restartNtpClient(void) {
//    sntp_restart();
 //   obtain_time();
}*/


bool getUseNtp(void) {
    return useNtp;
}

bool getTimeWasNotSetAtBoot(void)
{
    return timeWasNotSetAtBoot;
}


std::string getServerName(void) {
    char buf[100];

    if (sntp_getservername(0)){
        snprintf(buf, sizeof(buf), "%s", sntp_getservername(0));
        return std::string(buf);
    }
    else { // we have either IPv4 or IPv6 address
        ip_addr_t const *ip = sntp_getserver(0);
        if (ipaddr_ntoa_r(ip, buf, sizeof(buf)) != NULL) {
            return std::string(buf);
        }
    }
    return "";
}


/**
 * Load the TimeZone and TimeServer from the config file and initialize the NTP client
 */
bool setupTime() {
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    ConfigFile configFile = ConfigFile(CONFIG_FILE); 

    if (!configFile.ConfigFileExists()){
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "No ConfigFile defined - exit setupTime()!");
        return false;
    }

    std::vector<std::string> splitted;
    std::string line = "";
    bool disabledLine = false;
    bool eof = false;

    /* Load config from config file */
    while ((!configFile.GetNextParagraph(line, disabledLine, eof) || 
            (line.compare("[System]") != 0)) && !eof) {}
    if (eof) {
        return false;
    }

    if (disabledLine) {
        return false;
    }

    while (configFile.getNextLine(&line, disabledLine, eof) && 
            !configFile.isNewParagraph(line)) {
        splitted = ZerlegeZeile(line, "=");

        if (toUpper(splitted[0]) == "TIMEZONE") {
            if (splitted.size() <= 1) { // parameter part is empty
                timeZone = "";
            }
            else {
                timeZone = splitted[1];
            }
        }

        if (toUpper(splitted[0]) == "TIMESERVER") {
            if (splitted.size() <= 1) { // Key has no value => we use this to show it as disabled
                timeServer = "";
            }
            else {
                timeServer = splitted[1];
            }
        }
    }


    /* Setup NTP Server and Timezone */
    if (timeServer == "undefined") {
        timeServer = "pool.ntp.org";
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "TimeServer not defined, using default: " + timeServer);
    }
    else if (timeServer == "") {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "TimeServer config empty, disabling NTP");
        useNtp = false;
    }
    else {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "TimeServer: " + timeServer);
    }
    
    if (timeZone == "") {
        timeZone = "CET-1CEST,M3.5.0,M10.5.0/3";
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "TimeZone not set, using default: " + timeZone);
    }


    if (useNtp) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Configuring NTP Client...");        
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(timeServer.c_str());
        config.sync_cb = time_sync_notification_cb;
        esp_netif_sntp_init(&config);

        setTimeZone(timeZone);
    }

    /* The RTC keeps the time after a restart (Except on Power On or Pin Reset) 
     * There should only be a minor correction through NTP */

    // Get current time from RTC
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);


    if (getTimeIsSet()) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Time is already set: " + std::string(strftime_buf));
    }
    else {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "The local time is unknown, starting with " + std::string(strftime_buf));
        if (useNtp) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Once the NTP server provides a time, we will switch to that one");
            timeWasNotSetAtBoot = true;
            timeWasNotSetAtBoot_PrintStartBlock = true;
        }
    }

    return true;
}


