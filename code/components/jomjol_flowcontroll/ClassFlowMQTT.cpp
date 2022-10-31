#include <sstream>
#include "ClassFlowMQTT.h"
#include "Helper.h"
#include "connect_wlan.h"
#include "ClassLogFile.h"

#include "time_sntp.h"
#include "interface_mqtt.h"
#include "ClassFlowPostProcessing.h"
#include "ClassFlowControll.h"

#include <time.h>

#define __HIDE_PASSWORD

static const char *TAG = "class_flow_MQTT";

#define LWT_TOPIC        "connection"
#define LWT_CONNECTED    "connected"
#define LWT_DISCONNECTED "connection lost"

extern const char* libfive_git_version(void);
extern const char* libfive_git_revision(void);
extern const char* libfive_git_branch(void);

extern float AutoIntervalShared;

std::vector<NumberPost*>* NUMBERS;
bool HomeassistantDiscovery = false;
std::string meterType = "";
std::string valueUnit = "";
std::string timeUnit = "";
std::string rateUnit = "Unit/Minute";

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

    if (group != "") { // Prepend the group to the name
        name = group + " " + name;
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
     /*   if (deviceClass == "problem") {
            payload += "\"value_template\": \"{{ 'OFF' if 'no error' in value else 'ON'}}\"," + nl;
        }*/
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
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "interval",        "Interval",          "clock-time-eight-outline", "min",  "temperature",    "measurement", "diagnostic");
    sendHomeAssistantDiscoveryTopic(maintopic, "",     "IP",              "IP",                "network-outline",           "",    "",               "",            "diagnostic");

    for (int i = 0; i < (*NUMBERS).size(); ++i) {
  //      xxx needs splitting. only the name can have no group!
        std::string group = "";
     /*   if ((*NUMBERS).size() > 1) { // There is more than one meter, use the NUMBER name as group
            group = (*NUMBERS)[i]->name;
        }*/
         group = (*NUMBERS)[i]->name;
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
        sendHomeAssistantDiscoveryTopic(maintopic, group,   "json",               "JSON",                       "code-json",                "",        "",            "",                 "");
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

void GotConnected(std::string maintopic, int SetRetainFlag) {
    if (HomeassistantDiscovery) {
        MQTThomeassistantDiscovery(maintopic);
    }

    MQTTPublish(maintopic + "/" + "MAC", getMac(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "IP", *getIPAddress(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "hostname", hostname, SetRetainFlag);
    MQTTPublish(maintopic + "/" + "interval", std::to_string(AutoIntervalShared), SetRetainFlag);

    publishRuntimeData(maintopic, SetRetainFlag);
}

void ClassFlowMQTT::SetInitialParameter(void)
{
    uri = "";
    topic = "";
    topicError = "";
    topicRate = "";
    topicTimeStamp = "";
    maintopic = hostname;

    topicUptime = "";
    topicFreeMem = "";

    clientname = "AIOTED-" + getMac();

    OldValue = "";
    flowpostprocessing = NULL;  
    user = "";
    password = ""; 
    SetRetainFlag = 0;
    previousElement = NULL;
    ListFlowControll = NULL; 
    disabled = false;
    keepAlive = 25*60;
}       

ClassFlowMQTT::ClassFlowMQTT()
{
    SetInitialParameter();
}

ClassFlowMQTT::ClassFlowMQTT(std::vector<ClassFlow*>* lfc)
{
    SetInitialParameter();

    ListFlowControll = lfc;
    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }
    }

    NUMBERS = flowpostprocessing->GetNumbers();
    keepAlive = AutoIntervalShared * 60 * 2.5; // TODO find better way to access AutoIntervall in ClassFlowControll

    LogFile.WriteToFile(ESP_LOG_INFO, "Digitizer interval is " + std::to_string(AutoIntervalShared) + 
            " minutes => setting MQTT LWT timeout to " + std::to_string(keepAlive/60) + " minutes.");
}

ClassFlowMQTT::ClassFlowMQTT(std::vector<ClassFlow*>* lfc, ClassFlow *_prev)
{
    SetInitialParameter();

    previousElement = _prev;
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }
    }
}


bool ClassFlowMQTT::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[MQTT]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((toUpper(zerlegt[0]) == "USER") && (zerlegt.size() > 1))
        {
            this->user = zerlegt[1];
        }  
        if ((toUpper(zerlegt[0]) == "PASSWORD") && (zerlegt.size() > 1))
        {
            this->password = zerlegt[1];
        }               
        if ((toUpper(zerlegt[0]) == "URI") && (zerlegt.size() > 1))
        {
            this->uri = zerlegt[1];
        }
        if ((toUpper(zerlegt[0]) == "SETRETAINFLAG") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SetRetainFlag = 1;  
        }
        if ((toUpper(zerlegt[0]) == "HOMEASSISTANTDISCOVERY") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                HomeassistantDiscovery = true;  
        }
        if ((toUpper(zerlegt[0]) == "METERTYPE") && (zerlegt.size() > 1)) {
        /* Use meter type for the device class 
           Make sure it is a listed one on https://developers.home-assistant.io/docs/core/entity/sensor/#available-device-classes */
            if (toUpper(zerlegt[1]) == "WATER_M3") {
                meterType = "water";
                valueUnit = "m³";
                timeUnit = "h"; // Hour
                rateUnit = "m³/h";
            }
            else if (toUpper(zerlegt[1]) == "WATER_L") {
                meterType = "water";
                valueUnit = "L";
                timeUnit = "h"; // Hour
                rateUnit = "L/h";
            }
            else if (toUpper(zerlegt[1]) == "WATER_FT3") {
                meterType = "water";
                valueUnit = "ft³";
                timeUnit = "m"; // Minutes
                rateUnit = "ft³/m";
            }
            else if (toUpper(zerlegt[1]) == "WATER_GAL") {
                meterType = "water";
                valueUnit = "gal";
                timeUnit = "h"; // Hour
                rateUnit = "gal/h";
            }
            else if (toUpper(zerlegt[1]) == "GAS_M3") {
                meterType = "gas";
                valueUnit = "m³";
                timeUnit = "h"; // Hour
                rateUnit = "m³/h";
            }
            else if (toUpper(zerlegt[1]) == "GAS_FT3") {
                meterType = "gas";
                valueUnit = "ft³";
                timeUnit = "m"; // Minutes
                rateUnit = "ft³/m";
            }
            else if (toUpper(zerlegt[1]) == "ENERGY_W") {
                meterType = "energy";
                valueUnit = "Wh";
                timeUnit = "h"; // Hour
                rateUnit = "W";
            }
            else if (toUpper(zerlegt[1]) == "ENERGY_KW") {
                meterType = "energy";
                valueUnit = "kWh";
                timeUnit = "h"; // Hour
                rateUnit = "kW";
            }
            else if (toUpper(zerlegt[1]) == "ENERGY_MW") {
                meterType = "energy";
                valueUnit = "MWh";
                timeUnit = "h"; // Hour
                rateUnit = "MW";
            }
        }

        if ((toUpper(zerlegt[0]) == "CLIENTID") && (zerlegt.size() > 1))
        {
            this->clientname = zerlegt[1];
        }

        if (((toUpper(zerlegt[0]) == "TOPIC") || (toUpper(zerlegt[0]) == "MAINTOPIC")) && (zerlegt.size() > 1))
        {
            maintopic = zerlegt[1];
        }
    }

    MQTT_Configure(uri, clientname, user, password, maintopic, LWT_TOPIC, LWT_CONNECTED, LWT_DISCONNECTED, keepAlive, SetRetainFlag, (void *)&GotConnected);

    if (!MQTT_Init()) {
        if (!MQTT_Init()) { // Retry
            return false;
        }
    }
    return true;
}


string ClassFlowMQTT::GetMQTTMainTopic()
{
    return maintopic;
}


bool ClassFlowMQTT::doFlow(string zwtime)
{
    std::string result;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = ""; // Always Unit / Minute
    std::string resultrate2 = ""; // According to selection
    std::string resulttimestamp = "";
    std::string resultchangabs = "";
    string zw = "";
    string namenumber = "";

    publishRuntimeData(maintopic, SetRetainFlag);

    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue; // Unit per minutes
            resultchangabs = (*NUMBERS)[i]->ReturnChangeAbsolute;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;

            namenumber = (*NUMBERS)[i]->name;
            if (namenumber == "default")
                namenumber = maintopic + "/";
            else
                namenumber = maintopic + "/" + namenumber + "/";

            if (result.length() > 0)   
                MQTTPublish(namenumber + "value", result, SetRetainFlag);

            if (resulterror.length() > 0)  
                MQTTPublish(namenumber + "error", resulterror, SetRetainFlag);

            if (resultrate.length() > 0) {
                MQTTPublish(namenumber + "rate", resultrate, SetRetainFlag);
                
                std::string resultrate2;
                if (timeUnit == "h") { // Need conversion to be per hour
                    resultrate2 = resultrate2 = to_string((*NUMBERS)[i]->FlowRateAct / 60); // per minutes => per hour
                }
                else { // Keep per minute
                    resultrate2 = resultrate;
                }
                MQTTPublish(namenumber + "rate2", resultrate2, SetRetainFlag);
            }

            if (resultchangabs.length() > 0)   
                MQTTPublish(namenumber + "rate_per_interval", resultchangabs, SetRetainFlag);

            if (resultraw.length() > 0)   
                MQTTPublish(namenumber + "raw", resultraw, SetRetainFlag);

            if (resulttimestamp.length() > 0)
                MQTTPublish(namenumber + "timestamp", resulttimestamp, SetRetainFlag);

            std::string json = "";
            
            if (result.length() > 0)
                json += "{\"value\":"+result;
            else
                json += "{\"value\":\"\"";

            json += ",\"raw\":\""+resultraw;
            json += "\",\"error\":\""+resulterror;

            if (resultrate.length() > 0)
                json += "\",\"rate\":"+resultrate;
            else
                json += "\",\"rate\":\"\"";

            json += ",\"timestamp\":\""+resulttimestamp+"\"}";

            MQTTPublish(namenumber + "json", json, SetRetainFlag);
        }
    }
    else
    {
        for (int i = 0; i < ListFlowControll->size(); ++i)
        {
            zw = (*ListFlowControll)[i]->getReadout();
            if (zw.length() > 0)
            {
                if (result.length() == 0)
                    result = zw;
                else
                    result = result + "\t" + zw;
            }
        }
        MQTTPublish(topic, result, SetRetainFlag);
    }
    
    OldValue = result;
    
    return true;
}
