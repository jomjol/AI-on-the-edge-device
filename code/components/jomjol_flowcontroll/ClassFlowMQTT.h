#pragma once

#ifdef ENABLE_MQTT

#ifndef CLASSFFLOWMQTT_H
#define CLASSFFLOWMQTT_H

#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowMQTT : public ClassFlow
{
protected:
    std::string uri, topic, topicError, clientname, topicRate, topicTimeStamp, topicUptime, topicFreeMem;
    std::string OldValue;
    ClassFlowPostProcessing *flowpostprocessing;
    std::string user, password;
    std::string caCertFilename, clientCertFilename, clientKeyFilename;
    bool validateServerCert;
    bool SetRetainFlag;
    int keepAlive;       // Seconds
    float roundInterval; // Minutes
    std::string maintopic, domoticzintopic;
    void SetInitialParameter(void);
    void handleIdx(std::string _decsep, std::string _value);

public:
    ClassFlowMQTT(void);
    ClassFlowMQTT(std::vector<ClassFlow *> *lfc);
    ClassFlowMQTT(std::vector<ClassFlow *> *lfc, ClassFlow *_prev);

    bool Start(float AutoInterval);

    bool ReadParameter(FILE *pfile, std::string &aktparamgraph);
    bool doFlow(std::string time);
    std::string name(void) { return "ClassFlowMQTT"; };
};
#endif // CLASSFFLOWMQTT_H
#endif // ENABLE_MQTT