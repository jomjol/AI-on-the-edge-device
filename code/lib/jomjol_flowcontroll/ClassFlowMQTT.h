#pragma once
#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowMQTT :
    public ClassFlow
{
protected:
    std::string uri, topic, clientname;
    std::string OldValue;
	ClassFlowPostProcessing* flowpostprocessing;    


public:
    ClassFlowMQTT();
    ClassFlowMQTT(std::vector<ClassFlow*>* lfc);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowMQTT";};
};

