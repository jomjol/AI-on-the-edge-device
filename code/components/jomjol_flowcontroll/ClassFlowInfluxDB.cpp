#ifdef ENABLE_INFLUXDB
#include <sstream>
#include "ClassFlowInfluxDB.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_influxdb.h"

#include "ClassFlowPostProcessing.h"
#include "esp_log.h"
#include "../../include/defines.h"

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
    for (int i = 0; i < ListFlowControll->size(); ++i) {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0) {
            flowpostprocessing = (ClassFlowPostProcessing *)(*ListFlowControll)[i];
        }
    }
}

ClassFlowInfluxDB::ClassFlowInfluxDB(std::vector<ClassFlow *> *lfc, ClassFlow *_prev)
{
    SetInitialParameter();

    previousElement = _prev;
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i) {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0) {
            flowpostprocessing = (ClassFlowPostProcessing *)(*ListFlowControll)[i];
        }
    }
}


bool ClassFlowInfluxDB::ReadParameter(FILE *pfile, string &aktparamgraph)
{
    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0) {
        if (!this->GetNextParagraph(pfile, aktparamgraph)) {
            return false;
        }
    }

    if (toUpper(aktparamgraph).compare("[INFLUXDB]") != 0) {
        return false;
    }

    std::vector<string> splitted;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph)) {
        splitted = ZerlegeZeile(aktparamgraph);

        if (splitted.size() > 1) {
            std::string _param = toUpper(GetParameterName(splitted[0]));

            if (_param == "USER") {
                this->user = splitted[1];
            }
            else if (_param == "PASSWORD") {
                this->password = splitted[1];
            }
            else if (_param == "URI") {
                this->uri = splitted[1];
            }
            else if (_param == "DATABASE") {
                this->database = splitted[1];
            }
            else if (_param == "MEASUREMENT") {
                handleMeasurement(splitted[0], splitted[1]);
            }
            else if (_param == "FIELD") {
                handleFieldname(splitted[0], splitted[1]);
            }
        }
    }

    if ((uri.length() > 0) && (database.length() > 0)) {
        //        ESP_LOGD(TAG, "Init InfluxDB with uri: %s, measurement: %s, user: %s, password: %s", uri.c_str(), measurement.c_str(), user.c_str(), password.c_str());
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init InfluxDB with uri: " + uri + ", user: " + user + ", password: " + password);

        /////////////////////// NEW //////////////////////////
        //        InfluxDBInit(uri, database, user, password);
        influxDB.InfluxDBInitV1(uri, database, user, password);
        /////////////////////// NEW //////////////////////////

        InfluxDBenable = true;
    }
    else {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDB init skipped as we are missing some parameters");
    }

    return true;
}

bool ClassFlowInfluxDB::doFlow(string zwtime)
{
    if (!InfluxDBenable) {
        return true;
    }

    std::string result;
    std::string measurement;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    long int timeutc;
    string zw = "";
    string namenumber = "";

    if (flowpostprocessing) {
        std::vector<NumberPost *> *NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i) {
            measurement = (*NUMBERS)[i]->MeasurementV1;
            result = (*NUMBERS)[i]->ReturnValue;
            resultraw = (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;
            timeutc = (*NUMBERS)[i]->timeStampTimeUTC;

            if ((*NUMBERS)[i]->FieldV1.length() > 0) {
                namenumber = (*NUMBERS)[i]->FieldV1;
            }
            else {
                namenumber = (*NUMBERS)[i]->name;
                if (namenumber == "default") {
                    namenumber = "value";
                }
                else {
                    namenumber = namenumber + "/value";
                }
            }

            if (result.length() > 0) {
                //////////////////////// NEW //////////////////////////
                //                InfluxDBPublish(measurement, namenumber, result, timeutc);
                influxDB.InfluxDBPublish(measurement, namenumber, result, timeutc);
            }
            //////////////////////// NEW //////////////////////////
        }
    }

    OldValue = result;

    return true;
}

void ClassFlowInfluxDB::handleMeasurement(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
    //    ESP_LOGD(TAG, "Name: %s, Pospunkt: %d", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1) {
        _digit = _decsep.substr(0, _pospunkt);
    }
    else {
        _digit = "default";
    }
    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j) {
        if (_digit == "default") //  Set to default first (if nothing else is set)
        {
            flowpostprocessing->NUMBERS[j]->MeasurementV1 = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit) {
            flowpostprocessing->NUMBERS[j]->MeasurementV1 = _value;
        }
    }
}


void ClassFlowInfluxDB::handleFieldname(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
    //    ESP_LOGD(TAG, "Name: %s, Pospunkt: %d", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1) {
        _digit = _decsep.substr(0, _pospunkt);
    }
    else {
        _digit = "default";
    }
    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j) {
        if (_digit == "default") //  Set to default first (if nothing else is set)
        {
            flowpostprocessing->NUMBERS[j]->FieldV1 = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit) {
            flowpostprocessing->NUMBERS[j]->FieldV1 = _value;
        }
    }
}


#endif // ENABLE_INFLUXDB
