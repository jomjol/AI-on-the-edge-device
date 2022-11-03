#include <sstream>
#include <iomanip>
#include "ClassFlowMQTT.h"
#include "Helper.h"
#include "connect_wlan.h"
#include "ClassLogFile.h"

#include "time_sntp.h"
#include "interface_mqtt.h" // move
#include "ClassFlowPostProcessing.h"
#include "ClassFlowControll.h"

#include "server_mqtt.h"

#include <time.h>


#define __HIDE_PASSWORD

static const char *TAG = "class_flow_MQTT";

extern float AutoIntervalShared;

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
    
    keepAlive = AutoIntervalShared * 60 * 2.5; // TODO find better way to access AutoIntervall in ClassFlowControll

    mqttServer_Init(flowpostprocessing->GetNumbers(), keepAlive);

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
            if (toUpper(zerlegt[1]) == "TRUE") {
                SetRetainFlag = 1;  
                setMqtt_Server_Retain(SetRetainFlag);
            }
        }
        if ((toUpper(zerlegt[0]) == "HOMEASSISTANTDISCOVERY") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SetHomeassistantDiscoveryEnabled(true);  
        }
        if ((toUpper(zerlegt[0]) == "METERTYPE") && (zerlegt.size() > 1)) {
        /* Use meter type for the device class 
           Make sure it is a listed one on https://developers.home-assistant.io/docs/core/entity/sensor/#available-device-classes */
            if (toUpper(zerlegt[1]) == "WATER_M3") {
                mqttServer_setMeterType("water", "m³", "h", "m³/h");
            }
            else if (toUpper(zerlegt[1]) == "WATER_L") {
                mqttServer_setMeterType("water", "L", "h", "L/h");
            }
            else if (toUpper(zerlegt[1]) == "WATER_FT3") {
                mqttServer_setMeterType("water", "ft³", "m", "ft³/m"); // Minutes
            }
            else if (toUpper(zerlegt[1]) == "WATER_GAL") {
                mqttServer_setMeterType("water", "gal", "h", "gal/h");
            }
            else if (toUpper(zerlegt[1]) == "GAS_M3") {
                mqttServer_setMeterType("gas", "m³", "h", "m³/h");
            }
            else if (toUpper(zerlegt[1]) == "GAS_FT3") {
                mqttServer_setMeterType("gas", "ft³", "m", "ft³/m"); // Minutes
            }
            else if (toUpper(zerlegt[1]) == "ENERGY_WH") {
                mqttServer_setMeterType("energy", "Wh", "h", "W");
            }
            else if (toUpper(zerlegt[1]) == "ENERGY_KWH") {
                mqttServer_setMeterType("energy", "kWh", "h", "kW");
            }
            else if (toUpper(zerlegt[1]) == "ENERGY_MWH") {
                mqttServer_setMeterType("energy", "MWh", "h", "MW");
            }
        }

        if ((toUpper(zerlegt[0]) == "CLIENTID") && (zerlegt.size() > 1))
        {
            this->clientname = zerlegt[1];
        }

        if (((toUpper(zerlegt[0]) == "TOPIC") || (toUpper(zerlegt[0]) == "MAINTOPIC")) && (zerlegt.size() > 1))
        {
            maintopic = zerlegt[1];
            mqttServer_setMainTopic(maintopic);
        }
    }

    MQTT_Configure(uri, clientname, user, password, maintopic, LWT_TOPIC, LWT_CONNECTED, LWT_DISCONNECTED,
            keepAlive, SetRetainFlag, (void *)&GotConnected);

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
    std::string resultRatePerTimeUnit = ""; // According to selection
    std::string resulttimestamp = "";
    std::string resultchangabs = "";
    string zw = "";
    string namenumber = "";

    publishRuntimeData();

    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue; // Unit per minutes
            resultchangabs = (*NUMBERS)[i]->ReturnChangeAbsolute; // Units per round
            resulttimestamp = (*NUMBERS)[i]->timeStamp;

            namenumber = (*NUMBERS)[i]->name;
            if (namenumber == "default")
                namenumber = maintopic + "/";
            else
                namenumber = maintopic + "/" + namenumber + "/";

            LogFile.WriteToFile(ESP_LOG_INFO, "Publishing MQTT topics...");

            if (result.length() > 0)   
                MQTTPublish(namenumber + "value", result, SetRetainFlag);

            if (resulterror.length() > 0)  
                MQTTPublish(namenumber + "error", resulterror, SetRetainFlag);

            if (resultrate.length() > 0) {
                MQTTPublish(namenumber + "rate", resultrate, SetRetainFlag);
                
                std::string resultRatePerTimeUnit;
                if (getTimeUnit() == "h") { // Need conversion to be per hour
                    resultRatePerTimeUnit = resultRatePerTimeUnit = to_string((*NUMBERS)[i]->FlowRateAct / 60); // per minutes => per hour
                }
                else { // Keep per minute
                    resultRatePerTimeUnit = resultrate;
                }
                MQTTPublish(namenumber + "rate_per_time_unit", resultRatePerTimeUnit, SetRetainFlag);
            }

            if (resultchangabs.length() > 0) {
                MQTTPublish(namenumber + "changeabsolut", resultchangabs, SetRetainFlag); // Legacy API
                MQTTPublish(namenumber + "rate_per_digitalization_round", resultchangabs, SetRetainFlag);
            }

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
