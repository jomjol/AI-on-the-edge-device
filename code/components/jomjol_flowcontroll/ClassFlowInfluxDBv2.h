#pragma once

#ifdef ENABLE_INFLUXDB

#ifndef CLASSFINFLUXDBv2_H
#define CLASSFINFLUXDBv2_H

#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"

#include "interface_influxdb.h"

#include <string>

class ClassFlowInfluxDBv2 : public ClassFlow
{
protected:
    std::string uri, bucket;
    std::string dborg, dbtoken, dbfield;
    std::string OldValue;
    ClassFlowPostProcessing *flowpostprocessing;
    bool InfluxDBenable;

    InfluxDB influxdb;

    void SetInitialParameter(void);

    void handleFieldname(std::string _decsep, std::string _value);
    void handleMeasurement(std::string _decsep, std::string _value);

public:
    ClassFlowInfluxDBv2(void);
    ClassFlowInfluxDBv2(std::vector<ClassFlow *> *lfc);
    ClassFlowInfluxDBv2(std::vector<ClassFlow *> *lfc, ClassFlow *_prev);

    // std::string GetInfluxDBMeasurement(void);

    bool ReadParameter(FILE *pfile, std::string &aktparamgraph);
    bool doFlow(std::string time);
    std::string name(void) { return "ClassFlowInfluxDBv2"; };
};

#endif // CLASSFINFLUXDBv2_H
#endif // ENABLE_INFLUXDB