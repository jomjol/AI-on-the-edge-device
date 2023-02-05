#ifdef ENABLE_INFLUXDB

#pragma once

#ifndef CLASSFINFLUXDBv2_H
#define CLASSFINFLUXDBv2_H

#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowInfluxDBv2 :
    public ClassFlow
{
protected:
    std::string uri, database, measurement, org, token;
    std::string OldValue;
	ClassFlowPostProcessing* flowpostprocessing;  
    bool InfluxDBenable;

    void SetInitialParameter(void);        

public:
    ClassFlowInfluxDBv2();
    ClassFlowInfluxDBv2(std::vector<ClassFlow*>* lfc);
    ClassFlowInfluxDBv2(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

    string GetInfluxDBMeasurement();

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowInfluxDBv2";};
};

#endif //CLASSFINFLUXDBv2_H
#endif //ENABLE_INFLUXDB