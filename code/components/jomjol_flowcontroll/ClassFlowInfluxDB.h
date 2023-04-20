#ifdef ENABLE_INFLUXDB

#pragma once

#ifndef CLASSFINFLUXDB_H
#define CLASSFINFLUXDB_H

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
    
    void handleFieldname(string _decsep, string _value);   
    void handleMeasurement(string _decsep, string _value);

    

public:
    ClassFlowInfluxDB();
    ClassFlowInfluxDB(std::vector<ClassFlow*>* lfc);
    ClassFlowInfluxDB(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);

//    string GetInfluxDBMeasurement();

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowInfluxDB";};
};

#endif //CLASSFINFLUXDB_H
#endif //ENABLE_INFLUXDB