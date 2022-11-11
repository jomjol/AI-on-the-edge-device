#include <sstream>
#include "ClassFlowInfluxDB.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_influxdb.h"
#include "ClassFlowPostProcessing.h"
#include "esp_log.h"

#include <time.h>

static const char* TAG = "class_flow_influxDb";

void ClassFlowInfluxDB::SetInitialParameter(void)
{
    uri = "";
    database = "";
    measurement = "";

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
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[INFLUXDB]") != 0) 
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        ESP_LOGD(TAG, "while loop reading line: %s", aktparamgraph.c_str());
        zerlegt = ZerlegeZeile(aktparamgraph);
        if ((toUpper(zerlegt[0]) == "USER") && (zerlegt.size() > 1))
        {
            this->user = zerlegt[1];
        }  
        if ((toUpper(zerlegt[0]) == "PASSWORD") && (zerlegt.size() > 1))
        {
            this->password = zerlegt[1];
        }               
        if ((toUpper(zerlegt[0]) == "URI") && (zerlegt.size() > 1))
        {
            this->uri = zerlegt[1];
        }
        if (((toUpper(zerlegt[0]) == "MEASUREMENT")) && (zerlegt.size() > 1))
        {
            this->measurement = zerlegt[1];
        }
        if (((toUpper(zerlegt[0]) == "DATABASE")) && (zerlegt.size() > 1))
        {
            this->database = zerlegt[1];
        }
    }

    if ((uri.length() > 0) && (database.length() > 0) && (measurement.length() > 0)) 
    { 
        ESP_LOGD(TAG, "Init InfluxDB with uri: %s, measurement: %s, user: %s, password: %s", uri.c_str(), measurement.c_str(), user.c_str(), password.c_str());
        InfluxDBInit(uri, database, measurement, user, password); 
        InfluxDBenable = true;
    } else {
        ESP_LOGD(TAG, "InfluxDB init skipped as we are missing some parameters");
    }
   
    return true;
}


string ClassFlowInfluxDB::GetInfluxDBMeasurement()
{
    return measurement;
}


bool ClassFlowInfluxDB::doFlow(string zwtime)
{
    if (!InfluxDBenable)
        return true;

    std::string result;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    string zw = "";
    string namenumber = "";

    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;

            namenumber = (*NUMBERS)[i]->name;
            if (namenumber == "default")
                namenumber = "value";
            else
                namenumber = namenumber + "/value";

            if (result.length() > 0 && resulttimestamp.length() > 0)   
                InfluxDBPublish(namenumber, result, resulttimestamp);
        }
    }
   
    OldValue = result;
    
    return true;
}
