#pragma once

#ifdef ENABLE_WEBHOOK

#ifndef CLASSFWEBHOOK_H
#define CLASSFWEBHOOK_H

#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"
#include "ClassFlowAlignment.h"

#include <string>

class ClassFlowWebhook : public ClassFlow
{
protected:
    std::string uri, apikey;
    ClassFlowPostProcessing *flowpostprocessing;
    ClassFlowAlignment *flowAlignment;

    bool WebhookEnable;
    int WebhookUploadImg;

    void SetInitialParameter(void);

    void handleFieldname(std::string _decsep, std::string _value);
    void handleMeasurement(std::string _decsep, std::string _value);

public:
    ClassFlowWebhook(void);
    ClassFlowWebhook(std::vector<ClassFlow *> *lfc);
    ClassFlowWebhook(std::vector<ClassFlow *> *lfc, ClassFlow *_prev);

    bool ReadParameter(FILE *pfile, std::string &aktparamgraph);
    bool doFlow(std::string time);
	
    std::string name(void) { return "ClassFlowWebhook"; };
};

#endif // CLASSFWEBHOOK_H
#endif // ENABLE_WEBHOOK