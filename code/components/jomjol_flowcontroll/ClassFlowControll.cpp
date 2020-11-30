#include "ClassFlowControll.h"

#include "freertos/task.h"

#include <sys/stat.h>
#include <dirent.h>
#include "ClassLogFile.h"
#include "time_sntp.h"
#include "Helper.h"
#include "server_ota.h"

static const char* TAG = "flow_controll";

std::string ClassFlowControll::doSingleStep(std::string _stepname, std::string _host){
    std::string _classname = "";
    std::string result = "";
    if (_stepname.compare("[MakeImage]") == 0){
        _classname = "ClassFlowMakeImage";
    }
    if (_stepname.compare("[Alignment]") == 0){
        _classname = "ClassFlowAlignment";
    }
    if (_stepname.compare("[Digits]") == 0){
        _classname = "ClassFlowDigit";
    }
    if (_stepname.compare("[Analog]") == 0){
        _classname = "ClassFlowAnalog";
    }
    if (_stepname.compare("[MQTT]") == 0){
        _classname = "ClassFlowMQTT";
    }
//    std::string zw = "Classname: " + _classname + "\n";
//    printf(zw.c_str());

    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare(_classname) == 0){
 //           printf(FlowControll[i]->name().c_str()); printf("\n");
            FlowControll[i]->doFlow("");
            result = FlowControll[i]->getHTMLSingleStep(_host);
        }

    return result;
}

std::vector<HTMLInfo*> ClassFlowControll::GetAllDigital()
{
    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare("ClassFlowDigit") == 0)
            return ((ClassFlowDigit*) (FlowControll[i]))->GetHTMLInfo();

    std::vector<HTMLInfo*> empty;
    return empty;
}

std::vector<HTMLInfo*> ClassFlowControll::GetAllAnalog()
{
    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare("ClassFlowAnalog") == 0)
            return ((ClassFlowAnalog*) (FlowControll[i]))->GetHTMLInfo();

    std::vector<HTMLInfo*> empty;
    return empty;
}


void ClassFlowControll::SetInitialParameter(void)
{
    AutoStart = false;
    AutoIntervall = 10;
}

bool ClassFlowControll::isAutoStart(long &_intervall)
{
    _intervall = AutoIntervall * 60 * 1000; // AutoIntervall: Minuten -> ms
    return AutoStart;
}

ClassFlow* ClassFlowControll::CreateClassFlow(std::string _type)
{
    ClassFlow* cfc = NULL;

    _type = trim(_type);

    if (toUpper(_type).compare("[MAKEIMAGE]") == 0)
        cfc = new ClassFlowMakeImage(&FlowControll);
    if (toUpper(_type).compare("[ALIGNMENT]") == 0)
        cfc = new ClassFlowAlignment(&FlowControll);
    if (toUpper(_type).compare("[ANALOG]") == 0)
        cfc = new ClassFlowAnalog(&FlowControll);
    if (toUpper(_type).compare("[DIGITS]") == 0)
        cfc = new ClassFlowDigit(&FlowControll);
    if (toUpper(_type).compare("[MQTT]") == 0)
        cfc = new ClassFlowMQTT(&FlowControll);
    if (toUpper(_type).compare("[POSTPROCESSING]") == 0)
    {
        cfc = new ClassFlowPostProcessing(&FlowControll); 
        flowpostprocessing = (ClassFlowPostProcessing*) cfc;
    }

    if (cfc)                            // Wird nur angehangen, falls es nicht [AutoTimer] ist, denn dieses ist für FlowControll
        FlowControll.push_back(cfc);

    if (toUpper(_type).compare("[AUTOTIMER]") == 0)
        cfc = this;    

    if (toUpper(_type).compare("[DEBUG]") == 0)
        cfc = this;  

    if (toUpper(_type).compare("[SYSTEM]") == 0)
        cfc = this;          

    return cfc;
}

void ClassFlowControll::InitFlow(std::string config)
{
    string line;

    flowpostprocessing = NULL;

    ClassFlow* cfc;
    FILE* pFile;
    config = FormatFileName(config);
    pFile = fopen(config.c_str(), "r");

    line = "";

    char zw[1024];
    if (pFile != NULL)
    {
        fgets(zw, 1024, pFile);
        printf("%s", zw);
        line = std::string(zw);
    }

    while ((line.size() > 0) && !(feof(pFile)))
    {
        cfc = CreateClassFlow(line);
        if (cfc)
        {
            cfc->ReadParameter(pFile, line);
        }
        else
        {
            fgets(zw, 1024, pFile);
            printf("%s", zw);
            line = std::string(zw);
        }
    }

    fclose(pFile);

}

std::string ClassFlowControll::getActStatus(){
    return aktstatus;
}

bool ClassFlowControll::doFlow(string time)
{
//    CleanTempFolder();            // dazu muss man noch eine Rolling einführen

    bool result = true;
    std::string zw_time;
    int repeat = 0;

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        zw_time = gettimestring("%Y%m%d-%H%M%S");
        aktstatus = zw_time + ": " + FlowControll[i]->name();
        string zw = "FlowControll.doFlow - " + FlowControll[i]->name();
        LogFile.WriteToFile(zw);
        if (!FlowControll[i]->doFlow(time)){
            repeat++;
            LogFile.WriteToFile("Fehler im vorheriger Schritt - wird zum " + to_string(repeat) + ". Mal wiederholt");
            i = -1;    // vorheriger Schritt muss wiederholt werden (vermutlich Bilder aufnehmen)
            result = false;
            if (repeat > 5) {
                LogFile.WriteToFile("Wiederholung 5x nicht erfolgreich --> reboot");
                doReboot();
                // Schritt wurde 5x wiederholt --> reboot
            }
        }
        else
        {
            result = true;
        }
    }
    zw_time = gettimestring("%Y%m%d-%H%M%S");    
    aktstatus = zw_time + ": Flow is done";
    return result;
}

string ClassFlowControll::getReadout(bool _rawvalue = false, bool _noerror = false)
{
    if (flowpostprocessing)
        return flowpostprocessing->getReadoutParam(_rawvalue, _noerror);

    string zw = "";
    string result = "";

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        zw = FlowControll[i]->getReadout();
        if (zw.length() > 0)
        {
            if (result.length() == 0)
                result = zw;
            else
                result = result + "\t" + zw;
        }
    }

    return result;
}

string ClassFlowControll::GetPrevalue()	
{
    if (flowpostprocessing)
    {
        return flowpostprocessing->GetPreValue();   
    }

    return std::string();    
}

std::string ClassFlowControll::UpdatePrevalue(std::string _newvalue)
{
    float zw;
    char* p;

    _newvalue = trim(_newvalue);
//    printf("Input UpdatePreValue: %s\n", _newvalue.c_str());

    if (_newvalue.compare("0.0") == 0)
    {
        zw = 0;
    }
    else
    {
        zw = strtof(_newvalue.c_str(), &p);
        if (zw == 0)
            return "- Error in String to Value Conversion!!! Must be of format value=123.456";
    }
    

    if (flowpostprocessing)
    {
        flowpostprocessing->SavePreValue(zw);
        return _newvalue;    
    }

    return std::string();
}

bool ClassFlowControll::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if ((toUpper(aktparamgraph).compare("[AUTOTIMER]") != 0) && (toUpper(aktparamgraph).compare("[DEBUG]") != 0) && (toUpper(aktparamgraph).compare("[SYSTEM]") != 0))      // Paragraph passt nicht zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph, " =");
        if ((toUpper(zerlegt[0]) == "AUTOSTART") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                AutoStart = true;
            }
        }
        if ((toUpper(zerlegt[0]) == "INTERVALL") && (zerlegt.size() > 1))
        {
            AutoIntervall = std::stof(zerlegt[1]);
        }
        if ((toUpper(zerlegt[0]) == "LOGFILE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                LogFile.SwitchOnOff(true);
            }
            if (toUpper(zerlegt[1]) == "FALSE")
            {
                LogFile.SwitchOnOff(false);
            }
        }
        if ((toUpper(zerlegt[0]) == "LOGFILERETENTIONINDAYS") && (zerlegt.size() > 1))
        {
            LogFile.SetRetention(std::stoi(zerlegt[1]));
        }      

        if ((toUpper(zerlegt[0]) == "TIMEZONE") && (zerlegt.size() > 1))
        {
            string zw = "Set TimeZone: " + zerlegt[1];
            setTimeZone(zerlegt[1]);
        }      

        if ((toUpper(zerlegt[0]) == "TIMEUPDATEINTERVALL") && (zerlegt.size() > 1))
        {
            TimeUpdateIntervall = stof(zerlegt[1]);
            xTaskCreate(&task_doTimeSync, "update_time", configMINIMAL_STACK_SIZE * 16, &TimeUpdateIntervall, tskIDLE_PRIORITY, NULL);
        }      

    }
    return true;
}

int ClassFlowControll::CleanTempFolder() {
    const char* folderPath = "/sdcard/img_tmp";
    
    ESP_LOGI(TAG, "Clean up temporary folder to avoid damage of sdcard sectors : %s", folderPath);
    DIR *dir = opendir(folderPath);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", folderPath);
        return -1;
    }

    struct dirent *entry;
    int deleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        std::string path = string(folderPath) + "/" + entry->d_name;
		if (entry->d_type == DT_REG) {
			if (unlink(path.c_str()) == 0) {
				deleted ++;
			} else {
				ESP_LOGE(TAG, "can't delete file : %s", path.c_str());
			}
        } else if (entry->d_type == DT_DIR) {
			deleted += removeFolder(path.c_str(), TAG);
		}
    }
    closedir(dir);
    ESP_LOGI(TAG, "%d files deleted", deleted);
    
    return 0;
}