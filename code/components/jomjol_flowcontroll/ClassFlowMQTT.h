#pragma once
#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowMQTT :
    public ClassFlow
{
protected:
    std::string uri, topic, topicError, clientname, topicRate, topicTimeStamp, topicUptime, topicFreeMem;
    std::string OldValue;
	ClassFlowPostProcessing* flowpostprocessing;  
    std::string user, password; 
    int SetRetainFlag;
    int keepAlive; // Seconds
    float roundInterval; // Minutes

    std::string maintopic; 
	void SetInitialParameter(void);        

public:
    ClassFlowMQTT();
    ClassFlowMQTT(std::vector<ClassFlow*>* lfc);
    ClassFlowMQTT(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

    string GetMQTTMainTopic();
    bool Start(float AutoIntervall);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowMQTT";};
};

