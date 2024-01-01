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

static const char* TAG = "INFLUXDB";

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

ClassFlowInfluxDB::ClassFlowInfluxDB(std::vector<ClassFlow*>* lfc)
{
    SetInitialParameter();

    ListFlowControll = lfc;
    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }
    }
}

ClassFlowInfluxDB::ClassFlowInfluxDB(std::vector<ClassFlow*>* lfc, ClassFlow *_prev)
{
    SetInitialParameter();

    previousElement = _prev;
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }
    }
}


bool ClassFlowInfluxDB::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[INFLUXDB]") != 0) 
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        ESP_LOGD(TAG, "while loop reading line: %s", aktparamgraph.c_str());
        splitted = ZerlegeZeile(aktparamgraph);
        std::string _param = GetParameterName(splitted[0]);

        if ((toUpper(_param) == "USER") && (splitted.size() > 1))
        {
            this->user = splitted[1];
        }  
        if ((toUpper(_param) == "PASSWORD") && (splitted.size() > 1))
        {
            this->password = splitted[1];
        }               
        if ((toUpper(_param) == "URI") && (splitted.size() > 1))
        {
            this->uri = splitted[1];
        }
        if (((toUpper(_param) == "DATABASE")) && (splitted.size() > 1))
        {
            this->database = splitted[1];
        }
        if (((toUpper(_param) == "MEASUREMENT")) && (splitted.size() > 1))
        {
            handleMeasurement(splitted[0], splitted[1]);
        }
        if (((toUpper(_param) == "FIELD")) && (splitted.size() > 1))
        {
            handleFieldname(splitted[0], splitted[1]);
        }
    }

    if ((uri.length() > 0) && (database.length() > 0)) 
    { 
//        ESP_LOGD(TAG, "Init InfluxDB with uri: %s, measurement: %s, user: %s, password: %s", uri.c_str(), measurement.c_str(), user.c_str(), password.c_str());
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init InfluxDB with uri: " + uri + ", user: " + user + ", password: " + password);
        InfluxDBInit(uri, database, user, password); 
        InfluxDBenable = true;
    } else {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDB init skipped as we are missing some parameters");
    }
   
    return true;
}

bool ClassFlowInfluxDB::doFlow(string zwtime)
{
    if (!InfluxDBenable)
        return true;

    std::string result;
    std::string measurement;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    long int timeutc;
    string zw = "";
    string namenumber = "";

    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            measurement = (*NUMBERS)[i]->MeasurementV1;
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
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
                    namenumber = "value";
                else
                    namenumber = namenumber + "/value";
            }

            if (result.length() > 0)   
                InfluxDBPublish(measurement, namenumber, result, timeutc);
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
    if (_pospunkt > -1)
        _digit = _decsep.substr(0, _pospunkt);
    else
        _digit = "default";
    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j)
    {
        if (_digit == "default")                        //  Set to default first (if nothing else is set)
        {
            flowpostprocessing->NUMBERS[j]->MeasurementV1 = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit)
        {
            flowpostprocessing->NUMBERS[j]->MeasurementV1 = _value;
        }
    }
}


void ClassFlowInfluxDB::handleFieldname(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
//    ESP_LOGD(TAG, "Name: %s, Pospunkt: %d", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1)
        _digit = _decsep.substr(0, _pospunkt);
    else
        _digit = "default";
    for (int j = 0; j < flowpostprocessing->NUMBERS.size(); ++j)
    {
        if (_digit == "default")                        //  Set to default first (if nothing else is set)
        {
            flowpostprocessing->NUMBERS[j]->FieldV1 = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit)
        {
            flowpostprocessing->NUMBERS[j]->FieldV1 = _value;
        }
    }
}


#endif //ENABLE_INFLUXDB