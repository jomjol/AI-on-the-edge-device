#include <sstream>
#include "ClassFlowMQTT.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_mqtt.h"
#include "ClassFlowPostProcessing.h"
#include "ClassFlowPostProcessing.h"
#include "ClassFlowControll.h"

#include <time.h>

#define __HIDE_PASSWORD

static const char *TAG = "class_flow_MQTT";

void ClassFlowMQTT::SetInitialParameter(void)
{
    uri = "";
    topic = "";
    topicError = "";
    topicRate = "";
    topicTimeStamp = "";
    maintopic = "";

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

        if (((*ListFlowControll)[i])->name().compare("ClassFlowControll") == 0)
        {
            ClassFlowControll *cfc = (ClassFlowControll*) (*ListFlowControll)[i];
            keepAlive = cfc->getAutoInterval()* 2.5; // Allow at least than 2 failed rounds before we are threated as disconnected
        }
    }
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
        else { // Main topic not set, use the hostname
            maintopic = hostname;
        }
    }
    
    MQTT_Configure(uri, clientname, user, password, maintopic + "/connection", keepAlive);

    ESP_LOGD(TAG, "MQTT maintopic: %s", maintopic.c_str());

    if (!MQTT_Init()) {
        if (!MQTT_Init()) { // Retry
            return false;
        }
    }

    MQTTPublish(maintopic + "/" + "mac", getMac(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "ip", *getIPAddress(), SetRetainFlag);
    MQTTPublish(maintopic + "/" + "hostname", hostname, SetRetainFlag);

    publishRuntimeData();

    return true;
}


string ClassFlowMQTT::GetMQTTMainTopic()
{
    return maintopic;
}

void ClassFlowMQTT::publishRuntimeData() {
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

    publishRuntimeData();

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
