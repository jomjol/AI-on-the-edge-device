#pragma once

#ifdef ENABLE_INFLUXDB

#ifndef CLASSFINFLUXDB_H
#define CLASSFINFLUXDB_H

#include "ClassFlow.h"

#include "ClassFlowPostProcessing.h"
#include "interface_influxdb.h"

#include <string>

class ClassFlowInfluxDB : public ClassFlow
{
protected:
    std::string uri, database, measurement;
    std::string OldValue;
    ClassFlowPostProcessing *flowpostprocessing;
    std::string user, password;
    bool InfluxDBenable;

    InfluxDB influxDB;

    void SetInitialParameter(void);

    void handleFieldname(std::string _decsep, std::string _value);
    void handleMeasurement(std::string _decsep, std::string _value);

public:
    ClassFlowInfluxDB(void);
    ClassFlowInfluxDB(std::vector<ClassFlow *> *lfc);
    ClassFlowInfluxDB(std::vector<ClassFlow *> *lfc, ClassFlow *_prev);

    // std::string GetInfluxDBMeasurement(void);

    bool ReadParameter(FILE *pfile, std::string &aktparamgraph);
    bool doFlow(std::string time);
    std::string name(void) { return "ClassFlowInfluxDB"; };
};

#endif // CLASSFINFLUXDB_H
#endif // ENABLE_INFLUXDB