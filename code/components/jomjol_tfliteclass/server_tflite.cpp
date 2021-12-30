#include "server_tflite.h"

#include <string>
#include <vector>
#include "string.h"
#include "esp_log.h"

#include <iomanip>
#include <sstream>

#include "defines.h"
#include "Helper.h"

#include "esp_camera.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"

#include "ClassFlowControll.h"

#include "ClassLogFile.h"
#include "server_GPIO.h"

// #define DEBUG_DETAIL_ON       


ClassFlowControll tfliteflow;

TaskHandle_t xHandleblink_task_doFlow = NULL;
TaskHandle_t xHandletask_autodoFlow = NULL;

bool flowisrunning = false;

long auto_intervall = 0;
bool auto_isrunning = false;


int countRounds = 0;

static const char *TAGTFLITE = "server_tflite";


int getCountFlowRounds() {
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
    printf("Handle: xHandleblink_task_doFlow: %ld\n", (long) xHandleblink_task_doFlow);  
#endif  
    if (xHandleblink_task_doFlow != NULL)
    {
        TaskHandle_t xHandleblink_task_doFlowTmp = xHandleblink_task_doFlow;
        xHandleblink_task_doFlow = NULL;
        vTaskDelete(xHandleblink_task_doFlowTmp);
#ifdef DEBUG_DETAIL_ON      
        printf("Killed: xHandleblink_task_doFlow\n");
#endif
    }

#ifdef DEBUG_DETAIL_ON      
    printf("Handle: xHandletask_autodoFlow: %ld\n", (long) xHandletask_autodoFlow);  
#endif
    if (xHandletask_autodoFlow != NULL)
    {
        TaskHandle_t xHandletask_autodoFlowTmp = xHandletask_autodoFlow;
        xHandletask_autodoFlow = NULL;
        vTaskDelete(xHandletask_autodoFlowTmp);
#ifdef DEBUG_DETAIL_ON      
        printf("Killed: xHandletask_autodoFlow\n");
#endif
    }

}

void doInit(void)
{
#ifdef DEBUG_DETAIL_ON             
    printf("Start tfliteflow.InitFlow(config);\n");
#endif
    tfliteflow.InitFlow(CONFIG_FILE);
#ifdef DEBUG_DETAIL_ON      
    printf("Finished tfliteflow.InitFlow(config);\n");
#endif
}


bool doflow(void)
{
    
    std::string zw_time = gettimestring(LOGFILE_TIME_FORMAT);
    printf("doflow - start %s\n", zw_time.c_str());
    flowisrunning = true;
    tfliteflow.doFlow(zw_time);
    flowisrunning = false;

#ifdef DEBUG_DETAIL_ON      
    printf("doflow - end %s\n", zw_time.c_str());
#endif    
    return true;
}

void blink_task_doFlow(void *pvParameter)
{
#ifdef DEBUG_DETAIL_ON          
    printf("blink_task_doFlow\n");
#endif
    if (!flowisrunning)
    {
        flowisrunning = true;
        doflow();
        flowisrunning = false;
    }
    vTaskDelete(NULL); //Delete this task if it exits from the loop above
    xHandleblink_task_doFlow = NULL;
}


esp_err_t handler_init(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("handler_init - Start");       
    printf("handler_doinit uri:\n"); printf(req->uri); printf("\n");
#endif

    const char* resp_str = "Init started<br>";
    httpd_resp_send(req, resp_str, strlen(resp_str));     

    doInit();

    resp_str = "Init done<br>";
    httpd_resp_send(req, resp_str, strlen(resp_str));     
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);    

#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("handler_init - Done");       
#endif

    return ESP_OK;
};

esp_err_t handler_doflow(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON          
    LogFile.WriteHeapInfo("handler_doflow - Start");       
#endif

    printf("handler_doFlow uri: "); printf(req->uri); printf("\n");

    if (flowisrunning)
    {
        const char* resp_str = "doFlow läuft bereits und kann nicht nochmal gestartet werden";
        httpd_resp_send(req, resp_str, strlen(resp_str));       
        return 2;
    }
    else
    {
        xTaskCreate(&blink_task_doFlow, "blink_doFlow", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY+1, &xHandleblink_task_doFlow);
    }
    const char* resp_str = "doFlow gestartet - dauert ca. 60 Sekunden";
    httpd_resp_send(req, resp_str, strlen(resp_str));  
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);       

#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_doflow - Done");       
#endif

    return ESP_OK;
};


esp_err_t handler_json(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_json - Start");    
#endif


    printf("handler_JSON uri:\n"); printf(req->uri); printf("\n");

    char _query[100];
    char _size[10];

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "application/json");

    std::string zw = tfliteflow.getJSON();
    if (zw.length() > 0)
        httpd_resp_sendstr_chunk(req, zw.c_str()); 

    string query = std::string(_query);

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);   

#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_JSON - Done");   
#endif
    return ESP_OK;
};



esp_err_t handler_wasserzaehler(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_wasserzaehler - Start");    
#endif

    bool _rawValue = false;
    bool _noerror = false;
    bool _all = false;
    std::string _type = "value";
    string zw;

    printf("handler_wasserzaehler uri:\n"); printf(req->uri); printf("\n");

    char _query[100];
    char _size[10];

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
//        printf("Query: "); printf(_query); printf("\n");
        if (httpd_query_key_value(_query, "all", _size, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON       
            printf("all is found"); printf(_size); printf("\n"); 
#endif
            _all = true;
        }

        if (httpd_query_key_value(_query, "type", _size, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON       
            printf("all is found"); printf(_size); printf("\n"); 
#endif
            _type = std::string(_size);
        }

        if (httpd_query_key_value(_query, "rawvalue", _size, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON       
            printf("rawvalue is found"); printf(_size); printf("\n"); 
#endif
            _rawValue = true;
        }
        if (httpd_query_key_value(_query, "noerror", _size, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON       
            printf("noerror is found"); printf(_size); printf("\n"); 
#endif
            _noerror = true;
        }        
    }  

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (_all)
    {
        httpd_resp_set_type(req, "text/plain");
        printf("TYPE: %s\n", _type.c_str());
        int _intype = READOUT_TYPE_VALUE;
        if (_type == "prevalue")
            _intype = READOUT_TYPE_PREVALUE;
        if (_type == "raw")
            _intype = READOUT_TYPE_RAWVALUE;
        if (_type == "error")
            _intype = READOUT_TYPE_ERROR;


        zw = tfliteflow.getReadoutAll(_intype);
        printf("ZW: %s\n", zw.c_str());
        if (zw.length() > 0)
            httpd_resp_sendstr_chunk(req, zw.c_str()); 
        httpd_resp_sendstr_chunk(req, NULL);   
        return ESP_OK;
    }

    zw = tfliteflow.getReadout(_rawValue, _noerror);
    if (zw.length() > 0)
        httpd_resp_sendstr_chunk(req, zw.c_str()); 

    string query = std::string(_query);
//    printf("Query: %s\n", query.c_str());
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

#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_wasserzaehler - Done");   
#endif
    return ESP_OK;
};


esp_err_t handler_editflow(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_editflow - Start");       
#endif

    printf("handler_editflow uri: "); printf(req->uri); printf("\n");

    char _query[200];
    char _valuechar[30];
    string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON       
            printf("task is found: %s\n", _valuechar); 
#endif
            _task = string(_valuechar);
        }
    }  

    if (_task.compare("copy") == 0)
    {
        string in, out, zw;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);
        httpd_query_key_value(_query, "out", _valuechar, 30);         
        out = string(_valuechar);  

#ifdef DEBUG_DETAIL_ON       
        printf("in: "); printf(in.c_str()); printf("\n"); 
        printf("out: "); printf(out.c_str()); printf("\n"); 
#endif

        in = "/sdcard" + in;
        out = "/sdcard" + out;

        CopyFile(in, out);
        zw = "Copy Done";
        httpd_resp_sendstr_chunk(req, zw.c_str()); 
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
        printf("in: "); printf(in.c_str()); printf("\n"); 
        printf("out: "); printf(out.c_str()); printf("\n"); 
        printf("x: "); printf(zw.c_str()); printf("\n"); 
        printf("y: "); printf(zw.c_str()); printf("\n"); 
        printf("dx: "); printf(zw.c_str()); printf("\n"); 
        printf("dy: "); printf(zw.c_str()); printf("\n"); 
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
        httpd_resp_sendstr_chunk(req, zw.c_str()); 
        
    }

    if (_task.compare("test_take") == 0)
    {
        std::string _host = "";
        std::string _bri = "";
        std::string _con = "";
        std::string _sat = "";
        int bri = -100;
        int sat = -100;
        int con = -100;

        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK) {
            _host = std::string(_valuechar);
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


//        printf("Parameter host: "); printf(_host.c_str()); printf("\n"); 
//        string zwzw = "Do " + _task + " start\n"; printf(zwzw.c_str());
        Camera.SetBrightnessContrastSaturation(bri, con, sat);
        std::string zw = tfliteflow.doSingleStep("[MakeImage]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str()); 
    } 


    if (_task.compare("test_align") == 0)
    {
        std::string _host = "";
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK) {
            _host = std::string(_valuechar);
        }
//        printf("Parameter host: "); printf(_host.c_str()); printf("\n"); 

//        string zwzw = "Do " + _task + " start\n"; printf(zwzw.c_str());
        std::string zw = tfliteflow.doSingleStep("[Alignment]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str()); 
    }

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);   

#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_editflow - Done");       
#endif

    return ESP_OK;
};


esp_err_t handler_statusflow(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_prevalue - Start");       
#endif

    const char* resp_str;

#ifdef DEBUG_DETAIL_ON       
    printf("handler_prevalue:\n"); printf(req->uri); printf("\n");
#endif

    string* zw = tfliteflow.getActStatus();
    resp_str = zw->c_str();

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, resp_str, strlen(resp_str));   
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);      

#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_prevalue - Start");       
#endif

    return ESP_OK;
};


esp_err_t handler_prevalue(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_prevalue - Start");       
#endif

    const char* resp_str;
    string zw;

#ifdef DEBUG_DETAIL_ON       
    printf("handler_prevalue:\n"); printf(req->uri); printf("\n");
#endif

    char _query[100];
    char _size[10] = "";
    char _numbers[50] = "default";

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
#ifdef DEBUG_DETAIL_ON       
        printf("Query: "); printf(_query); printf("\n");
#endif

        if (httpd_query_key_value(_query, "value", _size, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON       
            printf("Value: "); printf(_size); printf("\n"); 
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


    httpd_resp_send(req, resp_str, strlen(resp_str));   
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);      

#ifdef DEBUG_DETAIL_ON       
    LogFile.WriteHeapInfo("handler_prevalue - Start");       
#endif

    return ESP_OK;
};

void task_autodoFlow(void *pvParameter)
{
    int64_t fr_start, fr_delta_ms;

    printf("task_autodoFlow: start\r\n");
    doInit();
    gpio_handler_init();

    auto_isrunning = tfliteflow.isAutoStart(auto_intervall);
    if (isSetupModusActive()) {
        auto_isrunning = false;
        std::string zw_time = gettimestring(LOGFILE_TIME_FORMAT);
        tfliteflow.doFlowMakeImageOnly(zw_time);

    }
    while (auto_isrunning)
    {
        std::string _zw = "task_autodoFlow - next round - Round #" + std::to_string(++countRounds);
        LogFile.WriteToFile(_zw); 
        printf("Autoflow: start\n");
        fr_start = esp_timer_get_time();

        if (flowisrunning)
        {
#ifdef DEBUG_DETAIL_ON       
            printf("Autoflow: doFLow laeuft bereits!\n");
#endif
        }
        else
        {
#ifdef DEBUG_DETAIL_ON       
            printf("Autoflow: doFLow wird gestartet\n");
#endif
            flowisrunning = true;
            doflow();
#ifdef DEBUG_DETAIL_ON       
            printf("Remove older log files\n");
#endif
            LogFile.RemoveOld();
        }
        
        LogFile.WriteToFile("task_autodoFlow - round done");
        //CPU Temp
        float cputmp = temperatureRead();
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << cputmp;
        string zwtemp = "CPU Temperature: " + stream.str();
        LogFile.WriteToFile(zwtemp); 
        printf("CPU Temperature: %.2f\n", cputmp);
        fr_delta_ms = (esp_timer_get_time() - fr_start) / 1000;
        if (auto_intervall > fr_delta_ms)
        {
            const TickType_t xDelay = (auto_intervall - fr_delta_ms)  / portTICK_PERIOD_MS;
            printf("Autoflow: sleep for : %ldms\n", (long) xDelay);
            vTaskDelay( xDelay );        
        }
    }
    vTaskDelete(NULL); //Delete this task if it exits from the loop above
    xHandletask_autodoFlow = NULL;
    printf("task_autodoFlow: end\r\n");
}

void TFliteDoAutoStart()
{
    BaseType_t xReturned;

    int _i = configMINIMAL_STACK_SIZE;

    printf("task_autodoFlow configMINIMAL_STACK_SIZE: %d\n", _i);
    printf("getESPHeapInfo: %s\n", getESPHeapInfo().c_str());

    xReturned = xTaskCreate(&task_autodoFlow, "task_autodoFlow", configMINIMAL_STACK_SIZE * 60, NULL, tskIDLE_PRIORITY+1, &xHandletask_autodoFlow);
    if( xReturned != pdPASS )
    {

       //Memory: 64 --> 48 --> 35 --> 25
       printf("ERROR task_autodoFlow konnte nicht erzeugt werden !!\r\n");
    }
    printf("getESPHeapInfo: %s\n", getESPHeapInfo().c_str());


}

std::string GetMQTTMainTopic()
{
    return tfliteflow.GetMQTTMainTopic();
}



void register_server_tflite_uri(httpd_handle_t server)
{
    ESP_LOGI(TAGTFLITE, "server_part_camera - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;

    camuri.uri       = "/doinit";
    camuri.handler   = handler_init;
    camuri.user_ctx  = (void*) "Light On";    
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/setPreValue.html";
    camuri.handler   = handler_prevalue;
    camuri.user_ctx  = (void*) "Prevalue";    
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/doflow";
    camuri.handler   = handler_doflow;
    camuri.user_ctx  = (void*) "Light Off"; 
    httpd_register_uri_handler(server, &camuri);  

    camuri.uri       = "/statusflow.html";
    camuri.handler   = handler_statusflow;
    camuri.user_ctx  = (void*) "Light Off"; 
    httpd_register_uri_handler(server, &camuri);  
    
    camuri.uri       = "/editflow.html";
    camuri.handler   = handler_editflow;
    camuri.user_ctx  = (void*) "EditFlow"; 
    httpd_register_uri_handler(server, &camuri);     

    camuri.uri       = "/wasserzaehler.html";
    camuri.handler   = handler_wasserzaehler;
    camuri.user_ctx  = (void*) "Wasserzaehler"; 
    httpd_register_uri_handler(server, &camuri);  

    camuri.uri       = "/json";
    camuri.handler   = handler_json;
    camuri.user_ctx  = (void*) "JSON"; 
    httpd_register_uri_handler(server, &camuri);     

}
