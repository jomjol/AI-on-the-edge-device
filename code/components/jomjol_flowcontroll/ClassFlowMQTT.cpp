#include "ClassFlowMQTT.h"
#include "Helper.h"

#include "interface_mqtt.h"
#include "ClassFlowPostProcessing.h"

#include <time.h>

ClassFlowMQTT::ClassFlowMQTT()
{
    uri = "";
    topic = "";
    clientname = "watermeter";
    OldValue = "";
    flowpostprocessing = NULL;  
    user = "";
    password = "";    
}

ClassFlowMQTT::ClassFlowMQTT(std::vector<ClassFlow*>* lfc)
{
    uri = "";
    topic = "";
    clientname = "watermeter";
    OldValue = "";
    flowpostprocessing = NULL;
    user = "";
    password = "";        

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
        if ((toUpper(zerlegt[0]) == "CLIENTID") && (zerlegt.size() > 1))
        {
            this->clientname = zerlegt[1];
        }

    }

    if ((uri.length() > 0) && (topic.length() > 0)) 
    {
        MQTTInit(uri, clientname, user, password);
    }
   
    return true;
}


bool ClassFlowMQTT::doFlow(string zwtime)
{
    std::string result;
    string zw = "";
    
    if (flowpostprocessing)
    {
        result =  flowpostprocessing->getReadoutParam(false, true);
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

    OldValue = result;


    return true;
}
