#ifdef ENABLE_INFLUXDB
#include <sstream>
#include "ClassFlowInfluxDBv2.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_influxdb.h"

#include "ClassFlowPostProcessing.h"
#include "esp_log.h"
#include "defines.h"

#include "ClassLogFile.h"

#include <time.h>

static const char *TAG = "INFLUXDBV2";

void ClassFlowInfluxDBv2::SetInitialParameter(void)
{
    uri = "";
    bucket = "";
    dborg = "";
    dbtoken = "";
    // dbfield = "";

    OldValue = "";
    flowpostprocessing = NULL;
    previousElement = NULL;
    ListFlowControll = NULL;
    disabled = false;
    InfluxDBenable = false;
}

ClassFlowInfluxDBv2::ClassFlowInfluxDBv2()
{
    SetInitialParameter();
}

ClassFlowInfluxDBv2::ClassFlowInfluxDBv2(std::vector<ClassFlow *> *lfc)
{
    SetInitialParameter();

    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing *)(*ListFlowControll)[i];
        }
    }
}

ClassFlowInfluxDBv2::ClassFlowInfluxDBv2(std::vector<ClassFlow *> *lfc, ClassFlow *_prev)
{
    SetInitialParameter();

    previousElement = _prev;
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing *)(*ListFlowControll)[i];
        }
    }
}

bool ClassFlowInfluxDBv2::ReadParameter(FILE *pfile, std::string &aktparamgraph)
{
    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
    {
        if (!GetNextParagraph(pfile, aktparamgraph))
        {
            return false;
        }
    }

    const std::string _aktparamgraph = toUpper(aktparamgraph);
    if (_aktparamgraph.compare("[INFLUXDBV2]") != 0)
    {
        return false;
    }

    std::vector<std::string> splitted;

    while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);

        if (splitted.size() > 1)
        {
            const std::string _param = toUpper(GetParameterName(splitted[0]));

            if (_param == "ORG")
            {
                dborg = splitted[1];
            }
            else if (_param == "TOKEN")
            {
                dbtoken = splitted[1];
            }
            else if (_param == "URI")
            {
                uri = splitted[1];
            }
            else if (_param == "FIELD")
            {
                handleFieldname(splitted[0], splitted[1]);
            }
            else if (_param == "MEASUREMENT")
            {
                handleMeasurement(splitted[0], splitted[1]);
            }
            else if (_param == "BUCKET")
            {
                bucket = splitted[1];
            }
        }
    }

    if ((uri.length() > 0) && (bucket.length() > 0) && (dbtoken.length() > 0) && (dborg.length() > 0))
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init InfluxDB with uri: " + uri + ", org: " + dborg + ", token: *****");
        influxdb.InfluxDBInitV2(uri, bucket, dborg, dbtoken);
        InfluxDBenable = true;
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDBv2 (Verion2 !!!) init skipped as we are missing some parameters");
    }

    return true;
}

void ClassFlowInfluxDBv2::handleFieldname(std::string _decsep, std::string _value)
{
    std::string _digit = "default";
    int _pospunkt = _decsep.find_first_of(".");

    if (_pospunkt > -1)
    {
        _digit = _decsep.substr(0, _pospunkt);
    }

    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j)
    {
        //  Set to default first (if nothing else is set)
        if ((_digit == "default") || (flowpostprocessing->NUMBERS[j]->name == _digit))
        {
            flowpostprocessing->NUMBERS[j]->FieldV2 = _value;
        }
    }
}

void ClassFlowInfluxDBv2::handleMeasurement(std::string _decsep, std::string _value)
{
    std::string _digit = "default";
    int _pospunkt = _decsep.find_first_of(".");

    if (_pospunkt > -1)
    {
        _digit = _decsep.substr(0, _pospunkt);
    }

    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j)
    {
        //  Set to default first (if nothing else is set)
        if ((_digit == "default") || (flowpostprocessing->NUMBERS[j]->name == _digit))
        {
            flowpostprocessing->NUMBERS[j]->MeasurementV2 = _value;
        }
    }
}

bool ClassFlowInfluxDBv2::doFlow(std::string zwtime)
{
    if (!InfluxDBenable)
    {
        return true;
    }

    std::string measurement;
    std::string result;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    long int resulttimeutc = 0;
    std::string namenumber = "";

    if (flowpostprocessing)
    {
        std::vector<NumberPost *> *NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            measurement = (*NUMBERS)[i]->MeasurementV2;
            result = (*NUMBERS)[i]->ReturnValue;
            resultraw = (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;
            resulttimeutc = (*NUMBERS)[i]->timeStampTimeUTC;

            if ((*NUMBERS)[i]->FieldV2.length() > 0)
            {
                namenumber = (*NUMBERS)[i]->FieldV2;
            }
            else
            {
                namenumber = (*NUMBERS)[i]->name;

                if (namenumber == "default")
                {
                    namenumber = "value";
                }
                else
                {
                    namenumber = namenumber + "/value";
                }
            }

            if (result.length() > 0)
            {
                influxdb.InfluxDBPublish(measurement, namenumber, result, resulttimeutc);
            }
        }
    }

    OldValue = result;

    return true;
}

#endif // ENABLE_INFLUXDB
