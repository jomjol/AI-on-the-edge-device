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
    bool MQTTenable;

    std::string maintopic, mainerrortopic; 
	void SetInitialParameter(void);        

public:
    ClassFlowMQTT();
    ClassFlowMQTT(std::vector<ClassFlow*>* lfc);
    ClassFlowMQTT(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

    string GetMQTTMainTopic();

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowMQTT";};
};

