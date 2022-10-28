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

std::vector<NumberPost*>* NUMBERS;

enum TopicType {
    TOPIC_TYPE_NORMAL,
    TOPIC_TYPE_ERROR,
    TOPIC_TYPE_SIGNAL_STRENGTH,
    TOPIC_TYPE_TEMPERATURE,
    TOPIC_TYPE_TIMESTAMP
};

void sendHomeAssistantDiscoveryTopic(std::string maintopic, std::string group, std::string field,
    std::string userFriendlyName, std::string icon, std::string unit, TopicType topicType) {
    std::string version = std::string(libfive_git_version());

    if (version == "") {
        version = std::string(libfive_git_branch()) + " (" + std::string(libfive_git_revision()) + ")";
    }
    
    std::string topic;
    std::string topicFull;
    std::string topicT;
    std::string payload;
    std::string nl = "\n";
    std::string name;

    if (group != "") {
        topic = group + "/" + field;
        topicT = group + "_" + field;
    }
    else {
        topic =  field;
        topicT = field;
    }

    name = field;
    if (group != "") {
        name = group + " " + name;
        userFriendlyName = group + " " + userFriendlyName;
    }

    topicFull = "homeassistant/sensor/" + maintopic + "/" + topicT + "/config";

    /* See https://www.home-assistant.io/docs/mqtt/discovery/ */
    payload = "{" + nl +
        "\"~\": \"" + maintopic + "\"," + nl +
        "\"unique_id\": \"" + maintopic + "-" +topicT + "\"," + nl +
        "\"name\": \"" + userFriendlyName + "\"," + nl +
        "\"icon\": \"mdi:" + icon + "\"," + nl;        

    if (group != "") {
        payload += "\"state_topic\": \"~/" + group + "/" + field + "\"," + nl;
    }
    else {
        payload += "\"state_topic\": \"~/" + field + "\"," + nl;
    }

    if (unit != "-") {
        payload += "\"unit_of_meas\": \"" + unit + "\"," + nl;
    }

    if (topicType == TOPIC_TYPE_ERROR) {
        payload += std::string("") + 
            "\"value_template\": \"{{ 'OFF' if 'no error' in value else 'ON'}}\"," + nl + 
            "\"device_class\": \"problem\"," + nl;
    }
    else if (topicType == TOPIC_TYPE_SIGNAL_STRENGTH) {
        payload += std::string("") + 
            "\"device_class\": \"signal_strength\"," + nl;
    }
    else if (topicType == TOPIC_TYPE_TEMPERATURE) {
        payload += std::string("") + 
            "\"device_class\": \"temperature\"," + nl;
    }
    else if (topicType == TOPIC_TYPE_TIMESTAMP) {
        payload += std::string("") + 
            "\"device_class\": \"timestamp\"," + nl;
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
    LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - Sending Homeassistant Discovery Topics...");
    //                              maintopic  group  field        User Friendly Name    icon                        unit   Topic Type
    sendHomeAssistantDiscoveryTopic(maintopic, "", "uptime",          "Uptime",          "clock-time-eight-outline", "s",   TOPIC_TYPE_NORMAL);
    sendHomeAssistantDiscoveryTopic(maintopic, "", "IP",              "IP",              "network-outline",          "-",   TOPIC_TYPE_NORMAL);
    sendHomeAssistantDiscoveryTopic(maintopic, "", "MAC",             "MAC Address",     "network-outline",          "-",   TOPIC_TYPE_NORMAL);
    sendHomeAssistantDiscoveryTopic(maintopic, "", "hostname",        "Hostname",        "network-outline",          "-",   TOPIC_TYPE_NORMAL);
    sendHomeAssistantDiscoveryTopic(maintopic, "", "FreeMem",         "Free Memory",     "memory",                   "B",   TOPIC_TYPE_NORMAL);
    sendHomeAssistantDiscoveryTopic(maintopic, "", "wifiRSSI",        "Wi-Fi RSSI",      "wifi",                     "dBm", TOPIC_TYPE_SIGNAL_STRENGTH);
    sendHomeAssistantDiscoveryTopic(maintopic, "", "CPUtemp",         "CPU Temperature", "thermometer",              "°C",  TOPIC_TYPE_TEMPERATURE);

    for (int i = 0; i < (*NUMBERS).size(); ++i) {
    //                                  maintopic  group                field           User Friendly Name  icon                        unit  Topic Type
        sendHomeAssistantDiscoveryTopic(maintopic, (*NUMBERS)[i]->name, "value",         "Value",           "gauge",                    "",   TOPIC_TYPE_NORMAL);
        sendHomeAssistantDiscoveryTopic(maintopic, (*NUMBERS)[i]->name, "error",         "Error",           "alert-circle-outline",     "-",  TOPIC_TYPE_ERROR);
        sendHomeAssistantDiscoveryTopic(maintopic, (*NUMBERS)[i]->name, "rate",          "Rate",            "swap-vertical",            "",   TOPIC_TYPE_NORMAL);
        sendHomeAssistantDiscoveryTopic(maintopic, (*NUMBERS)[i]->name, "changeabsolut", "Absolute Change", "arrow-expand-vertical",    "",   TOPIC_TYPE_NORMAL);
        sendHomeAssistantDiscoveryTopic(maintopic, (*NUMBERS)[i]->name, "raw",           "Raw Value",       "raw",                      "",   TOPIC_TYPE_NORMAL);
        sendHomeAssistantDiscoveryTopic(maintopic, (*NUMBERS)[i]->name, "timestamp",     "Timestamp",       "clock-time-eight-outline", "-",  TOPIC_TYPE_TIMESTAMP);
        sendHomeAssistantDiscoveryTopic(maintopic, (*NUMBERS)[i]->name, "json",          "JSON",            "code-json",                "-",  TOPIC_TYPE_NORMAL);
    }
}

void publishRuntimeData(std::string maintopic, int SetRetainFlag) {
    char tmp_char[50];

    sprintf(tmp_char, "%ld", (long)getUpTime());
    MQTTPublish(maintopic + "/" + "uptime", std::string(tmp_char), SetRetainFlag);
    
    sprintf(tmp_char, "%zu", esp_get_free_heap_size());
    MQTTPublish(maintopic + "/" + "freeMem", std::string(tmp_char), SetRetainFlag);

    sprintf(tmp_char, "%d", get_WIFI_RSSI());
    MQTTPublish(maintopic + "/" + "wifiRSSI", std::string(tmp_char), SetRetainFlag);

    sprintf(tmp_char, "%d", (int)temperatureRead());
    MQTTPublish(maintopic + "/" + "CPUtemp", std::string(tmp_char), SetRetainFlag);
}

void GotConnected(std::string maintopic, int SetRetainFlag) {
    MQTThomeassistantDiscovery(maintopic);

    MQTTPublish(maintopic + "/" + "MAC", getMac(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "IP", *getIPAddress(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "hostname", hostname, SetRetainFlag);

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
      //  ESP_LOGW(TAG, "LCF: %s", ((*ListFlowControll)[i])->name().c_str());

        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }

// TODO this does not work since ClassFlowControll is not in the list!
      /*  if (((*ListFlowControll)[i])->name().compare("ClassFlowControll") == 0)
        {
            ClassFlowControll *cfc = (ClassFlowControll*) (*ListFlowControll)[i];
            this->keepAlive = cfc->getAutoInterval()* 2.5; // Allow at least than 2 failed rounds before we are threated as disconnected
            ESP_LOGW(TAG, "KEEPALIVE: %d", this->keepAlive);       
        }*/
    }

    NUMBERS = flowpostprocessing->GetNumbers();
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
    std::string resultrate = "";
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
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
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

            if (resultrate.length() > 0)   
                MQTTPublish(namenumber + "rate", resultrate, SetRetainFlag);

            if (resultchangabs.length() > 0)   
                MQTTPublish(namenumber + "changeabsolut", resultchangabs, SetRetainFlag);

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
