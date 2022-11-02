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
int interval = 0;



void mqttServer_Init(std::vector<NumberPost*>* _NUMBERS, int _interval) {
    NUMBERS = _NUMBERS;
    interval = _interval;
}

void mqttServer_setMeterType(std::string _meterType, std::string _valueUnit, std::string _timeUnit,std::string _rateUnit) {
    meterType = _meterType;
    valueUnit = _valueUnit;
    timeUnit = _timeUnit;
    rateUnit = _rateUnit;
}

void sendHomeAssistantDiscoveryTopic(std::string maintopic, std::string group, std::string field,
    std::string name, std::string icon, std::string unit, std::string deviceClass, std::string stateClass, std::string entityCategory) {
    std::string version = std::string(libfive_git_version());

    if (version == "") {
        version = std::string(libfive_git_branch()) + " (" + std::string(libfive_git_revision()) + ")";
    }
    
    std::string topic;
    std::string topicFull;
    std::string topicT;
    std::string payload;
    std::string nl = "\n";

    if (group == "") {
        topic =  field;
        topicT = field;
    }
    else {
        topic = group + "/" + field;
        topicT = group + "_" + field;
    }

    if ((*NUMBERS).size() > 1) { // There is more than one meter, prepend the group so we can differentiate them
        if (group != "") { // But only if the group is set
            name = group + " " + name;
        }
    }

    topicFull = "homeassistant/sensor/" + maintopic + "/" + topicT + "/config";

    /* See https://www.home-assistant.io/docs/mqtt/discovery/ */
    payload = "{" + nl +
        "\"~\": \"" + maintopic + "\"," + nl +
        "\"unique_id\": \"" + maintopic + "-" + topicT + "\"," + nl +
        "\"object_id\": \"" + maintopic + "_" + topicT + "\"," + nl + // This used to generate the Entity ID
        "\"name\": \"" + name + "\"," + nl +
        "\"icon\": \"mdi:" + icon + "\"," + nl;        

    if (group != "") {
        if (field == "problem") { // Special binary sensor which is based on error topic
            payload += "\"state_topic\": \"~/" + group + "/error\"," + nl;
            payload += "\"value_template\": \"{{ 'OFF' if 'no error' in value else 'ON'}}\"," + nl;
        }
        else {
            payload += "\"state_topic\": \"~/" + group + "/" + field + "\"," + nl;
        }
    }
    else {
            payload += "\"state_topic\": \"~/" + field + "\"," + nl;
    }

    if (unit != "") {
        payload += "\"unit_of_meas\": \"" + unit + "\"," + nl;
    }

    if (deviceClass != "") {
        payload += "\"device_class\": \"" + deviceClass + "\"," + nl;
    }

    if (stateClass != "") {
        payload += "\"state_class\": \"" + stateClass + "\"," + nl;
    } 

    if (entityCategory != "") {
        payload += "\"entity_category\": \"" + entityCategory + "\"," + nl;
    } 

    payload += 
        "\"availability_topic\": \"~/" + std::string(LWT_TOPIC) + "\"," + nl +
        "\"payload_available\": \"" + LWT_CONNECTED + "\"," + nl +
        "\"payload_not_available\": \"" + LWT_DISCONNECTED + "\"," + nl;

    payload +=
    "\"device\": {" + nl +
        "\"identifiers\": [\"" + maintopic + "\"]," + nl +
        "\"name\": \"" + maintopic + "\"," + nl +
        "\"model\": \"Meter Digitizer\"," + nl +
        "\"manufacturer\": \"AI on the Edge Device\"," + nl +
      "\"sw_version\": \"" + version + "\"," + nl +
      "\"configuration_url\": \"http://" + *getIPAddress() + "\"" + nl +
    "}" + nl +
    "}" + nl;

    MQTTPublish(topicFull, payload, true);
}

void MQTThomeassistantDiscovery(std::string maintopic) {
    
    LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - Sending Homeassistant Discovery Topics (Meter Type: " + meterType + ", Value Unit: " + valueUnit + " , Rate Unit: " + rateUnit + ")...");

    //                              Maintopic | Group | Field            | User Friendly Name | Icon                      | Unit | Device Class     | State Class  | Entity Category
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "uptime",          "Uptime",            "clock-time-eight-outline", "s",   "",                "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "MAC",             "MAC Address",       "network-outline",          "",    "",                "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "hostname",        "Hostname",          "network-outline",          "",    "",                "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "free_memory",     "Free Memory",       "memory",                   "B",   "",                "measurement", "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "wifi_RSSI",       "Wi-Fi RSSI",        "wifi",                     "dBm", "signal_strength", "",            "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "CPU_temperature", "CPU Temperature",   "thermometer",              "°C",  "temperature",     "measurement", "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "interval",        "Interval",          "clock-time-eight-outline", "min",  ""           ,    "measurement", "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "IP",              "IP",                "network-outline",           "",    "",               "",            "diagnostic");

    for (int i = 0; i < (*NUMBERS).size(); ++i) {
         std::string group = (*NUMBERS)[i]->name;
    //                                  Maintopic | Group | Field              | User Friendly Name          | Icon                      | Unit     | Device Class | State Class       | Entity Category
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "value",             "Value",                      "gauge",                    valueUnit, meterType,       "total_increasing", "");
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "raw",               "Raw Value",                  "raw",                      valueUnit, "",              "total_increasing", "diagnostic");
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "error",             "Error",                      "alert-circle-outline",     "",        "",              "",                 "diagnostic");
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "rate",              "Rate (Unit/Minute)",         "swap-vertical",            "",        "",              "",                 ""); // Legacy, always Unit per Minute
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "rate2",             "Rate (" + rateUnit + ")",    "swap-vertical",            rateUnit,  "",              "",                "");        
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "rate_per_interval", "Change since last Interval", "arrow-expand-vertical",    valueUnit, "",              "measurement",      ""); // correctly the Unit is Uint/Interval!
        /* The timestamp string misses the Timezone, see PREVALUE_TIME_FORMAT_OUTPUT!
           We need to know the timezone and append it! Until we do this, we simply
           do not set the device class to "timestamp" to avoid errors in Homeassistant! */
        // sendHomeAssistantDiscoveryTopic(maintopic, group,   "timestamp",       "Timestamp",                  "clock-time-eight-outline", "",        "timestamp",   "",                 "diagnostic");
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "timestamp",          "Timestamp",                  "clock-time-eight-outline", "",        "",            "",                 "diagnostic");
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "json",               "JSON",                       "code-json",                "",        "",            "",                 "diagnostic");
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "problem",            "Problem",                    "alert-outline",            "",        "",            "",                 ""); // Special binary sensor which is based on error topic
    }
}

void publishRuntimeData(std::string maintopic, int SetRetainFlag) {
    char tmp_char[50];

    sprintf(tmp_char, "%ld", (long)getUpTime());
    MQTTPublish(maintopic + "/" + "uptime", std::string(tmp_char), SetRetainFlag);
    
    sprintf(tmp_char, "%zu", esp_get_free_heap_size());
    MQTTPublish(maintopic + "/" + "free_memory", std::string(tmp_char), SetRetainFlag);

    sprintf(tmp_char, "%d", get_WIFI_RSSI());
    MQTTPublish(maintopic + "/" + "wifi_RSSI", std::string(tmp_char), SetRetainFlag);

    sprintf(tmp_char, "%d", (int)temperatureRead());
    MQTTPublish(maintopic + "/" + "CPU_temperature", std::string(tmp_char), SetRetainFlag);
}


void publishStaticData(std::string maintopic, int SetRetainFlag) {
    MQTTPublish(maintopic + "/" + "MAC", getMac(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "IP", *getIPAddress(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "hostname", hostname, SetRetainFlag);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << interval;
    MQTTPublish(maintopic + "/" + "interval", stream.str(), SetRetainFlag);
}


void GotConnected(std::string maintopic, int SetRetainFlag) {
    if (HomeassistantDiscovery) {
        MQTThomeassistantDiscovery(maintopic);
    }

    publishStaticData(maintopic, SetRetainFlag);
    publishRuntimeData(maintopic, SetRetainFlag);
}


void register_server_mqtt_uri(httpd_handle_t server) {

 /*   httpd_uri_t uri = { };
    uri.method    = HTTP_GET;

    uri.uri       = "/mqtt_send";
    uri.handler   = handler_lightOn;
    uri.user_ctx  = (void*) "MQTT Send Static";    
    httpd_register_uri_handler(server, &uri); */
}


std::string getTimeUnit(void) {
    return timeUnit;
}


void SetHomeassistantDiscoveryEnabled(bool enabled) {
    HomeassistantDiscovery = enabled;
}