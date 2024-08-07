#ifdef ENABLE_WEBHOOK
#include <sstream>
#include "ClassFlowWebhook.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_webhook.h"

#include "ClassFlowPostProcessing.h"
#include "esp_log.h"
#include "../../include/defines.h"

#include "ClassLogFile.h"

#include <time.h>

static const char* TAG = "WEBHOOK";

void ClassFlowWebhook::SetInitialParameter(void)
{
    uri = "";
    flowpostprocessing = NULL;  
    previousElement = NULL;
    ListFlowControll = NULL; 
    disabled = false;
    WebhookEnable = false;
}       

ClassFlowWebhook::ClassFlowWebhook()
{
    SetInitialParameter();
}

ClassFlowWebhook::ClassFlowWebhook(std::vector<ClassFlow*>* lfc)
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

ClassFlowWebhook::ClassFlowWebhook(std::vector<ClassFlow*>* lfc, ClassFlow *_prev)
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


bool ClassFlowWebhook::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);
    printf("akt param: %s\n", aktparamgraph.c_str());

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[WEBHOOK]") != 0) 
        return false;

    

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        ESP_LOGD(TAG, "while loop reading line: %s", aktparamgraph.c_str());
        splitted = ZerlegeZeile(aktparamgraph);
        std::string _param = GetParameterName(splitted[0]);
            
        if ((toUpper(_param) == "URI") && (splitted.size() > 1))
        {
            this->uri = splitted[1];
        }
        if (((toUpper(_param) == "APIKEY")) && (splitted.size() > 1))
        {
            this->apikey = splitted[1];
        }
    }
    
    WebhookInit(uri,apikey);
    WebhookEnable = true;
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Webhook Enabled for Uri " + uri);

    printf("uri:         %s\n", uri.c_str());   
    return true;
}


void ClassFlowWebhook::handleMeasurement(string _decsep, string _value)
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
            flowpostprocessing->NUMBERS[j]->MeasurementV2 = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit)
        {
            flowpostprocessing->NUMBERS[j]->MeasurementV2 = _value;
        }
    }
}


bool ClassFlowWebhook::doFlow(string zwtime)
{
    if (!WebhookEnable)
        return true;

    std::string resultValue;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    long int resulttimeutc = 0;
    string zw = "";
    string namenumber = "";


    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            resultValue =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;
            resulttimeutc = (*NUMBERS)[i]->timeStampTimeUTC;
            
            printf("vor sende WebHook - result: %s, timestampt: %s", resultValue.c_str(), resulttimestamp.c_str());
            WebhookPublish(resultValue, resultraw, resulterror, resultrate, resulttimestamp, resulttimeutc);
        }
    }
       
    return true;
}

#endif //ENABLE_WEBHOOK