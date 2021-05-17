#include "ClassFlowMQTT.h"
#include "Helper.h"

#include "interface_mqtt.h"
#include "ClassFlowPostProcessing.h"

#include <time.h>

void ClassFlowMQTT::SetInitialParameter(void)
{
    uri = "";
    topic = "";
    topicError = "";
    topicRate = "";
    topicTimeStamp = "";
    clientname = "watermeter";
    OldValue = "";
    flowpostprocessing = NULL;  
    user = "";
    password = "";   
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
        if ((toUpper(zerlegt[0]) == "TOPIC") && (zerlegt.size() > 1))
        {
            this->topic = zerlegt[1];
        }
        if ((toUpper(zerlegt[0]) == "TOPICERROR") && (zerlegt.size() > 1))
        {
            this->topicError = zerlegt[1];
        }
        if ((toUpper(zerlegt[0]) == "TOPICRATE") && (zerlegt.size() > 1))
        {
            this->topicRate  = zerlegt[1];
        }
        if ((toUpper(zerlegt[0]) == "TOPICTIMESTAMP") && (zerlegt.size() > 1))
        {
            this->topicTimeStamp  = zerlegt[1];
        }

        if ((toUpper(zerlegt[0]) == "CLIENTID") && (zerlegt.size() > 1))
        {
            this->clientname = zerlegt[1];
        }

    }

    if ((uri.length() > 0) && (topic.length() > 0)) 
    {
        MQTTInit(uri, clientname, user, password, topicError, 60);
    }
   
    return true;
}


bool ClassFlowMQTT::doFlow(string zwtime)
{
    std::string result;
    std::string resulterror = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    string zw = "";
    
    if (flowpostprocessing)
    {
        result =  flowpostprocessing->getReadoutParam(false, true);
        resulterror = flowpostprocessing->getReadoutError();
        resultrate = flowpostprocessing->getReadoutRate();
        resulttimestamp = flowpostprocessing->getReadoutTimeStamp();
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
    }
    
    MQTTPublish(topic, result);

    if (topicError.length() > 0) {
        if (resulterror.length() == 0)
        {
            resulterror = " ";
        }
        MQTTPublish(topicError, resulterror, 1);
    }

    if (topicRate.length() > 0) {
        MQTTPublish(topicRate, resultrate);
    }

    if (topicRate.length() > 0) {
        MQTTPublish(topicTimeStamp, resulttimestamp);
    }

    OldValue = result;
    
    return true;
}
