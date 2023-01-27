#ifdef ENABLE_MQTT
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#include "esp_log.h"
#include "ClassLogFile.h"
#include "connect_wlan.h"
#include "server_mqtt.h"
#include "interface_mqtt.h"
#include "time_sntp.h"
#include "../../include/defines.h"



static const char *TAG = "MQTT SERVER";


extern const char* libfive_git_version(void);
extern const char* libfive_git_revision(void);
extern const char* libfive_git_branch(void);

std::vector<NumberPost*>* NUMBERS;
bool HomeassistantDiscovery = false;
std::string meterType = "";
std::string valueUnit = "";
std::string timeUnit = "";
std::string rateUnit = "Unit/Minute";
float roundInterval; // Minutes
int keepAlive = 0; // Seconds
int retainFlag;
static std::string maintopic;


void mqttServer_setParameter(std::vector<NumberPost*>* _NUMBERS, int _keepAlive, float _roundInterval) {
    NUMBERS = _NUMBERS;
    keepAlive = _keepAlive;
    roundInterval = _roundInterval; 
}

void mqttServer_setMeterType(std::string _meterType, std::string _valueUnit, std::string _timeUnit,std::string _rateUnit) {
    meterType = _meterType;
    valueUnit = _valueUnit;
    timeUnit = _timeUnit;
    rateUnit = _rateUnit;
}

void sendHomeAssistantDiscoveryTopic(std::string group, std::string field,
    std::string name, std::string icon, std::string unit, std::string deviceClass, std::string stateClass, std::string entityCategory) {
    std::string version = std::string(libfive_git_version());

    if (version == "") {
        version = std::string(libfive_git_branch()) + " (" + std::string(libfive_git_revision()) + ")";
    }
    
    std::string topicFull;
    std::string configTopic;
    std::string payload;

    configTopic = field;

    if (group != "" && (*NUMBERS).size() > 1) { // There is more than one meter, prepend the group so we can differentiate them
        configTopic = group + "_" + field;
        name = group + " " + name;
    }    

    if (field == "problem") { // Special binary sensor which is based on error topic
        topicFull = "homeassistant/binary_sensor/" + maintopic + "/" + configTopic + "/config";
    }
    else {
        topicFull = "homeassistant/sensor/" + maintopic + "/" + configTopic + "/config";
    }

    /* See https://www.home-assistant.io/docs/mqtt/discovery/ */
    payload = string("{")  +
        "\"~\": \"" + maintopic + "\","  +
        "\"unique_id\": \"" + maintopic + "-" + configTopic + "\","  +
        "\"object_id\": \"" + maintopic + "_" + configTopic + "\","  + // This used to generate the Entity ID
        "\"name\": \"" + name + "\","  +
        "\"icon\": \"mdi:" + icon + "\",";        

    if (group != "") {
        if (field == "problem") { // Special binary sensor which is based on error topic
            payload += "\"state_topic\": \"~/" + group + "/error\",";
            payload += "\"value_template\": \"{{ 'OFF' if 'no error' in value else 'ON'}}\",";
        }
        else {
            payload += "\"state_topic\": \"~/" + group + "/" + field + "\",";
        }
    }
    else {
        if (field == "problem") { // Special binary sensor which is based on error topic
            payload += "\"state_topic\": \"~/error\",";
            payload += "\"value_template\": \"{{ 'OFF' if 'no error' in value else 'ON'}}\",";
        }
        else {
            payload += "\"state_topic\": \"~/" + field + "\",";
        }
    }

    if (unit != "") {
        payload += "\"unit_of_meas\": \"" + unit + "\",";
    }

    if (deviceClass != "") {
        payload += "\"device_class\": \"" + deviceClass + "\",";
    }

    if (stateClass != "") {
        payload += "\"state_class\": \"" + stateClass + "\",";
    } 

    if (entityCategory != "") {
        payload += "\"entity_category\": \"" + entityCategory + "\",";
    } 

    payload += 
        "\"availability_topic\": \"~/" + std::string(LWT_TOPIC) + "\","  +
        "\"payload_available\": \"" + LWT_CONNECTED + "\","  +
        "\"payload_not_available\": \"" + LWT_DISCONNECTED + "\",";

    payload += string("\"device\": {")  +
        "\"identifiers\": [\"" + maintopic + "\"],"  +
        "\"name\": \"" + maintopic + "\","  +
        "\"model\": \"Meter Digitizer\","  +
        "\"manufacturer\": \"AI on the Edge Device\","  +
      "\"sw_version\": \"" + version + "\","  +
      "\"configuration_url\": \"http://" + *getIPAddress() + "\""  +
    "}"  +
    "}";

    MQTTPublish(topicFull, payload, true);
}

void MQTThomeassistantDiscovery() {  
    if (!getMQTTisConnected()) 
        return;

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "MQTT - Sending Homeassistant Discovery Topics (Meter Type: " + meterType + ", Value Unit: " + valueUnit + " , Rate Unit: " + rateUnit + ")...");

    //                              Group | Field            | User Friendly Name | Icon                      | Unit | Device Class     | State Class  | Entity Category
    sendHomeAssistantDiscoveryTopic("",     "uptime",          "Uptime",            "clock-time-eight-outline", "s",   "",                "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "MAC",             "MAC Address",       "network-outline",          "",    "",                "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "hostname",        "Hostname",          "network-outline",          "",    "",                "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "freeMem",         "Free Memory",       "memory",                   "B",   "",                "measurement", "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "wifiRSSI",        "Wi-Fi RSSI",        "wifi",                     "dBm", "signal_strength", "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "CPUtemp",         "CPU Temperature",   "thermometer",              "°C",  "temperature",     "measurement", "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "interval",        "Interval",          "clock-time-eight-outline", "min",  ""           ,    "measurement", "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "IP",              "IP",                "network-outline",           "",    "",               "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic("",     "status",          "Status",            "list-status",               "",    "",               "",            "diagnostic");



    for (int i = 0; i < (*NUMBERS).size(); ++i) {
        std::string group = (*NUMBERS)[i]->name;
        if (group == "default") {
            group = "";
        }

    //                                  Group | Field                 | User Friendly Name                | Icon                   | Unit     | Device Class | State Class       | Entity Category
        sendHomeAssistantDiscoveryTopic(group,   "value",              "Value",                            "gauge",                 valueUnit, meterType,     "total_increasing", "");
        sendHomeAssistantDiscoveryTopic(group,   "raw",                "Raw Value",                        "raw",                   valueUnit, "",            "total_increasing", "diagnostic");
        sendHomeAssistantDiscoveryTopic(group,   "error",              "Error",                            "alert-circle-outline",  "",        "",            "",                 "diagnostic");
        /* Not announcing "rate" as it is better to use rate_per_time_unit resp. rate_per_digitalization_round */
        // sendHomeAssistantDiscoveryTopic(group,   "rate",               "Rate (Unit/Minute)",               "swap-vertical",         "",        "",            "",                 ""); // Legacy, always Unit per Minute
        sendHomeAssistantDiscoveryTopic(group,   "rate_per_time_unit", "Rate (" + rateUnit + ")",          "swap-vertical",         rateUnit,  "",            "",                 "");        
        sendHomeAssistantDiscoveryTopic(group,   "rate_per_digitalization_round",  "Change since last digitalization round", "arrow-expand-vertical", valueUnit, "",            "measurement",      ""); // correctly the Unit is Uint/Interval!
        sendHomeAssistantDiscoveryTopic(group,   "timestamp",          "Timestamp",                  "clock-time-eight-outline", "",        "timestamp",   "",                "diagnostic");
        sendHomeAssistantDiscoveryTopic(group,   "json",               "JSON",                       "code-json",                "",        "",            "",                 "diagnostic");
        sendHomeAssistantDiscoveryTopic(group,   "problem",            "Problem",                    "alert-outline",            "",        "problem",            "",                 ""); // Special binary sensor which is based on error topic
    }
}

void publishSystemData() {
    if (!getMQTTisConnected()) 
        return;

    char tmp_char[50];

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Publishing system MQTT topics...");

    sprintf(tmp_char, "%ld", (long)getUpTime());
    MQTTPublish(maintopic + "/" + "uptime", std::string(tmp_char), retainFlag);
    
    sprintf(tmp_char, "%lu", (long) getESPHeapSize());
    MQTTPublish(maintopic + "/" + "freeMem", std::string(tmp_char), retainFlag);

    sprintf(tmp_char, "%d", get_WIFI_RSSI());
    MQTTPublish(maintopic + "/" + "wifiRSSI", std::string(tmp_char), retainFlag);

    sprintf(tmp_char, "%d", (int)temperatureRead());
    MQTTPublish(maintopic + "/" + "CPUtemp", std::string(tmp_char), retainFlag);
}


void publishStaticData() {
    if (!getMQTTisConnected()) 
        return;

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Publishing static MQTT topics...");
    MQTTPublish(maintopic + "/" + "MAC", getMac(), retainFlag);
    MQTTPublish(maintopic + "/" + "IP", *getIPAddress(), retainFlag);
    MQTTPublish(maintopic + "/" + "hostname", hostname, retainFlag);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << roundInterval; // minutes
    MQTTPublish(maintopic + "/" + "interval", stream.str(), retainFlag);
}

esp_err_t sendDiscovery_and_static_Topics(httpd_req_t *req) {
    if (HomeassistantDiscovery) {
        MQTThomeassistantDiscovery();
    }

    publishStaticData();

    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));  

    return ESP_OK;
}

void GotConnected(std::string maintopic, int retainFlag) {
    if (HomeassistantDiscovery) {
        MQTThomeassistantDiscovery();
    }

    publishStaticData();
    publishSystemData();
}

void register_server_mqtt_uri(httpd_handle_t server) {
    httpd_uri_t uri = { };
    uri.method    = HTTP_GET;

    uri.uri       = "/mqtt_publish_discovery";
    uri.handler   = sendDiscovery_and_static_Topics;
    uri.user_ctx  = (void*) "MQTT Discovery and Static Topics sent";    
    httpd_register_uri_handler(server, &uri); 
}


std::string getTimeUnit(void) {
    return timeUnit;
}


void SetHomeassistantDiscoveryEnabled(bool enabled) {
    HomeassistantDiscovery = enabled;
}


void setMqtt_Server_Retain(int _retainFlag) {
    retainFlag = _retainFlag;
}

void mqttServer_setMainTopic( std::string _maintopic) {
    maintopic = _maintopic;
}

std::string mqttServer_getMainTopic() {
    return maintopic;
}

#endif //ENABLE_MQTT
