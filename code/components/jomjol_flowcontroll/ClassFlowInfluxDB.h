#pragma once
#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowInfluxDB :
    public ClassFlow
{
protected:
    std::string uri, database, measurement;
    std::string OldValue;
	ClassFlowPostProcessing* flowpostprocessing;  
    std::string user, password; 
    bool InfluxDBenable;

    void SetInitialParameter(void);        

public:
    ClassFlowInfluxDB();
    ClassFlowInfluxDB(std::vector<ClassFlow*>* lfc);
    ClassFlowInfluxDB(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

    string GetInfluxDBMeasurement();

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowInfluxDB";};
};

