#pragma once
#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowMQTT :
    public ClassFlow
{
protected:
    std::string uri, topic, topicError, clientname;
    std::string OldValue;
	ClassFlowPostProcessing* flowpostprocessing;  
    std::string user, password;  
	void SetInitialParameter(void);        

public:
    ClassFlowMQTT();
    ClassFlowMQTT(std::vector<ClassFlow*>* lfc);
    ClassFlowMQTT(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowMQTT";};
};

