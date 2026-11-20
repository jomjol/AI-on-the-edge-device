#ifdef ENABLE_INFLUXDB
#include <sstream>
#include "ClassFlowInfluxDB.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_influxdb.h"

#include "ClassFlowPostProcessing.h"
#include "esp_log.h"
#include "defines.h"

#include "ClassLogFile.h"

#include <time.h>

static const char *TAG = "INFLUXDB";

void ClassFlowInfluxDB::SetInitialParameter(void)
{
    uri = "";
    database = "";

    OldValue = "";
    flowpostprocessing = NULL;
    user = "";
    password = "";
    previousElement = NULL;
    ListFlowControll = NULL;
    disabled = false;
    InfluxDBenable = false;
}

ClassFlowInfluxDB::ClassFlowInfluxDB()
{
    SetInitialParameter();
}

ClassFlowInfluxDB::ClassFlowInfluxDB(std::vector<ClassFlow *> *lfc)
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

ClassFlowInfluxDB::ClassFlowInfluxDB(std::vector<ClassFlow *> *lfc, ClassFlow *_prev)
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

bool ClassFlowInfluxDB::ReadParameter(FILE *pfile, std::string &aktparamgraph)
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
    if (_aktparamgraph.compare("[INFLUXDB]") != 0)
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

            if (_param == "USER")
            {
                user = splitted[1];
            }
            else if (_param == "PASSWORD")
            {
                password = splitted[1];
            }
            else if (_param == "URI")
            {
                uri = splitted[1];
            }
            else if (_param == "DATABASE")
            {
                database = splitted[1];
            }
            else if (_param == "MEASUREMENT")
            {
                handleMeasurement(splitted[0], splitted[1]);
            }
            else if (_param == "FIELD")
            {
                handleFieldname(splitted[0], splitted[1]);
            }
        }
    }

    if ((uri.length() > 0) && (database.length() > 0))
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init InfluxDB with uri: " + uri + ", user: " + user + ", password: " + password);
        influxDB.InfluxDBInitV1(uri, database, user, password);
        InfluxDBenable = true;
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDB init skipped as we are missing some parameters");
    }

    return true;
}

bool ClassFlowInfluxDB::doFlow(std::string zwtime)
{
    if (!InfluxDBenable)
    {
        return true;
    }

    std::string result;
    std::string measurement;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    long int timeutc;
    std::string namenumber = "";

    if (flowpostprocessing)
    {
        std::vector<NumberPost *> *NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            measurement = (*NUMBERS)[i]->MeasurementV1;
            result = (*NUMBERS)[i]->ReturnValue;
            resultraw = (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;
            timeutc = (*NUMBERS)[i]->timeStampTimeUTC;

            if ((*NUMBERS)[i]->FieldV1.length() > 0)
            {
                namenumber = (*NUMBERS)[i]->FieldV1;
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
                influxDB.InfluxDBPublish(measurement, namenumber, result, timeutc);
            }
        }
    }

    OldValue = result;

    return true;
}

void ClassFlowInfluxDB::handleMeasurement(std::string _decsep, std::string _value)
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
            flowpostprocessing->NUMBERS[j]->MeasurementV1 = _value;
        }
    }
}

void ClassFlowInfluxDB::handleFieldname(std::string _decsep, std::string _value)
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
            flowpostprocessing->NUMBERS[j]->FieldV1 = _value;
        }
    }
}

#endif // ENABLE_INFLUXDB
