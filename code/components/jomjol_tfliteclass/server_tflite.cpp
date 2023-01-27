#include "server_tflite.h"

#include <string>
#include <vector>
#include "string.h"
#include "esp_log.h"

#include <iomanip>
#include <sstream>

#include "../../include/defines.h"
#include "Helper.h"

#include "esp_camera.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"

#include "ClassFlowControll.h"

#include "ClassLogFile.h"
#include "server_GPIO.h"

#include "server_file.h"
#include "connect_wlan.h"

ClassFlowControll tfliteflow;

TaskHandle_t xHandletask_autodoFlow = NULL;

bool bTaskAutoFlowCreated = false;
bool flowisrunning = false;

long auto_intervall = 0;
bool auto_isrunning = false;

int countRounds = 0;
bool isPlannedReboot = false;

static const char *TAG = "TFLITE SERVER";

//#define DEBUG_DETAIL_ON


void CheckIsPlannedReboot()
{
 	FILE *pfile;
    if ((pfile = fopen("/sdcard/reboot.txt", "r")) == NULL)
    {
		LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Not a planned reboot.");
        isPlannedReboot = false;
	}
    else
    {
		LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Planned reboot.");
        DeleteFile("/sdcard/reboot.txt");   // Prevent Boot Loop!!!
        isPlannedReboot = true;
	}
}


int getCountFlowRounds() 
{
    return countRounds;
}


esp_err_t GetJPG(std::string _filename, httpd_req_t *req)
{
    return tfliteflow.GetJPGStream(_filename, req);
}


esp_err_t GetRawJPG(httpd_req_t *req)
{
    return tfliteflow.SendRawJPG(req);
}


bool isSetupModusActive() {
    return tfliteflow.getStatusSetupModus();
    return false;
}


void KillTFliteTasks()
{
    #ifdef DEBUG_DETAIL_ON      
        ESP_LOGD(TAG, "KillTFliteTasks: xHandletask_autodoFlow: %ld", (long) xHandletask_autodoFlow);
    #endif
    if( xHandletask_autodoFlow != NULL )
    {
        vTaskDelete(xHandletask_autodoFlow);
        xHandletask_autodoFlow = NULL;
    }
    #ifdef DEBUG_DETAIL_ON      
    	ESP_LOGD(TAG, "Killed: xHandletask_autodoFlow");
    #endif
}


void doInit(void)
{
    #ifdef DEBUG_DETAIL_ON             
        ESP_LOGD(TAG, "Start tfliteflow.InitFlow(config);");
    #endif
    tfliteflow.InitFlow(CONFIG_FILE);
    #ifdef DEBUG_DETAIL_ON      
        ESP_LOGD(TAG, "Finished tfliteflow.InitFlow(config);");
    #endif
    
    /* GPIO handler has to be initialized before MQTT init to ensure proper topic subscription */
    gpio_handler_init();

    #ifdef ENABLE_MQTT
        tfliteflow.StartMQTTService();
    #endif //ENABLE_MQTT
}


bool doflow(void)
{   
    std::string zw_time = getCurrentTimeString(LOGFILE_TIME_FORMAT);
    ESP_LOGD(TAG, "doflow - start %s", zw_time.c_str());
    flowisrunning = true;
    tfliteflow.doFlow(zw_time);
    flowisrunning = false;

    #ifdef DEBUG_DETAIL_ON      
        ESP_LOGD(TAG, "doflow - end %s", zw_time.c_str());
    #endif

    return true;
}


esp_err_t handler_get_heap(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON      
        LogFile.WriteHeapInfo("handler_get_heap - Start");       
        ESP_LOGD(TAG, "handler_get_heap uri: %s", req->uri);
    #endif

    std::string zw = "Heap info:<br>" + getESPHeapInfo();

    #ifdef TASK_ANALYSIS_ON
        char* pcTaskList = (char*) heap_caps_calloc(1, sizeof(char) * 768, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
        if (pcTaskList) {
            vTaskList(pcTaskList);
            zw = zw + "<br><br>Task info:<br><pre>Name | State | Prio | Lowest stacksize | Creation order | CPU (-1=NoAffinity)<br>"
                    + std::string(pcTaskList) + "</pre>";
            heap_caps_free(pcTaskList);
        }
        else {
            zw = zw + "<br><br>Task info:<br>ERROR - Allocation of TaskList buffer in PSRAM failed";
        }
    #endif 

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (zw.length() > 0) 
    {
        httpd_resp_send(req, zw.c_str(), zw.length());
    }
    else 
    {
        httpd_resp_send(req, NULL, 0);
    }

    #ifdef DEBUG_DETAIL_ON      
        LogFile.WriteHeapInfo("handler_get_heap - Done");       
    #endif

    return ESP_OK;
}


esp_err_t handler_init(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON      
        LogFile.WriteHeapInfo("handler_init - Start");       
        ESP_LOGD(TAG, "handler_doinit uri: %s", req->uri);
    #endif

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    const char* resp_str = "Init started<br>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);     

    doInit();

    resp_str = "Init done<br>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);     

    #ifdef DEBUG_DETAIL_ON      
        LogFile.WriteHeapInfo("handler_init - Done");       
    #endif

    return ESP_OK;
}


esp_err_t handler_flow_start(httpd_req_t *req) {

    #ifdef DEBUG_DETAIL_ON          
    LogFile.WriteHeapInfo("handler_flow_start - Start");       
    #endif

    ESP_LOGD(TAG, "handler_flow_start uri: %s", req->uri);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (auto_isrunning) {
        xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state. If task is already running, no action
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by REST API /flow_start");
        const char* resp_str = "The flow is going to be started immediately or is already running";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);  
    }
    else {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Flow start triggered by REST API, but flow is not active!");
        const char* resp_str = "WARNING: Flow start triggered by REST API, but flow is not active";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);  
    }

    #ifdef DEBUG_DETAIL_ON   
        LogFile.WriteHeapInfo("handler_flow_start - Done");       
    #endif

    return ESP_OK;
}


#ifdef ENABLE_MQTT
esp_err_t MQTTCtrlFlowStart(std::string _topic) {

    #ifdef DEBUG_DETAIL_ON          
        LogFile.WriteHeapInfo("MQTTCtrlFlowStart - Start");       
    #endif

    ESP_LOGD(TAG, "MQTTCtrlFlowStart: topic %s", _topic.c_str());

    if (auto_isrunning) 
    {
        xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state. If task is already running, no action
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by MQTT topic " + _topic);
    }
    else 
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Flow start triggered by MQTT topic " + _topic + ", but flow is not active!");
    }  

    #ifdef DEBUG_DETAIL_ON   
        LogFile.WriteHeapInfo("MQTTCtrlFlowStart - Done");       
    #endif

    return ESP_OK;
}
#endif //ENABLE_MQTT


esp_err_t handler_json(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_json - Start");    
    #endif

    ESP_LOGD(TAG, "handler_JSON uri: %s", req->uri);
    
    if (bTaskAutoFlowCreated) 
    {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_set_type(req, "application/json");

        std::string zw = tfliteflow.getJSON();
        if (zw.length() > 0) 
        {
            httpd_resp_send(req, zw.c_str(), zw.length());
        }
        else 
        {
            httpd_resp_send(req, NULL, 0);
        }
    }
    else 
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Flow not (yet) started: REST API /json not yet available!");
        return ESP_ERR_NOT_FOUND;
    }

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_JSON - Done");   
    #endif

    return ESP_OK;
}


esp_err_t handler_wasserzaehler(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler water counter - Start");    
    #endif

    if (bTaskAutoFlowCreated) 
    {
        bool _rawValue = false;
        bool _noerror = false;
        bool _all = false;
        std::string _type = "value";
        string zw;

        ESP_LOGD(TAG, "handler water counter uri: %s", req->uri);														   

        char _query[100];
        char _size[10];

        if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
        {
    //        ESP_LOGD(TAG, "Query: %s", _query);
            if (httpd_query_key_value(_query, "all", _size, 10) == ESP_OK)
            {
                #ifdef DEBUG_DETAIL_ON       
                    ESP_LOGD(TAG, "all is found%s", _size);
                #endif
                _all = true;
            }

            if (httpd_query_key_value(_query, "type", _size, 10) == ESP_OK)
            {
                #ifdef DEBUG_DETAIL_ON       
                    ESP_LOGD(TAG, "all is found: %s", _size);
                #endif
                _type = std::string(_size);
            }

            if (httpd_query_key_value(_query, "rawvalue", _size, 10) == ESP_OK)
            {
                #ifdef DEBUG_DETAIL_ON       
                    ESP_LOGD(TAG, "rawvalue is found: %s", _size);
                #endif
                _rawValue = true;
            }
            if (httpd_query_key_value(_query, "noerror", _size, 10) == ESP_OK)
            {
                #ifdef DEBUG_DETAIL_ON       
                    ESP_LOGD(TAG, "noerror is found: %s", _size);
                #endif
                _noerror = true;
            }        
        }  

        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

        if (_all)
        {
            httpd_resp_set_type(req, "text/plain");
            ESP_LOGD(TAG, "TYPE: %s", _type.c_str());
            int _intype = READOUT_TYPE_VALUE;
            if (_type == "prevalue")
                _intype = READOUT_TYPE_PREVALUE;
            if (_type == "raw")
                _intype = READOUT_TYPE_RAWVALUE;
            if (_type == "error")
                _intype = READOUT_TYPE_ERROR;


            zw = tfliteflow.getReadoutAll(_intype);
            ESP_LOGD(TAG, "ZW: %s", zw.c_str());
            if (zw.length() > 0)
                httpd_resp_send(req, zw.c_str(), zw.length()); 
            
            return ESP_OK;
        }

        zw = tfliteflow.getReadout(_rawValue, _noerror);
        if (zw.length() > 0)
            httpd_resp_sendstr_chunk(req, zw.c_str()); 

        string query = std::string(_query);
    //    ESP_LOGD(TAG, "Query: %s, query.c_str());
        if (query.find("full") != std::string::npos)
        {
            string txt, zw;
            
            txt = "<p>Aligned Image: <p><img src=\"/img_tmp/alg_roi.jpg\"> <p>\n";
            txt = txt + "Digital Counter: <p> ";
            httpd_resp_sendstr_chunk(req, txt.c_str()); 
            
            std::vector<HTMLInfo*> htmlinfodig;
            htmlinfodig = tfliteflow.GetAllDigital();  

            for (int i = 0; i < htmlinfodig.size(); ++i)
            {
                if (tfliteflow.GetTypeDigital() == Digital)
                {
                    if (htmlinfodig[i]->val == 10)
                        zw = "NaN";
                    else
                        zw = to_string((int) htmlinfodig[i]->val);

                    txt = "<img src=\"/img_tmp/" +  htmlinfodig[i]->filename + "\"> " + zw;
                }
                else
                {
                    std::stringstream stream;
                    stream << std::fixed << std::setprecision(1) << htmlinfodig[i]->val;
                    zw = stream.str();

                    txt = "<img src=\"/img_tmp/" +  htmlinfodig[i]->filename + "\"> " + zw;
                }
                httpd_resp_sendstr_chunk(req, txt.c_str()); 
                delete htmlinfodig[i];
            }
            htmlinfodig.clear();
        
            txt = " <p> Analog Meter: <p> ";
            httpd_resp_sendstr_chunk(req, txt.c_str()); 
            
            std::vector<HTMLInfo*> htmlinfoana;
            htmlinfoana = tfliteflow.GetAllAnalog();
            for (int i = 0; i < htmlinfoana.size(); ++i)
            {
                std::stringstream stream;
                stream << std::fixed << std::setprecision(1) << htmlinfoana[i]->val;
                zw = stream.str();

                txt = "<img src=\"/img_tmp/" +  htmlinfoana[i]->filename + "\"> " + zw;
                httpd_resp_sendstr_chunk(req, txt.c_str()); 
                delete htmlinfoana[i];
            }
            htmlinfoana.clear();   

        }   

        /* Respond with an empty chunk to signal HTTP response completion */
        httpd_resp_sendstr_chunk(req, NULL);   
    }
    else 
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Flow not (yet) started: REST API /value not available!");
        return ESP_ERR_NOT_FOUND;
    }

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_wasserzaehler - Done");   
    #endif

    return ESP_OK;
}


esp_err_t handler_editflow(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_editflow - Start");       
    #endif

    ESP_LOGD(TAG, "handler_editflow uri: %s", req->uri);

    char _query[200];
    char _valuechar[30];
    string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
            #ifdef DEBUG_DETAIL_ON       
                ESP_LOGD(TAG, "task is found: %s", _valuechar);
            #endif
            _task = string(_valuechar);
        }
    }  

    if (_task.compare("namenumbers") == 0)
    {
        ESP_LOGD(TAG, "Get NUMBER list");
        return get_numbers_file_handler(req);
    }

    if (_task.compare("data") == 0)
    {
        ESP_LOGD(TAG, "Get data list");
        return get_data_file_handler(req);
    }

    if (_task.compare("tflite") == 0)
    {
        ESP_LOGD(TAG, "Get tflite list");
        return get_tflite_file_handler(req);
    }


    if (_task.compare("copy") == 0)
    {
        string in, out, zw;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);
        httpd_query_key_value(_query, "out", _valuechar, 30);         
        out = string(_valuechar);  

        #ifdef DEBUG_DETAIL_ON       
            ESP_LOGD(TAG, "in: %s", in.c_str());
            ESP_LOGD(TAG, "out: %s", out.c_str());
        #endif

        in = "/sdcard" + in;
        out = "/sdcard" + out;

        CopyFile(in, out);
        zw = "Copy Done";
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, zw.c_str(), zw.length()); 
    }


    if (_task.compare("cutref") == 0)
    {
        string in, out, zw;
        int x, y, dx, dy;
        bool enhance = false;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);

        httpd_query_key_value(_query, "out", _valuechar, 30);         
        out = string(_valuechar);  

        httpd_query_key_value(_query, "x", _valuechar, 30);
        zw = string(_valuechar);  
        x = stoi(zw);              

        httpd_query_key_value(_query, "y", _valuechar, 30);
        zw = string(_valuechar);  
        y = stoi(zw);              

        httpd_query_key_value(_query, "dx", _valuechar, 30);
        zw = string(_valuechar);  
        dx = stoi(zw);  

        httpd_query_key_value(_query, "dy", _valuechar, 30);
        zw = string(_valuechar);  
        dy = stoi(zw);          

        #ifdef DEBUG_DETAIL_ON       
            ESP_LOGD(TAG, "in: %s", in.c_str());
            ESP_LOGD(TAG, "out: %s", out.c_str());
            ESP_LOGD(TAG, "x: %s", zw.c_str());
            ESP_LOGD(TAG, "y: %s", zw.c_str());
            ESP_LOGD(TAG, "dx: %s", zw.c_str());
            ESP_LOGD(TAG, "dy: %s", zw.c_str());
        #endif

        if (httpd_query_key_value(_query, "enhance", _valuechar, 10) == ESP_OK)
        {
            zw = string(_valuechar);
            if (zw.compare("true") == 0)
            {
                enhance = true;
            }
        }

        in = "/sdcard" + in;
        out = "/sdcard" + out;

        string out2 = out.substr(0, out.length() - 4) + "_org.jpg";

        CAlignAndCutImage *caic = new CAlignAndCutImage(in);
        caic->CutAndSave(out2, x, y, dx, dy);
        delete caic;    

        CImageBasis *cim = new CImageBasis(out2);
        if (enhance)
        {
            cim->Contrast(90);
        }

        cim->SaveToFile(out);
        delete cim;        

        zw = "CutImage Done";
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, zw.c_str(), zw.length()); 
        
    }

    if (_task.compare("test_take") == 0)
    {
        std::string _host = "";
        std::string _bri = "";
        std::string _con = "";
        std::string _sat = "";
        std::string _int = "";
        int bri = -100;
        int sat = -100;
        int con = -100;
        int intens = -100;

        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK) {
            _host = std::string(_valuechar);
        }
        if (httpd_query_key_value(_query, "int", _valuechar, 30) == ESP_OK) {
            _int = std::string(_valuechar);
            intens = stoi(_int);
        }
        if (httpd_query_key_value(_query, "bri", _valuechar, 30) == ESP_OK) {
            _bri = std::string(_valuechar);
            bri = stoi(_bri);
        }
        if (httpd_query_key_value(_query, "con", _valuechar, 30) == ESP_OK) {
            _con = std::string(_valuechar);
            con = stoi(_con);
        }
        if (httpd_query_key_value(_query, "sat", _valuechar, 30) == ESP_OK) {
            _sat = std::string(_valuechar);
            sat = stoi(_sat);
        }


//        ESP_LOGD(TAG, "Parameter host: %s", _host.c_str());
//        string zwzw = "Do " + _task + " start\n"; ESP_LOGD(TAG, zwzw.c_str());
        Camera.SetBrightnessContrastSaturation(bri, con, sat);
        Camera.SetLEDIntensity(intens);
        ESP_LOGD(TAG, "test_take - vor MakeImage");
        std::string zw = tfliteflow.doSingleStep("[MakeImage]", _host);
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, zw.c_str(), zw.length()); 
    } 


    if (_task.compare("test_align") == 0)
    {
        std::string _host = "";
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK) {
            _host = std::string(_valuechar);
        }
//        ESP_LOGD(TAG, "Parameter host: %s", _host.c_str());

//        string zwzw = "Do " + _task + " start\n"; ESP_LOGD(TAG, zwzw.c_str());
        std::string zw = tfliteflow.doSingleStep("[Alignment]", _host);
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, zw.c_str(), zw.length()); 
    }

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_editflow - Done");       
    #endif

    return ESP_OK;
}


esp_err_t handler_statusflow(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_prevalue - Start");       
    #endif

    const char* resp_str;
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (bTaskAutoFlowCreated) 
    {
        #ifdef DEBUG_DETAIL_ON       
            ESP_LOGD(TAG, "handler_prevalue: %s", req->uri);
        #endif

        string* zw = tfliteflow.getActStatus();
        resp_str = zw->c_str();

        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);   
    }
    else 
    {
        resp_str = "Flow task not yet created";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);  
    }

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_prevalue - Done");       
    #endif

    return ESP_OK;
}


esp_err_t handler_cputemp(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_cputemp - Start");       
    #endif

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, std::to_string((int)temperatureRead()).c_str(), HTTPD_RESP_USE_STRLEN);

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_cputemp - End");       
    #endif

    return ESP_OK;
}


esp_err_t handler_rssi(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_rssi - Start");       
    #endif

    if (getWIFIisConnected()) 
    {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, std::to_string(get_WIFI_RSSI()).c_str(), HTTPD_RESP_USE_STRLEN);
    }
    else 
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "WIFI not (yet) connected: REST API /rssi not available!");
        return ESP_ERR_NOT_FOUND;
    }      

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_rssi - End");       
    #endif

    return ESP_OK;
}


esp_err_t handler_uptime(httpd_req_t *req)
{

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_uptime - Start");       
    #endif
    
    std::string formatedUptime = getFormatedUptime(false);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, formatedUptime.c_str(), formatedUptime.length());  

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_uptime - End");       
    #endif

    return ESP_OK;
}


esp_err_t handler_prevalue(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_prevalue - Start");       
    #endif

    const char* resp_str;
    string zw;

    #ifdef DEBUG_DETAIL_ON       
        ESP_LOGD(TAG, "handler_prevalue: %s", req->uri);
    #endif

    char _query[100];
    char _size[10] = "";
    char _numbers[50] = "default";

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
        #ifdef DEBUG_DETAIL_ON       
            ESP_LOGD(TAG, "Query: %s", _query);
        #endif

        if (httpd_query_key_value(_query, "value", _size, 10) == ESP_OK)
        {
            #ifdef DEBUG_DETAIL_ON       
                ESP_LOGD(TAG, "Value: %s", _size);
            #endif
        }

        httpd_query_key_value(_query, "numbers", _numbers, 50);
    }      

    if (strlen(_size) == 0)
    {
        zw = tfliteflow.GetPrevalue(std::string(_numbers));
    }
    else
    {
        zw = "SetPrevalue to " + tfliteflow.UpdatePrevalue(_size, _numbers, true);
    }
    
    resp_str = zw.c_str();

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);  

    #ifdef DEBUG_DETAIL_ON       
        LogFile.WriteHeapInfo("handler_prevalue - End");       
    #endif

    return ESP_OK;
}


void task_autodoFlow(void *pvParameter)
{
    int64_t fr_start, fr_delta_ms;

    bTaskAutoFlowCreated = true;

    if (!isPlannedReboot)
    {
        if (esp_reset_reason() == ESP_RST_PANIC) {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Restarted due to an Exception/panic! Postponing first round start by 5 minutes to allow for an OTA Update or to fetch the log!"); 
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Setting logfile level to DEBUG until the next reboot!");
            LogFile.setLogLevel(ESP_LOG_DEBUG);
            tfliteflow.setActStatus("Initialization (delayed)");
            //#ifdef ENABLE_MQTT
                //MQTTPublish(mqttServer_getMainTopic() + "/" + "status", "Initialization (delayed)", false); // Right now, not possible -> MQTT Service is going to be started later
            //#endif //ENABLE_MQTT
            vTaskDelay(60*5000 / portTICK_RATE_MS); // Wait 5 minutes to give time to do an OTA Update or fetch the log
        }
    }


    ESP_LOGD(TAG, "task_autodoFlow: start");
    doInit();

    auto_isrunning = tfliteflow.isAutoStart(auto_intervall);

    if (isSetupModusActive()) 
    {
        auto_isrunning = false;
        std::string zw_time = getCurrentTimeString(LOGFILE_TIME_FORMAT);
        tfliteflow.doFlowMakeImageOnly(zw_time);
    }
    
    while (auto_isrunning)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "----------------------------------------------------------------"); // Clear separation between runs
        std::string _zw = "Round #" + std::to_string(++countRounds) + " started";
        time_t roundStartTime = getUpTime();
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, _zw); 
        fr_start = esp_timer_get_time();

        if (flowisrunning)
        {
            #ifdef DEBUG_DETAIL_ON       
                ESP_LOGD(TAG, "Autoflow: doFlow is already running!");
            #endif
        }
        else
        {
            #ifdef DEBUG_DETAIL_ON       
                ESP_LOGD(TAG, "Autoflow: doFlow is started");
            #endif
            flowisrunning = true;
            doflow();
            #ifdef DEBUG_DETAIL_ON       
                ESP_LOGD(TAG, "Remove older log files");
            #endif
            LogFile.RemoveOldLogFile();
            LogFile.RemoveOldDataLog();
        }
        
        //CPU Temp -> Logfile
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CPU Temperature: " + std::to_string((int)temperatureRead()) + "°C");
        
        // WIFI Signal Strength (RSSI) -> Logfile
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "WIFI Signal (RSSI): " + std::to_string(get_WIFI_RSSI()) + "dBm");
        
        //Round finished -> Logfile
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Round #" + std::to_string(countRounds) + 
                " completed (" + std::to_string(getUpTime() - roundStartTime) + " seconds)");

        fr_delta_ms = (esp_timer_get_time() - fr_start) / 1000;
        if (auto_intervall > fr_delta_ms)
        {
            const TickType_t xDelay = (auto_intervall - fr_delta_ms)  / portTICK_PERIOD_MS;
            ESP_LOGD(TAG, "Autoflow: sleep for: %ldms", (long) xDelay);
            vTaskDelay( xDelay );        
        }
    }
    vTaskDelete(NULL); //Delete this task if it exits from the loop above
    xHandletask_autodoFlow = NULL;
    ESP_LOGD(TAG, "task_autodoFlow: end");
}


void TFliteDoAutoStart()
{
    BaseType_t xReturned;

    ESP_LOGD(TAG, "getESPHeapInfo: %s", getESPHeapInfo().c_str());

    xReturned = xTaskCreatePinnedToCore(&task_autodoFlow, "task_autodoFlow", 16 * 1024, NULL, tskIDLE_PRIORITY+2, &xHandletask_autodoFlow, 0);
    //xReturned = xTaskCreate(&task_autodoFlow, "task_autodoFlow", 16 * 1024, NULL, tskIDLE_PRIORITY+2, &xHandletask_autodoFlow);
    if( xReturned != pdPASS )
    {
       ESP_LOGD(TAG, "ERROR task_autodoFlow konnte nicht erzeugt werden!");
    }
    ESP_LOGD(TAG, "getESPHeapInfo: %s", getESPHeapInfo().c_str());
}


#ifdef ENABLE_MQTT
std::string GetMQTTMainTopic()
{
    return tfliteflow.GetMQTTMainTopic();
}
#endif//ENABLE_MQTT


void register_server_tflite_uri(httpd_handle_t server)
{
    ESP_LOGI(TAG, "server_part_camera - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;

    camuri.uri       = "/doinit";
    camuri.handler   = handler_init;
    camuri.user_ctx  = (void*) "Light On";    
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/setPreValue"
    camuri.uri       = "/setPreValue.html";
    camuri.handler   = handler_prevalue;
    camuri.user_ctx  = (void*) "Prevalue";    
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/setPreValue";
    camuri.handler   = handler_prevalue;
    camuri.user_ctx  = (void*) "Prevalue";    
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/flow_start";
    camuri.handler   = handler_flow_start;
    camuri.user_ctx  = (void*) "Flow Start"; 
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/statusflow.html";
    camuri.handler   = handler_statusflow;
    camuri.user_ctx  = (void*) "Light Off"; 
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/statusflow";
    camuri.handler   = handler_statusflow;
    camuri.user_ctx  = (void*) "Light Off";
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/cpu_temperature"
    camuri.uri       = "/cputemp.html";
    camuri.handler   = handler_cputemp;
    camuri.user_ctx  = (void*) "Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/cpu_temperature";
    camuri.handler   = handler_cputemp;
    camuri.user_ctx  = (void*) "Light Off"; 
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/rssi"
    camuri.uri       = "/rssi.html";
    camuri.handler   = handler_rssi;
    camuri.user_ctx  = (void*) "Light Off"; 
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/rssi";
    camuri.handler   = handler_rssi;
    camuri.user_ctx  = (void*) "Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/uptime";
    camuri.handler   = handler_uptime;
    camuri.user_ctx  = (void*) "Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/editflow";
    camuri.handler   = handler_editflow;
    camuri.user_ctx  = (void*) "EditFlow"; 
    httpd_register_uri_handler(server, &camuri);   

    // Legacy API => New: "/value"
    camuri.uri       = "/value.html";
    camuri.handler   = handler_wasserzaehler;
    camuri.user_ctx  = (void*) "Value";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/value";
    camuri.handler   = handler_wasserzaehler;
    camuri.user_ctx  = (void*) "Value"; 
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/value"
    camuri.uri       = "/wasserzaehler.html";
    camuri.handler   = handler_wasserzaehler;
    camuri.user_ctx  = (void*) "Wasserzaehler"; 
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/json";
    camuri.handler   = handler_json;
    camuri.user_ctx  = (void*) "JSON"; 
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/heap";
    camuri.handler   = handler_get_heap;
    camuri.user_ctx  = (void*) "Heap"; 
    httpd_register_uri_handler(server, &camuri);
}
