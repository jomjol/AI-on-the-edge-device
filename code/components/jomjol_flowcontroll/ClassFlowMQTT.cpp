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
    validateServerCert = true;
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
    domoticzintopic = "";
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
        std::string _param = GetParameterName(splitted[0]);
        if ((toUpper(_param) == "CACERT") && (splitted.size() > 1))
        {
            this->caCertFilename = "/sdcard" + splitted[1];
        }
        if ((toUpper(_param) == "VALIDATESERVERCERT") && (splitted.size() > 1))
        {
            validateServerCert = alphanumericToBoolean(splitted[1]);
        }  
        if ((toUpper(_param) == "CLIENTCERT") && (splitted.size() > 1))
        {
            this->clientCertFilename = "/sdcard" + splitted[1];
        }  
        if ((toUpper(_param) == "CLIENTKEY") && (splitted.size() > 1))
        {
            this->clientKeyFilename = "/sdcard" + splitted[1];
        }  
        if ((toUpper(_param) == "USER") && (splitted.size() > 1))
        {
            this->user = splitted[1];
        }  
        if ((toUpper(_param) == "PASSWORD") && (splitted.size() > 1))
        {
            this->password = splitted[1];
        }               
        if ((toUpper(_param) == "URI") && (splitted.size() > 1))
        {
            this->uri = splitted[1];
        }
        if ((toUpper(_param) == "RETAINMESSAGES") && (splitted.size() > 1))
        {
            SetRetainFlag = alphanumericToBoolean(splitted[1]);
            setMqtt_Server_Retain(SetRetainFlag);
        }
        if ((toUpper(_param) == "HOMEASSISTANTDISCOVERY") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                SetHomeassistantDiscoveryEnabled(true);  
        }
        if ((toUpper(_param) == "METERTYPE") && (splitted.size() > 1)) {
        /* Use meter type for the device class 
           Make sure it is a listed one on https://developers.home-assistant.io/docs/core/entity/sensor/#available-device-classes */
            if (toUpper(splitted[1]) == "WATER_M3") {
                mqttServer_setMeterType("water", "m³", "h", "m³/h");
            }
            else if (toUpper(splitted[1]) == "WATER_L") {
                mqttServer_setMeterType("water", "L", "h", "L/h");
            }
            else if (toUpper(splitted[1]) == "WATER_FT3") {
                mqttServer_setMeterType("water", "ft³", "min", "ft³/min"); // min = Minutes
            }
            else if (toUpper(splitted[1]) == "WATER_GAL") {
                mqttServer_setMeterType("water", "gal", "h", "gal/h");
            }
            else if (toUpper(splitted[1]) == "GAS_M3") {
                mqttServer_setMeterType("gas", "m³", "h", "m³/h");
            }
            else if (toUpper(splitted[1]) == "GAS_FT3") {
                mqttServer_setMeterType("gas", "ft³", "min", "ft³/min"); // min = Minutes
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
            else if (toUpper(splitted[1]) == "TEMPERATURE_C") {
                mqttServer_setMeterType("temperature", "°C", "min", "°C/min"); // min = Minutes
            }
            else if (toUpper(splitted[1]) == "TEMPERATURE_F") {
                mqttServer_setMeterType("temperature", "°F", "min", "°F/min"); // min = Minutes
            }
            else if (toUpper(splitted[1]) == "TEMPERATURE_K") {
                mqttServer_setMeterType("temperature", "K", "min", "K/m"); // min = Minutes
            }
        }

        if ((toUpper(_param) == "CLIENTID") && (splitted.size() > 1))
        {
            this->clientname = splitted[1];
        }

        if (((toUpper(_param) == "TOPIC") || (toUpper(splitted[0]) == "MAINTOPIC")) && (splitted.size() > 1))
        {
            maintopic = splitted[1];
        }

        if (((toUpper(_param) == "DOMOTICZTOPICIN")) && (splitted.size() > 1))
        {
            this->domoticzintopic = splitted[1];
        }

        if (((toUpper(_param) == "DOMOTICZIDX")) && (splitted.size() > 1))
        {
            handleIdx(splitted[0], splitted[1]);
        }
        
    }

    /* Note:
     * Originally, we started the MQTT client here.
     * How ever we need the interval parameter from the ClassFlowControll, but that only gets started later.
     * To work around this, we delay the start and trigger it from ClassFlowControll::ReadParameter() */

    mqttServer_setMainTopic(maintopic);
    mqttServer_setDmoticzInTopic(domoticzintopic);

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

    bool MQTTConfigCheck = MQTT_Configure(uri, clientname, user, password, maintopic, domoticzintopic, LWT_TOPIC, LWT_CONNECTED,
                                     LWT_DISCONNECTED, caCertFilename, validateServerCert, clientCertFilename, clientKeyFilename,
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
    string domoticzpayload = "";
    string DomoticzIdx = "";
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

            DomoticzIdx = (*NUMBERS)[i]->DomoticzIdx;
            domoticzpayload = "{\"command\":\"udevice\",\"idx\":" + DomoticzIdx + ",\"svalue\":\""+ result + "\"}";

            namenumber = (*NUMBERS)[i]->name;
            if (namenumber == "default")
                namenumber = maintopic + "/";
            else
                namenumber = maintopic + "/" + namenumber + "/";

            if ((domoticzintopic.length() > 0) && (result.length() > 0)) 
                success |= MQTTPublish(domoticzintopic, domoticzpayload, qos, SetRetainFlag);

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
                success |= MQTTPublish(namenumber + "rate_per_digitization_round", resultchangabs, qos, SetRetainFlag);
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
void ClassFlowMQTT::handleIdx(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
//    ESP_LOGD(TAG, "Name: %s, Pospunkt: %d", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1)
        _digit = _decsep.substr(0, _pospunkt);
    else
        _digit = "default";
    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j)
    {
        if (_digit == "default")                        //  Set to default first (if nothing else is set)
        {
            flowpostprocessing->NUMBERS[j]->DomoticzIdx = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit)
        {
            flowpostprocessing->NUMBERS[j]->DomoticzIdx = _value;
        }
    }
}

#endif //ENABLE_MQTT
