#include "server_tflite.h"

#include <string>
#include <vector>
#include "string.h"
#include "esp_log.h"

#include <iomanip>
#include <sstream>

#include "Helper.h"

#include "esp_camera.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"

#include "ClassFlowControll.h"

#include "ClassLogFile.h"

ClassFlowControll tfliteflow;

TaskHandle_t xHandleblink_task_doFlow = NULL;
TaskHandle_t xHandletask_autodoFlow = NULL;


bool flowisrunning = false;

long auto_intervall = 0;
bool auto_isrunning = false;

bool isSetupModusActive() {
    return tfliteflow.getStatusSetupModus();
    return false;
}


void KillTFliteTasks()
{
    printf("Handle: xHandleblink_task_doFlow: %ld\n", (long) xHandleblink_task_doFlow);    
    if (xHandleblink_task_doFlow)
    {
        vTaskDelete(xHandleblink_task_doFlow);
        printf("Killed: xHandleblink_task_doFlow\n");
    }

    printf("Handle: xHandletask_autodoFlow: %ld\n", (long) xHandletask_autodoFlow);  
    if (xHandletask_autodoFlow)
    {
        vTaskDelete(xHandletask_autodoFlow);
        printf("Killed: xHandletask_autodoFlow\n");
    }

}

void doInit(void)
{
    string config = "/sdcard/config/config.ini";   
    printf("Start tfliteflow.InitFlow(config);\n");
    tfliteflow.InitFlow(config);
    printf("Finished tfliteflow.InitFlow(config);\n");
}


bool doflow(void)
{
    
    std::string zw_time = gettimestring(LOGFILE_TIME_FORMAT);
    printf("doflow - start %s\n", zw_time.c_str());
    flowisrunning = true;
    tfliteflow.doFlow(zw_time);
    flowisrunning = false;

    printf("doflow - end %s\n", zw_time.c_str());
    return true;
}

void blink_task_doFlow(void *pvParameter)
{
    printf("blink_task_doFlow\n");
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
    LogFile.WriteToFile("handler_init"); 
    printf("handler_doinit uri:\n"); printf(req->uri); printf("\n");

    char* resp_str = "Init started<br>";
    httpd_resp_send(req, resp_str, strlen(resp_str));     

    doInit();

    resp_str = "Init done<br>";
    httpd_resp_send(req, resp_str, strlen(resp_str));     
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);    

    return ESP_OK;
};

esp_err_t handler_doflow(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_doflow");   
    char* resp_str;

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
    resp_str = "doFlow gestartet - dauert ca. 60 Sekunden";
    httpd_resp_send(req, resp_str, strlen(resp_str));  
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);        
    return ESP_OK;
};




esp_err_t handler_wasserzaehler(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_wasserzaehler");    
    bool _rawValue = false;
    bool _noerror = false;
    string zw;

    printf("handler_wasserzaehler uri:\n"); printf(req->uri); printf("\n");

    char _query[100];
    char _size[10];

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
//        printf("Query: "); printf(_query); printf("\n");
        if (httpd_query_key_value(_query, "rawvalue", _size, 10) == ESP_OK)
        {
            printf("rawvalue is found"); printf(_size); printf("\n"); 
            _rawValue = true;
        }
        if (httpd_query_key_value(_query, "noerror", _size, 10) == ESP_OK)
        {
            printf("noerror is found"); printf(_size); printf("\n"); 
            _noerror = true;
        }        
    }  

    zw = tfliteflow.getReadout(_rawValue, _noerror);
    if (zw.length() > 0)
        httpd_resp_sendstr_chunk(req, zw.c_str()); 

    string query = std::string(_query);
//    printf("Query: %s\n", query.c_str());
    if (query.find("full") != std::string::npos)
    {
        string txt, zw;
        
        txt = "<p>Aligned Image: <p><img src=\"/img_tmp/alg.jpg\"> <p>\n";
        txt = txt + "Digital Counter: <p> ";
        httpd_resp_sendstr_chunk(req, txt.c_str()); 
        
        std::vector<HTMLInfo*> htmlinfo;
        htmlinfo = tfliteflow.GetAllDigital();
        for (int i = 0; i < htmlinfo.size(); ++i)
        {
            if (htmlinfo[i]->val == 10)
                zw = "NaN";
            else
            {
                zw = to_string((int) htmlinfo[i]->val);
            }
            txt = "<img src=\"/img_tmp/" +  htmlinfo[i]->filename + "\"> " + zw;
            httpd_resp_sendstr_chunk(req, txt.c_str()); 
            delete htmlinfo[i];
        }
        htmlinfo.clear();
      
        txt = " <p> Analog Meter: <p> ";
        httpd_resp_sendstr_chunk(req, txt.c_str()); 
        
        htmlinfo = tfliteflow.GetAllAnalog();
        for (int i = 0; i < htmlinfo.size(); ++i)
        {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(1) << htmlinfo[i]->val;
            zw = stream.str();

            txt = "<img src=\"/img_tmp/" +  htmlinfo[i]->filename + "\"> " + zw;
            httpd_resp_sendstr_chunk(req, txt.c_str()); 
            delete htmlinfo[i];
        }
        htmlinfo.clear();         

    }   

  




    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);   

    return ESP_OK;
};


esp_err_t handler_editflow(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_editflow");    

    printf("handler_editflow uri: "); printf(req->uri); printf("\n");

    char _query[200];
    char _valuechar[30];
    string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
            printf("task is found: %s\n", _valuechar); 
            _task = string(_valuechar);
        }
    }  

    if (_task.compare("copy") == 0)
    {
        string in, out, zw;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);
        printf("in: "); printf(in.c_str()); printf("\n"); 

        httpd_query_key_value(_query, "out", _valuechar, 30);         
        out = string(_valuechar);  
        printf("out: "); printf(out.c_str()); printf("\n"); 

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
        printf("in: "); printf(in.c_str()); printf("\n"); 

        httpd_query_key_value(_query, "out", _valuechar, 30);         
        out = string(_valuechar);  
        printf("out: "); printf(out.c_str()); printf("\n"); 

        httpd_query_key_value(_query, "x", _valuechar, 30);
        zw = string(_valuechar);  
        x = stoi(zw);              
        printf("x: "); printf(zw.c_str()); printf("\n"); 

        httpd_query_key_value(_query, "y", _valuechar, 30);
        zw = string(_valuechar);  
        y = stoi(zw);              
        printf("y: "); printf(zw.c_str()); printf("\n"); 

        httpd_query_key_value(_query, "dx", _valuechar, 30);
        zw = string(_valuechar);  
        dx = stoi(zw);  
        printf("dx: "); printf(zw.c_str()); printf("\n"); 

        httpd_query_key_value(_query, "dy", _valuechar, 30);
        zw = string(_valuechar);  
        dy = stoi(zw);          
        printf("dy: "); printf(zw.c_str()); printf("\n"); 

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
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK) {
            _host = std::string(_valuechar);
        }
//        printf("Parameter host: "); printf(_host.c_str()); printf("\n"); 
//        string zwzw = "Do " + _task + " start\n"; printf(zwzw.c_str());
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
    if (_task.compare("test_analog") == 0)
    {
        std::string _host = "";
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK) {
            _host = std::string(_valuechar);
        }
//        printf("Parameter host: "); printf(_host.c_str()); printf("\n"); 
//        string zwzw = "Do " + _task + " start\n"; printf(zwzw.c_str());
        std::string zw = tfliteflow.doSingleStep("[Analog]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str()); 
    }  
    if (_task.compare("test_digits") == 0)
    {
        std::string _host = "";
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK) {
            _host = std::string(_valuechar);
        }
//        printf("Parameter host: "); printf(_host.c_str()); printf("\n"); 

//        string zwzw = "Do " + _task + " start\n"; printf(zwzw.c_str());
        std::string zw = tfliteflow.doSingleStep("[Digits]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str()); 
    } 


    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);   

    return ESP_OK;
};


esp_err_t handler_prevalue(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_prevalue"); 
    const char* resp_str;
    string zw;

//    printf("handler_prevalue:\n"); printf(req->uri); printf("\n");

    char _query[100];
    char _size[10] = "";

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
//        printf("Query: "); printf(_query); printf("\n");
        if (httpd_query_key_value(_query, "value", _size, 10) == ESP_OK)
        {
            printf("Value: "); printf(_size); printf("\n"); 
        }
    }           

    if (strlen(_size) == 0)
        zw = tfliteflow.GetPrevalue();
    else
        zw = "SetPrevalue to " + tfliteflow.UpdatePrevalue(_size);
    
    resp_str = zw.c_str();

    httpd_resp_send(req, resp_str, strlen(resp_str));   
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);      

    return ESP_OK;
};

void task_autodoFlow(void *pvParameter)
{
    int64_t fr_start, fr_delta_ms;

    doInit();
    
    auto_isrunning = tfliteflow.isAutoStart(auto_intervall);
    auto_isrunning = auto_isrunning && (!isSetupModusActive());

    while (auto_isrunning)
    {
        LogFile.WriteToFile("task_autodoFlow - next round"); 
        printf("Autoflow: start\n");
        fr_start = esp_timer_get_time();

        if (flowisrunning)
        {
            printf("Autoflow: doFLow laeuft bereits!\n");
        }
        else
        {
            printf("Autoflow: doFLow wird gestartet\n");
            flowisrunning = true;
            doflow();
            printf("Remove older log files\n");
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
        const TickType_t xDelay = (auto_intervall - fr_delta_ms)  / portTICK_PERIOD_MS;
        printf("Autoflow: sleep for : %ldms\n", (long) xDelay);
        vTaskDelay( xDelay );        
    }
    vTaskDelete(NULL); //Delete this task if it exits from the loop above
    xHandletask_autodoFlow = NULL;
}

void TFliteDoAutoStart()
{
    xTaskCreate(&task_autodoFlow, "task_autodoFlow", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY+1, &xHandletask_autodoFlow);
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

    
    camuri.uri       = "/editflow.html";
    camuri.handler   = handler_editflow;
    camuri.user_ctx  = (void*) "EditFlow"; 
    httpd_register_uri_handler(server, &camuri);     

    camuri.uri       = "/wasserzaehler.html";
    camuri.handler   = handler_wasserzaehler;
    camuri.user_ctx  = (void*) "Wasserzaehler"; 
    httpd_register_uri_handler(server, &camuri);  

}
