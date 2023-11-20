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
    std::string uri, bucket;
    std::string dborg, dbtoken, dbfield;
    std::string OldValue;
	ClassFlowPostProcessing* flowpostprocessing;  
    bool InfluxDBenable;

    void SetInitialParameter(void);     

    void handleFieldname(string _decsep, string _value);   
    void handleMeasurement(string _decsep, string _value);


public:
    ClassFlowInfluxDBv2();
    ClassFlowInfluxDBv2(std::vector<ClassFlow*>* lfc);
    ClassFlowInfluxDBv2(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

//    string GetInfluxDBMeasurement();

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowInfluxDBv2";};
};

#endif //CLASSFINFLUXDBv2_H
#endif //ENABLE_INFLUXDB