#ifdef ENABLE_WEBHOOK
#include <sstream>
#include "ClassFlowWebhook.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_webhook.h"

#include "ClassFlowPostProcessing.h"
#include "ClassFlowAlignment.h"
#include "esp_log.h"
#include "defines.h"

#include "ClassLogFile.h"

#include <time.h>

static const char *TAG = "WEBHOOK";

void ClassFlowWebhook::SetInitialParameter(void)
{
    uri = "";
    flowpostprocessing = NULL;
    flowAlignment = NULL;
    previousElement = NULL;
    ListFlowControll = NULL;
    disabled = false;
    WebhookEnable = false;
    WebhookUploadImg = 0;
}

ClassFlowWebhook::ClassFlowWebhook()
{
    SetInitialParameter();
}

ClassFlowWebhook::ClassFlowWebhook(std::vector<ClassFlow *> *lfc)
{
    SetInitialParameter();

    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing *)(*ListFlowControll)[i];
        }
        else if (((*ListFlowControll)[i])->name().compare("ClassFlowAlignment") == 0)
        {
            flowAlignment = (ClassFlowAlignment *)(*ListFlowControll)[i];
        }
    }
}

ClassFlowWebhook::ClassFlowWebhook(std::vector<ClassFlow *> *lfc, ClassFlow *_prev)
{
    SetInitialParameter();

    previousElement = _prev;
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing *)(*ListFlowControll)[i];
        }
        else if (((*ListFlowControll)[i])->name().compare("ClassFlowAlignment") == 0)
        {
            flowAlignment = (ClassFlowAlignment *)(*ListFlowControll)[i];
        }
    }
}

bool ClassFlowWebhook::ReadParameter(FILE *pfile, string &aktparamgraph)
{
    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
    {
        if (!GetNextParagraph(pfile, aktparamgraph))
        {
            return false;
        }
    }

    const std::string _aktparamgraph = toUpper(aktparamgraph);
    if (_aktparamgraph.compare("[WEBHOOK]") != 0)
    {
        return false;
    }

    std::vector<string> splitted;

    while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);

        if (splitted.size() > 1)
        {
            const std::string _param = toUpper(GetParameterName(splitted[0]));

            if (_param == "URI")
            {
                uri = splitted[1];
            }
            else if (_param == "APIKEY")
            {
                apikey = splitted[1];
            }
            else if (_param == "UPLOADIMG")
            {
                if (toUpper(splitted[1]) == "1")
                {
                    WebhookUploadImg = 1;
                }
                else if (toUpper(splitted[1]) == "2")
                {
                    WebhookUploadImg = 2;
                }
            }
        }
    }

    WebhookInit(uri, apikey);

    if (uri.empty())
    {
        WebhookEnable = false;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Webhook not enabled: no Uri configured");
    }
    else
    {
        WebhookEnable = true;
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Webhook Enabled for Uri " + uri);
    }

    return true;
}

void ClassFlowWebhook::handleMeasurement(std::string _decsep, std::string _value)
{
    std::string _digit = "default";
    int _pospunkt = _decsep.find_first_of(".");

    if (_pospunkt > -1)
    {
        _digit = _decsep.substr(0, _pospunkt);
    }

    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j)
    {
        //  Set to default first (if nothing else is set)
        if ((_digit == "default") || (flowpostprocessing->NUMBERS[j]->name == _digit))
        {
            flowpostprocessing->NUMBERS[j]->MeasurementV2 = _value;
        }
    }
}

bool ClassFlowWebhook::doFlow(string zwtime)
{
    if (!WebhookEnable)
    {
        return true;
    }

    if (flowpostprocessing)
    {
        bool numbersWithError = WebhookPublish(flowpostprocessing->GetNumbers());

#ifdef ALGROI_LOAD_FROM_MEM_AS_JPG
        if ((WebhookUploadImg == 1 || (WebhookUploadImg != 0 && numbersWithError)) && flowAlignment && flowAlignment->AlgROI)
        {
            WebhookUploadPic(flowAlignment->AlgROI);
        }
#endif
    }

    return true;
}
#endif // ENABLE_WEBHOOK
