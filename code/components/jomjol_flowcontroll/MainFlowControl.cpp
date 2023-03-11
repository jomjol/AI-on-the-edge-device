#include "MainFlowControl.h"

#include <string>
#include <vector>
#include "string.h"
#include "esp_log.h"

#include <iomanip>
#include <sstream>

#include "../../include/defines.h"
#include "Helper.h"
#include "statusled.h"

#include "esp_camera.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"

#include "ClassFlowControll.h"

#include "ClassLogFile.h"
#include "server_GPIO.h"

#include "server_file.h"
#include "server_help.h"

#include "read_wlanini.h"
#include "connect_wlan.h"

#ifdef ENABLE_MQTT
    #include "interface_mqtt.h"
    #include "server_mqtt.h"
#endif //ENABLE_MQTT


ClassFlowControll flowctrl;

static bool isPlannedReboot = false;
static TaskHandle_t xHandletask_autodoFlow = NULL;
static bool bTaskAutoFlowCreated = false;
static int taskAutoFlowState = FLOW_TASK_STATE_INIT;
static bool reloadConfig = false;
static bool manualFlowStart = false;
static long auto_interval = 0;
static int countRounds = 0;

static const char *TAG = "MAINCTRL";


//#define DEBUG_DETAIL_ON


void CheckIsPlannedReboot()
{
 	FILE *pfile;
    if ((pfile = fopen("/sdcard/reboot.txt", "r")) == NULL) {
		//LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Initial boot or not a planned reboot");
        isPlannedReboot = false;
	}
    else {
		LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Planned reboot");
        DeleteFile("/sdcard/reboot.txt");   // Prevent Boot Loop!!!
        isPlannedReboot = true;
	}
}


bool getIsPlannedReboot() 
{
    return isPlannedReboot;
}


int getCountFlowRounds() 
{
    return countRounds;
}


void setTaskAutoFlowState(uint8_t _value) 
{
    taskAutoFlowState = _value;
}


esp_err_t GetJPG(std::string _filename, httpd_req_t *req)
{
    return flowctrl.GetJPGStream(_filename, req);
}


esp_err_t GetRawJPG(httpd_req_t *req)
{
    return flowctrl.SendRawJPG(req);
}


bool isSetupModusActive() 
{
    return flowctrl.getStatusSetupModus();
}


void DeleteMainFlowTask()
{
    #ifdef DEBUG_DETAIL_ON      
        ESP_LOGD(TAG, "DeleteMainFlowTask: xHandletask_autodoFlow: %ld", (long) xHandletask_autodoFlow);
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


bool doInit(void)
{
    bool bRetVal = true;

    if (!flowctrl.InitFlow(CONFIG_FILE))
        bRetVal = false;
    
    /* GPIO handler has to be initialized before MQTT init to ensure proper topic subscription */
    gpio_handler_init();

    #ifdef ENABLE_MQTT
        flowctrl.StartMQTTService();
    #endif //ENABLE_MQTT

    return bRetVal;
}

/*
bool doflow(void)
{   
    std::string zw_time = getCurrentTimeString(LOGFILE_TIME_FORMAT);

    #ifdef DEBUG_DETAIL_ON    
        ESP_LOGD(TAG, "doflow - start %s", zw_time.c_str());
    #endif

    flowctrl.doFlow(zw_time);

    #ifdef DEBUG_DETAIL_ON      
        ESP_LOGD(TAG, "doflow - end %s", zw_time.c_str());
    #endif

    return true;
}
*/

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


esp_err_t handler_reload_config(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (taskAutoFlowState == FLOW_TASK_STATE_INIT ||
        taskAutoFlowState == FLOW_TASK_STATE_SETUPMODE ||
        taskAutoFlowState == FLOW_TASK_STATE_IDLE_AUTOSTART)
    {
        const std::string zw = getCurrentTimeString("%H:%M:%S") + ": Reload config and redo flow initialization...";
        httpd_resp_send(req, zw.c_str(), zw.length());
        reloadConfig = true;
    }
    else if (taskAutoFlowState == FLOW_TASK_STATE_INIT_DELAYED) 
    {
        const std::string zw = getCurrentTimeString("%H:%M:%S") + ": Abort waiting delay and continue with flow initialization...";
        httpd_resp_send(req, zw.c_str(), zw.length());
        xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state.      
    }
    else if (taskAutoFlowState == FLOW_TASK_STATE_IDLE_AUTOSTART) 
    {
        const std::string zw = getCurrentTimeString("%H:%M:%S") + ": Abort waiting delay, reload config and redo flow initialization...";
        httpd_resp_send(req, zw.c_str(), zw.length());
        xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state.                                                                 // TODO: Skip delay
        reloadConfig = true;
    }
    else if (taskAutoFlowState == FLOW_TASK_STATE_IMG_PROCESSING || 
             taskAutoFlowState == FLOW_TASK_STATE_PUBLISH_DATA ||
             taskAutoFlowState == FLOW_TASK_STATE_ADDITIONAL_TASKS) 
    {
        const std::string zw = getCurrentTimeString(LOGFILE_TIME_FORMAT) + ": Reload config and redo flow initialization as soon as possible";
        httpd_resp_send(req, zw.c_str(), zw.length());
        reloadConfig = true;
    }
    else 
    {
        const std::string zw = getCurrentTimeString(LOGFILE_TIME_FORMAT) + ": Reload config not possible because flow not available";
        httpd_resp_send(req, zw.c_str(), zw.length());
    }

    return ESP_OK;
}


esp_err_t handler_flow_start(httpd_req_t *req) 
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (taskAutoFlowState == FLOW_TASK_STATE_IDLE_NO_AUTOSTART || 
        taskAutoFlowState == FLOW_TASK_STATE_IDLE_AUTOSTART) 
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by REST API");
        const std::string zw = getCurrentTimeString("%H:%M:%S") + ": Flow start triggered by REST API";
        httpd_resp_send(req, zw.c_str(), zw.length());

        if (taskAutoFlowState == FLOW_TASK_STATE_IDLE_AUTOSTART)
            xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state

        manualFlowStart = true;
    }
    else if (taskAutoFlowState == FLOW_TASK_STATE_IMG_PROCESSING || 
             taskAutoFlowState == FLOW_TASK_STATE_PUBLISH_DATA ||
             taskAutoFlowState == FLOW_TASK_STATE_ADDITIONAL_TASKS) 
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by REST API (flow is processing, request delayed)");
        const std::string zw = getCurrentTimeString("%H:%M:%S") + ": Flow start triggered (flow is processing -> request delayed)";
        httpd_resp_send(req, zw.c_str(), zw.length());

        manualFlowStart = true;
    }
    else {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Flow start triggered by REST API, but flow is not yet initialized. Request processing not possible");
        const std::string zw = getCurrentTimeString("%H:%M:%S") + ": Flow start triggered by REST API, but flow is not yet initialized";
        httpd_resp_send(req, zw.c_str(), zw.length());
    }

    return ESP_OK;
}


#ifdef ENABLE_MQTT
esp_err_t MQTTCtrlFlowStart(std::string _topic) 
{
    if (taskAutoFlowState == FLOW_TASK_STATE_IDLE_NO_AUTOSTART || 
        taskAutoFlowState == FLOW_TASK_STATE_IDLE_AUTOSTART) 
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by MQTT topic " + _topic);

        if (taskAutoFlowState == FLOW_TASK_STATE_IDLE_AUTOSTART)
            xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state

        manualFlowStart = true;
    }
    else if (taskAutoFlowState == FLOW_TASK_STATE_IMG_PROCESSING || 
             taskAutoFlowState == FLOW_TASK_STATE_PUBLISH_DATA ||
             taskAutoFlowState == FLOW_TASK_STATE_ADDITIONAL_TASKS) 
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by MQTT topic "+ _topic + " (flow is processing, request delayed)");
        
        manualFlowStart = true;
    }
    else {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Flow start triggered by MQTT topic " + _topic + ", but flow is not yet initialized");
    }  

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

        std::string zw = flowctrl.getJSON();
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


            zw = flowctrl.getReadoutAll(_intype);
            ESP_LOGD(TAG, "ZW: %s", zw.c_str());
            if (zw.length() > 0)
                httpd_resp_send(req, zw.c_str(), zw.length()); 
            
            return ESP_OK;
        }


        std::string status = flowctrl.getActStatus();
        string query = std::string(_query);
    //    ESP_LOGD(TAG, "Query: %s, query.c_str());
        if (query.find("full") != std::string::npos)
        {
            string txt;
            txt = "<body style=\"font-family: arial\">";

            if ((countRounds <= 1) && (taskAutoFlowState <= FLOW_TASK_STATE_IMG_PROCESSING)) { // First round not completed yet
                txt += "<h3>Please wait for the first round to complete!</h3><h3>Current state: " + status + "</h3>\n";
            }
            else {
                txt += "<h3>Value</h3>";
            }

            httpd_resp_sendstr_chunk(req, txt.c_str());
        }


        zw = flowctrl.getReadout(_rawValue, _noerror);
        if (zw.length() > 0)
            httpd_resp_sendstr_chunk(req, zw.c_str()); 


        if (query.find("full") != std::string::npos)
        {
            string txt, zw;

            if ((countRounds <= 1) && (taskAutoFlowState <= FLOW_TASK_STATE_IMG_PROCESSING)) { // First round not completed yet
                // Nothing to do
            }
            else {
                /* Digital ROIs */
                txt = "<body style=\"font-family: arial\">";
                txt += "<h3>Recognized Digit ROIs (previous round)</h3>\n";
                txt += "<table style=\"border-spacing: 5px\"><tr style=\"text-align: center; vertical-align: top;\">\n";

                std::vector<HTMLInfo*> htmlinfodig;
                htmlinfodig = flowctrl.GetAllDigital(); 

                for (int i = 0; i < htmlinfodig.size(); ++i)
                {
                    if (flowctrl.GetTypeDigital() == Digital)
                    {
                        if (htmlinfodig[i]->val == 10)
                            zw = "NaN";
                        else
                            zw = to_string((int) htmlinfodig[i]->val);

                        txt += "<td style=\"width: 100px\"><h4>" + zw + "</h4><p><img src=\"/img_tmp/" +  htmlinfodig[i]->filename + "\"></p></td>\n";
                    }
                    else
                    {
                        std::stringstream stream;
                        stream << std::fixed << std::setprecision(1) << htmlinfodig[i]->val;
                        zw = stream.str();

                        txt += "<td style=\"width: 100px\"><h4>" + zw + "</h4><p><img src=\"/img_tmp/" +  htmlinfodig[i]->filename + "\"></p></td>\n";
                    }
                    delete htmlinfodig[i];
                }

                htmlinfodig.clear();
            
                txt += "</tr></table>\n";
                httpd_resp_sendstr_chunk(req, txt.c_str()); 


                /* Analog ROIs */
                txt = "<h3>Recognized Analog ROIs (previous round)</h3>\n";
                txt += "<table style=\"border-spacing: 5px\"><tr style=\"text-align: center; vertical-align: top;\">\n";
                
                std::vector<HTMLInfo*> htmlinfoana;
                htmlinfoana = flowctrl.GetAllAnalog();
                for (int i = 0; i < htmlinfoana.size(); ++i)
                {
                    std::stringstream stream;
                    stream << std::fixed << std::setprecision(1) << htmlinfoana[i]->val;
                    zw = stream.str();

                    txt += "<td style=\"width: 150px;\"><h4>" + zw + "</h4><p><img src=\"/img_tmp/" +  htmlinfoana[i]->filename + "\"></p></td>\n";
                delete htmlinfoana[i];
                }
                htmlinfoana.clear();   

                txt += "</tr>\n</table>\n";
                httpd_resp_sendstr_chunk(req, txt.c_str()); 


                /* Full Image 
                 * Only show it after the image got taken and aligned */
                txt = "<h3>Aligned Image (current round)</h3>\n";
                if ((status == std::string("Initialization")) || 
                    (status == std::string("Initialization (delayed)")) || 
                    (status == std::string("Take Image"))) {
                    txt += "<p>Current state: " + status + "</p>\n";
                }
                else {
                    txt += "<img src=\"/img_tmp/alg_roi.jpg\">\n";
                }
                httpd_resp_sendstr_chunk(req, txt.c_str()); 
            }
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
        ESP_LOGD(TAG, "test_take - vor TakeImage");
        std::string zw = flowctrl.doSingleStep("[TakeImage]", _host);
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
        std::string zw = flowctrl.doSingleStep("[Alignment]", _host);
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

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (bTaskAutoFlowCreated) 
    {
        #ifdef DEBUG_DETAIL_ON       
            ESP_LOGD(TAG, "handler_prevalue: %s", req->uri);
        #endif

        std::string zw = flowctrl.getActStatusWithTime();
        httpd_resp_send(req, zw.c_str(), zw.length());   
    }
    else 
    {
        httpd_resp_send(req, "Flow task not yet created", HTTPD_RESP_USE_STRLEN);  
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
        zw = flowctrl.GetPrevalue(std::string(_numbers));
    }
    else
    {
        zw = "SetPrevalue to " + flowctrl.UpdatePrevalue(_size, _numbers, true);
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
    int64_t fr_start = 0;
    time_t roundStartTime = 0;
    bTaskAutoFlowCreated = true;

    while (true)
    {
        // FLOW INITIALIZATION - DELAYED
        // Delay flow initialization if reboot was triggered by software exception
        // Note: Init and logging of the event is handled already in "main.cpp"
        // ********************************************
        if (taskAutoFlowState == FLOW_TASK_STATE_INIT_DELAYED) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Process state: " + (std::string)FLOW_INIT_DELAYED);
            flowctrl.setActStatus(FLOW_INIT_DELAYED);
            // Right now, it's not possible to provide state via MQTT because mqtt service is not yet started

            vTaskDelay(60*5000 / portTICK_PERIOD_MS); // Wait 5 minutes to give time to do an OTA update or fetch the log 

            taskAutoFlowState = FLOW_TASK_STATE_INIT; // Continue to FLOW INIT
        }

        // FLOW INITIALIZATION
        // ********************************************
        else if (taskAutoFlowState == FLOW_TASK_STATE_INIT) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Process state: " + (std::string)FLOW_INIT);
            flowctrl.setActStatus(FLOW_INIT);
            // Right now, it's not possible to provide state via MQTT because mqtt service is not yet started

            if (!doInit()) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Process state: " + (std::string)FLOW_INIT_FAILED);
                flowctrl.setActStatus(FLOW_INIT_FAILED);
                #ifdef ENABLE_MQTT
                if (getMQTTisConnected())
                    MQTTPublish(mqttServer_getMainTopic() + "/" + "status", flowctrl.getActStatus(), false);
                #endif //ENABLE_MQTT

                while (true) {                                      // Waiting for a REQUEST
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    if (reloadConfig) {
                        reloadConfig = false;
                        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Trigger: Reload configuration");
                        taskAutoFlowState = FLOW_TASK_STATE_INIT;   // Repeat FLOW INIT
                        break;
                    }
                }
            }
            else {
                taskAutoFlowState = FLOW_TASK_STATE_SETUPMODE;      // Continue to test if SETUP is ACTIVE
            }
        }

        // SETUP MODE CHECK
        // ********************************************
        else if (taskAutoFlowState == FLOW_TASK_STATE_SETUPMODE) {

            if (isSetupModusActive())
            {
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Process state: " + (std::string)FLOW_SETUP_MODE);
                flowctrl.setActStatus(FLOW_SETUP_MODE);
                #ifdef ENABLE_MQTT
                    MQTTPublish(mqttServer_getMainTopic() + "/" + "status", flowctrl.getActStatus(), false);
                #endif //ENABLE_MQTT

                //std::string zw_time = getCurrentTimeString(LOGFILE_TIME_FORMAT);
                //flowctrl.doFlowTakeImageOnly(zw_time);    // Start only ClassFlowTakeImage to capture images

                while (true) {                              // Waiting for a REQUEST
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    if (reloadConfig) {
                        reloadConfig = false;
                        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Trigger: Reload configuration");
                        taskAutoFlowState = FLOW_TASK_STATE_INIT;       // Setup Mode done --> Do FLOW INIT
                        break;
                    }
                }
            }
            else {
                taskAutoFlowState = FLOW_TASK_STATE_IDLE_NO_AUTOSTART;  // Continue to test if AUTOSTART is TRUE
            }
        }

        // AUTOSTART CHECK
        // ********************************************      
        else if (taskAutoFlowState == FLOW_TASK_STATE_IDLE_NO_AUTOSTART) {
    
            if (!flowctrl.isAutoStart(auto_interval)) {
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Process state: " + (std::string)FLOW_IDLE_NO_AUTOSTART);
                flowctrl.setActStatus(FLOW_IDLE_NO_AUTOSTART);
                #ifdef ENABLE_MQTT
                    MQTTPublish(mqttServer_getMainTopic() + "/" + "status", flowctrl.getActStatus(), false);
                #endif //ENABLE_MQTT

                while (true) {                              // Waiting for a REQUEST
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    if (reloadConfig) {
                        reloadConfig = false;
                        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Trigger: Reload configuration");
                        taskAutoFlowState = FLOW_TASK_STATE_INIT;           // Return to state "FLOW INIT"
                        break;
                    }
                    if (manualFlowStart) { 
                        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Start process (manual trigger)");
                        taskAutoFlowState = FLOW_TASK_STATE_IMG_PROCESSING; // Start manual triggered single round of "FLOW PROCESSING"  
                        break;
                    }
                }   
            }
            else {
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Start process (automatic trigger)");
                taskAutoFlowState = FLOW_TASK_STATE_IMG_PROCESSING;         // Continue to state "FLOW PROCESSING"
            }
        }

        // IMAGE PROCESSING / EVALUATION
        // ********************************************     
        else if (taskAutoFlowState == FLOW_TASK_STATE_IMG_PROCESSING) {       
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "----------------------------------------------------------------"); // Clear separation between runs
            std::string _zw = "Round #" + std::to_string(++countRounds) + " started";
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, _zw); 
            roundStartTime = getUpTime();
            fr_start = esp_timer_get_time();
                   
            if (flowctrl.doFlowImageEvaluation(getCurrentTimeString(LOGFILE_TIME_FORMAT))) {
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Image evaluation completed (" + 
                                    std::to_string(getUpTime() - roundStartTime) + "s)");
            }
            else {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Image evaluation failed");
            }

            taskAutoFlowState = FLOW_TASK_STATE_PUBLISH_DATA;               // Continue with TASKS after FLOW FINISHED
        }

        // PUBLISH DATA / RESULTS
        // ******************************************** 
        else if (taskAutoFlowState == FLOW_TASK_STATE_PUBLISH_DATA) {  

            if (!flowctrl.doFlowPublishData(getCurrentTimeString(LOGFILE_TIME_FORMAT))) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Publish data failed"); 
            }
            taskAutoFlowState = FLOW_TASK_STATE_ADDITIONAL_TASKS;           // Continue with TASKS after FLOW FINISHED
        }

        // ADDITIONAL TASKS
        // Process further tasks after image is fully processed and results are published
        // ********************************************
        else if (taskAutoFlowState == FLOW_TASK_STATE_ADDITIONAL_TASKS) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Process state: " + (std::string)FLOW_ADDITIONAL_TASKS);
            flowctrl.setActStatus(FLOW_ADDITIONAL_TASKS);
            #ifdef ENABLE_MQTT
                MQTTPublish(mqttServer_getMainTopic() + "/" + "status", flowctrl.getActStatus(), false);
            #endif //ENABLE_MQTT

            // Cleanup outdated log and data files (retention policy)  
            LogFile.RemoveOldLogFile();
            LogFile.RemoveOldDataLog();
 
            // CPU Temp -> Logfile
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CPU Temperature: " + std::to_string((int)temperatureRead()) + "°C");
            
            // WIFI Signal Strength (RSSI) -> Logfile
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "WIFI Signal (RSSI): " + std::to_string(get_WIFI_RSSI()) + "dBm");

            // Check if time is synchronized (if NTP is configured)
            if (getUseNtp() && !getTimeIsSet()) {
                LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Time server is configured, but time is not yet set. Check configuration");
                StatusLED(TIME_CHECK, 1, false);
            }


            /* Automatic error handling (if neccessary)
            // ********************************************
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "State: " + (std::string)FLOW_AUTO_ERROR_HANDLING);
            flowctrl.setActStatus(FLOW_AUTO_ERROR_HANDLING);
            #ifdef ENABLE_MQTT
                MQTTPublish(mqttServer_getMainTopic() + "/" + "status", flowctrl.getActStatus(), false);
            #endif //ENABLE_MQTT
            
            // nothing to do
            */


            // Round finished -> Logfile
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Round #" + std::to_string(countRounds) + 
                    " completed (" + std::to_string(getUpTime() - roundStartTime) + "s)");   


            // Check if triggerd reload config or manually triggered single round
            // ********************************************    
            if (reloadConfig) {
                reloadConfig = false;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Trigger: Reload configuration");
                taskAutoFlowState = FLOW_TASK_STATE_INIT;                   // Return to state "FLOW INIT"
            }
            else if (manualFlowStart) {
                manualFlowStart = false;
                taskAutoFlowState = FLOW_TASK_STATE_IDLE_NO_AUTOSTART;      // Return to state "Idle (NO AUTOSTART)"
            }
            else {
                taskAutoFlowState = FLOW_TASK_STATE_IDLE_AUTOSTART;         // Continue to state "Idle (AUTOSTART / WAITING STATE)"
            }
        }

        // IDLE / WAIT STATE
        // "Wait state" until autotimer is elapsed to restart next round
        // ********************************************
        else if (taskAutoFlowState == FLOW_TASK_STATE_IDLE_AUTOSTART) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Flow state: " + (std::string)FLOW_IDLE_AUTOSTART);
            flowctrl.setActStatus(FLOW_IDLE_AUTOSTART);
            #ifdef ENABLE_MQTT
                MQTTPublish(mqttServer_getMainTopic() + "/" + "status", flowctrl.getActStatus(), false);
            #endif //ENABLE_MQTT
            int64_t fr_delta_ms = (esp_timer_get_time() - fr_start) / 1000;
            if (auto_interval > fr_delta_ms)
            {
                const TickType_t xDelay = (auto_interval - fr_delta_ms)  / portTICK_PERIOD_MS;
                ESP_LOGD(TAG, "Autoflow: sleep for: %ldms", (long) xDelay * CONFIG_FREERTOS_HZ/portTICK_PERIOD_MS);
                vTaskDelay( xDelay );   
            }

            // Check if reload config is triggered by REST API
            // ********************************************    
            if (reloadConfig) {                     
                reloadConfig = false;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Trigger: Reload configuration");
                taskAutoFlowState = FLOW_TASK_STATE_INIT;               // Return to state "FLOW INIT"
            }
            else {
                taskAutoFlowState = FLOW_TASK_STATE_IMG_PROCESSING;     // Continue with next "FLOW PROCESSING" round
            }
        }

        // INVALID STATE
        // ********************************************
        else {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "taskAutoFlowState: Invalid state called. Programming error!");
            flowctrl.setActStatus(FLOW_INVALID_STATE);
        }
    }

    // Delete task if it exits from the loop above
    // ********************************************
    vTaskDelete(NULL);
    xHandletask_autodoFlow = NULL;
}


void StartMainFlowTask()
{
    #ifdef DEBUG_DETAIL_ON      
            LogFile.WriteHeapInfo("CreateFlowTask: start");
    #endif

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Process state: " FLOW_START_FLOW_TASK);
    flowctrl.setActStatus(FLOW_START_FLOW_TASK);

    BaseType_t xReturned = xTaskCreatePinnedToCore(&task_autodoFlow, "task_autodoFlow", 16 * 1024, NULL, tskIDLE_PRIORITY+2, &xHandletask_autodoFlow, 0);
    if( xReturned != pdPASS ) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to create task_autodoFlow");
        LogFile.WriteHeapInfo("CreateFlowTask: Failed to create task");
        flowctrl.setActStatus(FLOW_FLOW_TASK_FAILED);
    }

    #ifdef DEBUG_DETAIL_ON      
            LogFile.WriteHeapInfo("CreateFlowTask: end");
    #endif
}


void register_server_main_flow_task_uri(httpd_handle_t server)
{
    ESP_LOGI(TAG, "server_main_flow_task - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;

    camuri.uri       = "/reload_config";
    camuri.handler   = handler_reload_config;
    camuri.user_ctx  = (void*) "reload_config";    
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
