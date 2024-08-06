#ifdef ENABLE_WEBHOOK

#pragma once

#ifndef CLASSFWEBHOOK_H
#define CLASSFWEBHOOK_H

#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowWebhook :
    public ClassFlow
{
protected:
    std::string uri, apikey;
    std::string OldValue;
	ClassFlowPostProcessing* flowpostprocessing;  
    bool WebhookEnable;

    void SetInitialParameter(void); 

    void handleFieldname(string _decsep, string _value);   
    void handleMeasurement(string _decsep, string _value);


public:
    ClassFlowWebhook();
    ClassFlowWebhook(std::vector<ClassFlow*>* lfc);
    ClassFlowWebhook(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowWebhook";};
};

#endif //CLASSFWEBHOOK_H
#endif //ENABLE_WEBHOOK