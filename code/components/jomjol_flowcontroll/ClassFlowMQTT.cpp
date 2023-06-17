#ifdef ENABLE_MQTT

#include <sstream>
#include <iomanip>
#include "ClassFlowMQTT.h"
#include "Helper.h"
#include "connect_wlan.h"
#include "read_wlanini.h"
#include "ClassLogFile.h"

#include "time_sntp.h"
#include "interface_mqtt.h"
#include "ClassFlowPostProcessing.h"
#include "ClassFlowControll.h"

#include "server_mqtt.h"

#include <time.h>
#include "../../include/defines.h"

static const char *TAG = "MQTT";

extern const char* libfive_git_version(void);
extern const char* libfive_git_revision(void);
extern const char* libfive_git_branch(void);


void ClassFlowMQTT::SetInitialParameter(void)
{
    uri = "";
    topic = "";
    topicError = "";
    topicRate = "";
    topicTimeStamp = "";
    maintopic = wlan_config.hostname;

    topicUptime = "";
    topicFreeMem = "";

    caCertFilename = "";
    clientCertFilename = "";
    clientKeyFilename = "";
    clientname = wlan_config.hostname;

    OldValue = "";
    flowpostprocessing = NULL;  
    user = "";
    password = ""; 
    SetRetainFlag = false;
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
    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[MQTT]") != 0)       // Paragraph does not fit MQTT
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);
        if ((toUpper(splitted[0]) == "CACERT") && (splitted.size() > 1))
        {
            this->caCertFilename = splitted[1];
        }  
        if ((toUpper(splitted[0]) == "CLIENTCERT") && (splitted.size() > 1))
        {
            this->clientCertFilename = splitted[1];
        }  
        if ((toUpper(splitted[0]) == "CLIENTKEY") && (splitted.size() > 1))
        {
            this->clientKeyFilename = splitted[1];
        }  
        if ((toUpper(splitted[0]) == "USER") && (splitted.size() > 1))
        {
            this->user = splitted[1];
        }  
        if ((toUpper(splitted[0]) == "PASSWORD") && (splitted.size() > 1))
        {
            this->password = splitted[1];
        }               
        if ((toUpper(splitted[0]) == "URI") && (splitted.size() > 1))
        {
            this->uri = splitted[1];
        }
        if ((toUpper(splitted[0]) == "RETAINMESSAGES") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE") {
                SetRetainFlag = true;  
                setMqtt_Server_Retain(SetRetainFlag);
            }
        }
        if ((toUpper(splitted[0]) == "HOMEASSISTANTDISCOVERY") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                SetHomeassistantDiscoveryEnabled(true);  
        }
        if ((toUpper(splitted[0]) == "METERTYPE") && (splitted.size() > 1)) {
        /* Use meter type for the device class 
           Make sure it is a listed one on https://developers.home-assistant.io/docs/core/entity/sensor/#available-device-classes */
            if (toUpper(splitted[1]) == "WATER_M3") {
                mqttServer_setMeterType("water", "m³", "h", "m³/h");
            }
            else if (toUpper(splitted[1]) == "WATER_L") {
                mqttServer_setMeterType("water", "L", "h", "L/h");
            }
            else if (toUpper(splitted[1]) == "WATER_FT3") {
                mqttServer_setMeterType("water", "ft³", "m", "ft³/m"); // Minutes
            }
            else if (toUpper(splitted[1]) == "WATER_GAL") {
                mqttServer_setMeterType("water", "gal", "h", "gal/h");
            }
            else if (toUpper(splitted[1]) == "GAS_M3") {
                mqttServer_setMeterType("gas", "m³", "h", "m³/h");
            }
            else if (toUpper(splitted[1]) == "GAS_FT3") {
                mqttServer_setMeterType("gas", "ft³", "m", "ft³/m"); // Minutes
            }
            else if (toUpper(splitted[1]) == "ENERGY_WH") {
                mqttServer_setMeterType("energy", "Wh", "h", "W");
            }
            else if (toUpper(splitted[1]) == "ENERGY_KWH") {
                mqttServer_setMeterType("energy", "kWh", "h", "kW");
            }
            else if (toUpper(splitted[1]) == "ENERGY_MWH") {
                mqttServer_setMeterType("energy", "MWh", "h", "MW");
            }
            else if (toUpper(splitted[1]) == "ENERGY_GJ") {
                mqttServer_setMeterType("energy", "GJ", "h", "GJ/h");
            }
        }

        if ((toUpper(splitted[0]) == "CLIENTID") && (splitted.size() > 1))
        {
            this->clientname = splitted[1];
        }

        if (((toUpper(splitted[0]) == "TOPIC") || (toUpper(splitted[0]) == "MAINTOPIC")) && (splitted.size() > 1))
        {
            maintopic = splitted[1];
        }
    }

    /* Note:
     * Originally, we started the MQTT client here.
     * How ever we need the interval parameter from the ClassFlowControll, but that only gets started later.
     * To work around this, we delay the start and trigger it from ClassFlowControll::ReadParameter() */

    mqttServer_setMainTopic(maintopic);

    return true;
}


bool ClassFlowMQTT::Start(float AutoInterval) 
{
    roundInterval = AutoInterval; // Minutes
    keepAlive = roundInterval * 60 * 2.5; // Seconds, make sure it is greater thatn 2 rounds!

    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << "Digitizer interval is " << roundInterval <<
            " minutes => setting MQTT LWT timeout to " << ((float)keepAlive/60) << " minutes.";
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, stream.str());

    mqttServer_setParameter(flowpostprocessing->GetNumbers(), keepAlive, roundInterval);

    bool MQTTConfigCheck = MQTT_Configure(uri, clientname, user, password, maintopic, LWT_TOPIC, LWT_CONNECTED,
                                     LWT_DISCONNECTED, caCertFilename, clientCertFilename, clientKeyFilename,
                                     keepAlive, SetRetainFlag, (void *)&GotConnected);

    if (!MQTTConfigCheck) {
        return false;
    }

    return (MQTT_Init() == 1);
}


bool ClassFlowMQTT::doFlow(string zwtime)
{
    bool success;
    std::string result;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultpre = "";
    std::string resultrate = ""; // Always Unit / Minute
    std::string resultRatePerTimeUnit = ""; // According to selection
    std::string resulttimestamp = "";
    std::string resultchangabs = "";
    string zw = "";
    string namenumber = "";
    int qos = 1;

    /* Send the the Homeassistant Discovery and the Static Topics in case they where scheduled */
    sendDiscovery_and_static_Topics();

    success = publishSystemData(qos);

    if (flowpostprocessing && getMQTTisConnected())
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Publishing MQTT topics...");

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
            resultpre =  (*NUMBERS)[i]->ReturnPreValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue; // Unit per minutes
            resultchangabs = (*NUMBERS)[i]->ReturnChangeAbsolute; // Units per round
            resulttimestamp = (*NUMBERS)[i]->timeStamp;

            namenumber = (*NUMBERS)[i]->name;
            if (namenumber == "default")
                namenumber = maintopic + "/";
            else
                namenumber = maintopic + "/" + namenumber + "/";


            if (result.length() > 0)   
                success |= MQTTPublish(namenumber + "value", result, qos, SetRetainFlag);

            if (resulterror.length() > 0)  
                success |= MQTTPublish(namenumber + "error", resulterror, qos, SetRetainFlag);

            if (resultrate.length() > 0) {
                success |= MQTTPublish(namenumber + "rate", resultrate, qos, SetRetainFlag);
                
                std::string resultRatePerTimeUnit;
                if (getTimeUnit() == "h") { // Need conversion to be per hour
                    resultRatePerTimeUnit = resultRatePerTimeUnit = to_string((*NUMBERS)[i]->FlowRateAct * 60); // per minutes => per hour
                }
                else { // Keep per minute
                    resultRatePerTimeUnit = resultrate;
                }
                success |= MQTTPublish(namenumber + "rate_per_time_unit", resultRatePerTimeUnit, qos, SetRetainFlag);
            }

            if (resultchangabs.length() > 0) {
                success |= MQTTPublish(namenumber + "changeabsolut", resultchangabs, qos, SetRetainFlag); // Legacy API
                success |= MQTTPublish(namenumber + "rate_per_digitalization_round", resultchangabs, qos, SetRetainFlag);
            }

            if (resultraw.length() > 0)   
                success |= MQTTPublish(namenumber + "raw", resultraw, qos, SetRetainFlag);

            if (resulttimestamp.length() > 0)
                success |= MQTTPublish(namenumber + "timestamp", resulttimestamp, qos, SetRetainFlag);

            std::string json = flowpostprocessing->getJsonFromNumber(i, "\n");
            success |= MQTTPublish(namenumber + "json", json, qos, SetRetainFlag);
        }
    }
    
    /* Disabled because this is no longer a use case */
    // else
    // {
    //     for (int i = 0; i < ListFlowControll->size(); ++i)
    //     {
    //         zw = (*ListFlowControll)[i]->getReadout();
    //         if (zw.length() > 0)
    //         {
    //             if (result.length() == 0)
    //                 result = zw;
    //             else
    //                 result = result + "\t" + zw;
    //         }
    //     }
    //     success |= MQTTPublish(topic, result, qos, SetRetainFlag);
    // }
    
    OldValue = result;

    if (!success) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "One or more MQTT topics failed to be published!");
    }
    
    return true;
}


#endif //ENABLE_MQTT