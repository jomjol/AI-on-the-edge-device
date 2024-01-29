#ifdef ENABLE_INFLUXDB
#include <sstream>
#include "ClassFlowInfluxDBv2.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_influxdb.h"

#include "ClassFlowPostProcessing.h"
#include "esp_log.h"
#include "../../include/defines.h"

#include "ClassLogFile.h"

#include <time.h>

static const char* TAG = "INFLUXDBV2";

void ClassFlowInfluxDBv2::SetInitialParameter(void)
{
    uri = "";
    bucket = "";
    dborg = "";  
    dbtoken = "";  
//    dbfield = "";

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

ClassFlowInfluxDBv2::ClassFlowInfluxDBv2(std::vector<ClassFlow*>* lfc)
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

ClassFlowInfluxDBv2::ClassFlowInfluxDBv2(std::vector<ClassFlow*>* lfc, ClassFlow *_prev)
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


bool ClassFlowInfluxDBv2::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);
    printf("akt param: %s\n", aktparamgraph.c_str());

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[INFLUXDBV2]") != 0) 
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
//        ESP_LOGD(TAG, "while loop reading line: %s", aktparamgraph.c_str());
        splitted = ZerlegeZeile(aktparamgraph);
        std::string _param = GetParameterName(splitted[0]);

        if ((toUpper(_param) == "ORG") && (splitted.size() > 1))
        {
            this->dborg = splitted[1];
        }  
        if ((toUpper(_param) == "TOKEN") && (splitted.size() > 1))
        {
            this->dbtoken = splitted[1];
        }               
        if ((toUpper(_param) == "URI") && (splitted.size() > 1))
        {
            this->uri = splitted[1];
        }
        if (((toUpper(_param) == "FIELD")) && (splitted.size() > 1))
        {
            handleFieldname(splitted[0], splitted[1]);
        }
        if (((toUpper(_param) == "MEASUREMENT")) && (splitted.size() > 1))
        {
            handleMeasurement(splitted[0], splitted[1]);
        }
        if (((toUpper(splitted[0]) == "BUCKET")) && (splitted.size() > 1))
        {
            this->bucket = splitted[1];
        }
    }

    printf("uri:         %s\n", uri.c_str());
    printf("org:         %s\n", dborg.c_str());
    printf("token:       %s\n", dbtoken.c_str());

    if ((uri.length() > 0) && (bucket.length() > 0) && (dbtoken.length() > 0) && (dborg.length() > 0)) 
    { 
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init InfluxDB with uri: " + uri + ", org: " + dborg + ", token: *****");
//        printf("vor V2 Init\n");
        InfluxDB_V2_Init(uri, bucket, dborg, dbtoken); 
//        printf("nach V2 Init\n");
        InfluxDBenable = true;
    } else {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDBv2 (Verion2 !!!) init skipped as we are missing some parameters");
    }
   
    return true;
}

/*
string ClassFlowInfluxDBv2::GetInfluxDBMeasurement()
{
    return measurement;
}
*/

void ClassFlowInfluxDBv2::handleFieldname(string _decsep, string _value)
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
            flowpostprocessing->NUMBERS[j]->FieldV2 = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit)
        {
            flowpostprocessing->NUMBERS[j]->FieldV2 = _value;
        }
    }
}

void ClassFlowInfluxDBv2::handleMeasurement(string _decsep, string _value)
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
            flowpostprocessing->NUMBERS[j]->MeasurementV2 = _value;
        }
        if (flowpostprocessing->NUMBERS[j]->name == _digit)
        {
            flowpostprocessing->NUMBERS[j]->MeasurementV2 = _value;
        }
    }
}


bool ClassFlowInfluxDBv2::doFlow(string zwtime)
{
    if (!InfluxDBenable)
        return true;

    std::string measurement;
    std::string result;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    long int resulttimeutc = 0;
    string zw = "";
    string namenumber = "";


    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            measurement = (*NUMBERS)[i]->MeasurementV2;
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
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
                    namenumber = "value";
                else
                    namenumber = namenumber + "/value";
            }
            
            printf("vor sende Influx_DB_V2 - namenumber. %s, result: %s, timestampt: %s", namenumber.c_str(), result.c_str(), resulttimestamp.c_str());

            if (result.length() > 0)   
                InfluxDB_V2_Publish(measurement, namenumber, result, resulttimeutc);
        }
    }
   
    OldValue = result;
    
    return true;
}

#endif //ENABLE_INFLUXDB