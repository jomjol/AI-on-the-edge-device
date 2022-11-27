#include "ClassFlowControll.h"

#include "connect_wlan.h"
#include "read_wlanini.h"

#include "freertos/task.h"

#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <dirent.h>
#ifdef __cplusplus
}
#endif

#include "ClassLogFile.h"
#include "time_sntp.h"
#include "Helper.h"
#include "server_ota.h"


//#include "CImg.h"

#include "server_help.h"

//#define DEBUG_DETAIL_ON  

static const char* TAG = "FLOW CTRL";



std::string ClassFlowControll::doSingleStep(std::string _stepname, std::string _host){
    std::string _classname = "";
    std::string result = "";

    ESP_LOGD(TAG, "Step %s start", _stepname.c_str());

    if ((_stepname.compare("[MakeImage]") == 0) || (_stepname.compare(";[MakeImage]") == 0)){
        _classname = "ClassFlowMakeImage";
    }
    if ((_stepname.compare("[Alignment]") == 0) || (_stepname.compare(";[Alignment]") == 0)){
        _classname = "ClassFlowAlignment";
    }
    if ((_stepname.compare(0, 7, "[Digits") == 0) || (_stepname.compare(0, 8, ";[Digits") == 0)) {
        _classname = "ClassFlowCNNGeneral";
    }
    if ((_stepname.compare("[Analog]") == 0) || (_stepname.compare(";[Analog]") == 0)){
        _classname = "ClassFlowCNNGeneral";
    }
    if ((_stepname.compare("[MQTT]") == 0) || (_stepname.compare(";[MQTT]") == 0)){
        _classname = "ClassFlowMQTT";
    }
    if ((_stepname.compare("[InfluxDB]") == 0) || (_stepname.compare(";[InfluxDB]") == 0)){
        _classname = "ClassFlowInfluxDB";
    }

    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare(_classname) == 0){
            if (!(FlowControll[i]->name().compare("ClassFlowMakeImage") == 0))      // falls es ein MakeImage ist, braucht das Bild nicht extra aufgenommen zu werden, dass passiert bei html-Abfrage automatisch
                FlowControll[i]->doFlow("");
            result = FlowControll[i]->getHTMLSingleStep(_host);
        }

    ESP_LOGD(TAG, "Step %s end", _stepname.c_str());

    return result;
}


std::string ClassFlowControll::TranslateAktstatus(std::string _input)
{
    if (_input.compare("ClassFlowMakeImage") == 0)
        return ("Take Image");
    if (_input.compare("ClassFlowAlignment") == 0)
        return ("Aligning");
    if (_input.compare("ClassFlowCNNGeneral") == 0)
        return ("Digitalization of ROIs");
    if (_input.compare("ClassFlowMQTT") == 0)
        return ("Sending MQTT");
    if (_input.compare("ClassFlowInfluxDB") == 0)
        return ("Sending InfluxDB");
    if (_input.compare("ClassFlowPostProcessing") == 0)
        return ("Processing");
    if (_input.compare("ClassFlowWriteList") == 0)
        return ("Processing");

    return "Unkown Status";
}


std::vector<HTMLInfo*> ClassFlowControll::GetAllDigital() 
{
    if (flowdigit)
    {
        ESP_LOGD(TAG, "ClassFlowControll::GetAllDigital - flowdigit != NULL");
        return flowdigit->GetHTMLInfo();
    }

    std::vector<HTMLInfo*> empty;
    return empty;
}

std::vector<HTMLInfo*> ClassFlowControll::GetAllAnalog()
{
    if (flowanalog)
        return flowanalog->GetHTMLInfo();

    std::vector<HTMLInfo*> empty;
    return empty;
}

t_CNNType ClassFlowControll::GetTypeDigital()
{
    if (flowdigit)
        return flowdigit->getCNNType();

    return t_CNNType::None;
}

t_CNNType ClassFlowControll::GetTypeAnalog()
{
    if (flowanalog)
        return flowanalog->getCNNType();

    return t_CNNType::None;
}




string ClassFlowControll::GetMQTTMainTopic()
{
    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare("ClassFlowMQTT") == 0)
            return ((ClassFlowMQTT*) (FlowControll[i]))->GetMQTTMainTopic();

    return "";
}



void ClassFlowControll::SetInitialParameter(void)
{
    AutoStart = false;
    SetupModeActive = false;
    AutoIntervall = 10; // Minutes
    flowdigit = NULL;
    flowanalog = NULL;
    flowpostprocessing = NULL;
    disabled = false;
    aktRunNr = 0;
    aktstatus = "Booting ...";
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
    {
        cfc = new ClassFlowMakeImage(&FlowControll);
        flowmakeimage = (ClassFlowMakeImage*) cfc;
    }
    if (toUpper(_type).compare("[ALIGNMENT]") == 0)
    {
        cfc = new ClassFlowAlignment(&FlowControll);
        flowalignment = (ClassFlowAlignment*) cfc;
    }
    if (toUpper(_type).compare("[ANALOG]") == 0)
    {
        cfc = new ClassFlowCNNGeneral(flowalignment);
        flowanalog = (ClassFlowCNNGeneral*) cfc;
    }
    if (toUpper(_type).compare(0, 7, "[DIGITS") == 0)
    {
        cfc = new ClassFlowCNNGeneral(flowalignment);
        flowdigit = (ClassFlowCNNGeneral*) cfc;
    }
    if (toUpper(_type).compare("[MQTT]") == 0)
        cfc = new ClassFlowMQTT(&FlowControll);

    if (toUpper(_type).compare("[INFLUXDB]") == 0)
        cfc = new ClassFlowInfluxDB(&FlowControll);
        
    if (toUpper(_type).compare("[WRITELIST]") == 0)
        cfc = new ClassFlowWriteList(&FlowControll);

    if (toUpper(_type).compare("[POSTPROCESSING]") == 0)
    {
        cfc = new ClassFlowPostProcessing(&FlowControll, flowanalog, flowdigit); 
        flowpostprocessing = (ClassFlowPostProcessing*) cfc;
    }

    if (cfc)                            // Wird nur angehangen, falls es nicht [AutoTimer] ist, denn dieses ist für FlowControll
        FlowControll.push_back(cfc);

    if (toUpper(_type).compare("[AUTOTIMER]") == 0)
        cfc = this;    

    if (toUpper(_type).compare("[DATALOGGING]") == 0)
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
    pFile = OpenFileAndWait(config.c_str(), "r");

    line = "";

    char zw[1024];
    if (pFile != NULL)
    {
        fgets(zw, 1024, pFile);
        ESP_LOGD(TAG, "%s", zw);
        line = std::string(zw);
    }

    while ((line.size() > 0) && !(feof(pFile)))
    {
        cfc = CreateClassFlow(line);
        if (cfc)
        {
            ESP_LOGD(TAG, "Start ReadParameter (%s)", line.c_str());
            cfc->ReadParameter(pFile, line);
        }
        else
        {
            line = "";
            if (fgets(zw, 1024, pFile) && !feof(pFile))
                {
                    ESP_LOGD(TAG, "Read: %s", zw);
                    line = std::string(zw);
                }
        }
    }

    fclose(pFile);

}

std::string* ClassFlowControll::getActStatus(){
    return &aktstatus;
}

void ClassFlowControll::doFlowMakeImageOnly(string time){
    std::string zw_time;

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        if (FlowControll[i]->name() == "ClassFlowMakeImage") {
//            zw_time = gettimestring("%Y%m%d-%H%M%S");
            zw_time = gettimestring("%H:%M:%S");
            aktstatus = TranslateAktstatus(FlowControll[i]->name()) + " (" + zw_time + ")";
            FlowControll[i]->doFlow(time);
        }
    }
}

bool ClassFlowControll::doFlow(string time)
{
//    CleanTempFolder();            // dazu muss man noch eine Rolling einführen

    bool result = true;
    std::string zw_time;
    int repeat = 0;

#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("ClassFlowControll::doFlow - Start");
#endif

    /* Check if we have a valid date/time and if not restart the NTP client */
    if (! getTimeIsSet()) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Time not set, restarting NTP Client!");
        restartNtpClient();
    }

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        zw_time = gettimestring("%H:%M:%S");
        aktstatus = TranslateAktstatus(FlowControll[i]->name()) + " (" + zw_time + ")";

//        zw_time = gettimestring("%Y%m%d-%H%M%S");
//        aktstatus = zw_time + ": " + FlowControll[i]->name();
        
       
        string zw = "FlowControll.doFlow - " + FlowControll[i]->name();
        LogFile.WriteHeapInfo(zw);

        if (!FlowControll[i]->doFlow(time)){
            repeat++;
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Fehler im vorheriger Schritt - wird zum " + to_string(repeat) + ". Mal wiederholt");
            if (i) i -= 1;    // vorheriger Schritt muss wiederholt werden (vermutlich Bilder aufnehmen)
            result = false;
            if (repeat > 5) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Wiederholung 5x nicht erfolgreich --> reboot");
                doReboot();
                // Schritt wurde 5x wiederholt --> reboot
            }
        }
        else
        {
            result = true;
        }
        
#ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("ClassFlowControll::doFlow");
#endif

    }
    zw_time = gettimestring("%H:%M:%S");
    aktstatus = "Flow finished (" + zw_time + ")";
    return result;
}


string ClassFlowControll::getReadoutAll(int _type)
{
    std::string out = "";
    if (flowpostprocessing)
    {
        std::vector<NumberPost*> *numbers = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*numbers).size(); ++i)
        {
            out = out + (*numbers)[i]->name + "\t";
            switch (_type) {
                case READOUT_TYPE_VALUE:
                    out = out + (*numbers)[i]->ReturnValue;
                    break;
                case READOUT_TYPE_PREVALUE:
                    if (flowpostprocessing->PreValueUse)
                    {
                        if ((*numbers)[i]->PreValueOkay)
                            out = out + (*numbers)[i]->ReturnPreValue;
                        else
                            out = out + "PreValue too old";                
                    }
                    else
                        out = out + "PreValue deactivated";
                    break;
                case READOUT_TYPE_RAWVALUE:
                    out = out + (*numbers)[i]->ReturnRawValue;
                    break;
                case READOUT_TYPE_ERROR:
                    out = out + (*numbers)[i]->ErrorMessageText;
                    break;
            }
            if (i < (*numbers).size()-1)
                out = out + "\r\n";
        }
    //    ESP_LOGD(TAG, "OUT: %s", out.c_str());
    }

    return out;
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

string ClassFlowControll::GetPrevalue(std::string _number)	
{
    if (flowpostprocessing)
    {
        return flowpostprocessing->GetPreValue(_number);   
    }

    return std::string("");    
}

std::string ClassFlowControll::UpdatePrevalue(std::string _newvalue, std::string _numbers, bool _extern)
{
    float zw;
    char* p;

    _newvalue = trim(_newvalue);
//    ESP_LOGD(TAG, "Input UpdatePreValue: %s", _newvalue.c_str());

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
        flowpostprocessing->SetPreValue(zw, _numbers, _extern);
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


    if ((toUpper(aktparamgraph).compare("[AUTOTIMER]") != 0) && (toUpper(aktparamgraph).compare("[DEBUG]") != 0) &&
        (toUpper(aktparamgraph).compare("[SYSTEM]") != 0 && (toUpper(aktparamgraph).compare("[DATALOGGING]") != 0)))      // Paragraph passt nicht zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = ZerlegeZeile(aktparamgraph, " =");
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

        if ((toUpper(zerlegt[0]) == "DATALOGACTIVE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                LogFile.SetDataLogToSD(true);
            }
            else {
                LogFile.SetDataLogToSD(false);
            }
        }

        if ((toUpper(zerlegt[0]) == "DATALOGRETENTIONINDAYS") && (zerlegt.size() > 1))
        {
            LogFile.SetDataLogRetention(std::stoi(zerlegt[1]));
        }

        if ((toUpper(zerlegt[0]) == "LOGFILE") && (zerlegt.size() > 1))
        {
            /* matches esp_log_level_t */
            if ((toUpper(zerlegt[1]) == "TRUE") || (toUpper(zerlegt[1]) == "2"))
            {
                LogFile.setLogLevel(ESP_LOG_WARN);
            }
            else if ((toUpper(zerlegt[1]) == "FALSE") || (toUpper(zerlegt[1]) == "0") || (toUpper(zerlegt[1]) == "1"))
            {
                LogFile.setLogLevel(ESP_LOG_ERROR);
            }
            else if (toUpper(zerlegt[1]) == "3")
            {
                LogFile.setLogLevel(ESP_LOG_INFO);
            }
            else if (toUpper(zerlegt[1]) == "4")
            {
                LogFile.setLogLevel(ESP_LOG_DEBUG);
            }
        }
        if ((toUpper(zerlegt[0]) == "LOGFILERETENTIONINDAYS") && (zerlegt.size() > 1))
        {
            LogFile.SetLogFileRetention(std::stoi(zerlegt[1]));
        }

        if ((toUpper(zerlegt[0]) == "TIMEZONE") && (zerlegt.size() > 1))
        {
            string zw = "Set TimeZone: " + zerlegt[1];
            setTimeZone(zerlegt[1]);
        }      

        if ((toUpper(zerlegt[0]) == "TIMESERVER") && (zerlegt.size() > 1))
        {
            string zw = "Set TimeZone: " + zerlegt[1];
            reset_servername(zerlegt[1]);
        }  

        if ((toUpper(zerlegt[0]) == "HOSTNAME") && (zerlegt.size() > 1))
        {
            if (ChangeHostName("/sdcard/wlan.ini", zerlegt[1]))
            {
                // reboot notwendig damit die neue wlan.ini auch benutzt wird !!!
                fclose(pfile);
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Rebooting to activate new HOSTNAME...");
                esp_restart();
                hard_restart();                   
                doReboot();
            }
        }

        if ((toUpper(zerlegt[0]) == "SETUPMODE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                SetupModeActive = true;
            }        
        }
    }

    /* Start the MQTT service */
    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare("ClassFlowMQTT") == 0)
            return ((ClassFlowMQTT*) (FlowControll[i]))->Start(AutoIntervall);

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


esp_err_t ClassFlowControll::SendRawJPG(httpd_req_t *req)
{
    return flowmakeimage != NULL ? flowmakeimage->SendRawJPG(req) : ESP_FAIL;
}


esp_err_t ClassFlowControll::GetJPGStream(std::string _fn, httpd_req_t *req)
{
    ESP_LOGD(TAG, "ClassFlowControll::GetJPGStream %s", _fn.c_str());

    CImageBasis *_send = NULL;
    esp_err_t result = ESP_FAIL;
    bool Dodelete = false;    

    if (flowalignment == NULL)
    {
        ESP_LOGD(TAG, "Can't continue, flowalignment is NULL");
        return ESP_FAIL;
    }

    if (_fn == "alg.jpg")
    {
        _send = flowalignment->ImageBasis;  
    }
    else
    {
        if (_fn == "alg_roi.jpg")
        {
            CImageBasis* _imgzw = new CImageBasis(flowalignment->ImageBasis);
            flowalignment->DrawRef(_imgzw);
            if (flowdigit) flowdigit->DrawROI(_imgzw);
            if (flowanalog) flowanalog->DrawROI(_imgzw);
            _send = _imgzw;
            Dodelete = true;
        }
        else
        {
            std::vector<HTMLInfo*> htmlinfo;
            htmlinfo = GetAllDigital();
            ESP_LOGD(TAG, "After getClassFlowControll::GetAllDigital");

            for (int i = 0; i < htmlinfo.size(); ++i)
            {
                if (_fn == htmlinfo[i]->filename)
                {
                    if (htmlinfo[i]->image)
                        _send = htmlinfo[i]->image;
                }
                if (_fn == htmlinfo[i]->filename_org)
                {
                    if (htmlinfo[i]->image_org)
                        _send = htmlinfo[i]->image_org;        
                }
                delete htmlinfo[i];
            }
            htmlinfo.clear();

            if (!_send)
            {
                htmlinfo = GetAllAnalog();
                for (int i = 0; i < htmlinfo.size(); ++i)
                {
                    if (_fn == htmlinfo[i]->filename)
                    {
                        if (htmlinfo[i]->image)
                            _send = htmlinfo[i]->image;
                    }
                    if (_fn == htmlinfo[i]->filename_org)
                    {
                        if (htmlinfo[i]->image_org)
                            _send = htmlinfo[i]->image_org;        
                    }
                    delete htmlinfo[i];
                }
                htmlinfo.clear();

            }
        }
    }

    if (_send)
    {
        ESP_LOGI(TAG, "Sending file : %s ...", _fn.c_str());
        set_content_type_from_file(req, _fn.c_str());
        result = _send->SendJPGtoHTTP(req);
        ESP_LOGI(TAG, "File sending complete");    
        /* Respond with an empty chunk to signal HTTP response completion */
        httpd_resp_send_chunk(req, NULL, 0);
    }

    if (Dodelete) 
    {
        delete _send;
    }

    return result;
}


string ClassFlowControll::getNumbersName()
{
    return flowpostprocessing->getNumbersName();
}

string ClassFlowControll::getJSON(std::string _id, std::string _mac)
{
    return flowpostprocessing->GetJSON(_id, _mac);
}
