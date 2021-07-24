#include "server_camera.h"

#include <string>
#include "string.h"

#include "esp_camera.h"
#include "ClassControllCamera.h"

#include "ClassLogFile.h"

#define SCRATCH_BUFSIZE2  8192 
char scratch2[SCRATCH_BUFSIZE2];

//#define DEBUG_DETAIL_ON   
static const char *TAGPARTCAMERA = "server_camera";


void PowerResetCamera(){
        ESP_LOGD(TAGPARTCAMERA, "Resetting camera by power down line");
        gpio_config_t conf;
        conf.intr_type = GPIO_INTR_DISABLE;
        conf.pin_bit_mask = 1LL << GPIO_NUM_32;
        conf.mode = GPIO_MODE_OUTPUT;
        conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&conf);

        // carefull, logic is inverted compared to reset pin
        gpio_set_level(GPIO_NUM_32, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_32, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
}


esp_err_t handler_lightOn(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_lightOn - Start");
    printf("handler_lightOn uri:\n"); printf(req->uri); printf("\n");
#endif

    Camera.LightOnOff(true);
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));  

#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_lightOn - Done");
#endif

    return ESP_OK;
};

esp_err_t handler_lightOff(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_lightOff - Start");
    printf("handler_lightOff uri:\n"); printf(req->uri); printf("\n");
#endif
    Camera.LightOnOff(false);
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));       

#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_lightOff - Done");
#endif

    return ESP_OK;
};

esp_err_t handler_capture(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_capture - Start");
#endif

    int quality;
    framesize_t res;

    Camera.GetCameraParameter(req, quality, res);

#ifdef DEBUG_DETAIL_ON   
    printf("Size: %d", res); printf(" Quality: %d\n", quality);
#endif

    Camera.SetQualitySize(quality, res);

    esp_err_t ressult;
    ressult = Camera.CaptureToHTTP(req);

#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_capture - Done");
#endif

    return ressult;
};


esp_err_t handler_capture_with_ligth(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON  
    LogFile.WriteHeapInfo("handler_capture_with_ligth - Start");
#endif
    char _query[100];
    char _delay[10];

    int quality;
    framesize_t res;    
    int delay = 2500;

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
        printf("Query: "); printf(_query); printf("\n");
        if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON   
            printf("Delay: "); printf(_delay); printf("\n");    
#endif        
            delay = atoi(_delay);

            if (delay < 0)
                delay = 0;
        }
    };

    Camera.GetCameraParameter(req, quality, res);

#ifdef DEBUG_DETAIL_ON   
    printf("Size: %d", res); printf(" Quality: %d\n", quality);
#endif

    Camera.SetQualitySize(quality, res);
    Camera.LightOnOff(true);
    const TickType_t xDelay = delay / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );

    esp_err_t ressult;
    ressult = Camera.CaptureToHTTP(req);  

    Camera.LightOnOff(false);

#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_capture_with_ligth - Done");
#endif

    return ressult;
};



esp_err_t handler_capture_save_to_file(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_capture_save_to_file - Start");
#endif

    char _query[100];
    char _delay[10];
    int delay = 0;
    char filename[100];
    std::string fn = "/sdcard/";


    int quality;
    framesize_t res;    

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
        printf("Query: "); printf(_query); printf("\n");
        if (httpd_query_key_value(_query, "filename", filename, 100) == ESP_OK)
        {
            fn.append(filename);
#ifdef DEBUG_DETAIL_ON   
            printf("Filename: "); printf(fn.c_str()); printf("\n");            
#endif
        }
        else
            fn.append("noname.jpg");

        if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON   
            printf("Delay: "); printf(_delay); printf("\n");            
#endif
            delay = atoi(_delay);

            if (delay < 0)
                delay = 0;
        }

    }
    else
        fn.append("noname.jpg");

    Camera.GetCameraParameter(req, quality, res);
#ifdef DEBUG_DETAIL_ON   
    printf("Size: %d", res); printf(" Quality: %d\n", quality);
#endif
    Camera.SetQualitySize(quality, res);

    esp_err_t ressult;
    ressult = Camera.CaptureToFile(fn, delay);  

    const char* resp_str = (const char*) fn.c_str();
    httpd_resp_send(req, resp_str, strlen(resp_str));  
  
#ifdef DEBUG_DETAIL_ON   
    LogFile.WriteHeapInfo("handler_capture_save_to_file - Done");
#endif

    return ressult;
};



void register_server_camera_uri(httpd_handle_t server)
{
#ifdef DEBUG_DETAIL_ON   
    ESP_LOGI(TAGPARTCAMERA, "server_part_camera - Registering URI handlers");
#endif

    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;

    camuri.uri       = "/lighton";
    camuri.handler   = handler_lightOn;
    camuri.user_ctx  = (void*) "Light On";    
    httpd_register_uri_handler(server, &camuri);

    camuri.uri       = "/lightoff";
    camuri.handler   = handler_lightOff;
    camuri.user_ctx  = (void*) "Light Off"; 
    httpd_register_uri_handler(server, &camuri);    

    camuri.uri       = "/capture";
    camuri.handler   = handler_capture;
    camuri.user_ctx  = NULL; 
    httpd_register_uri_handler(server, &camuri);      

    camuri.uri       = "/capture_with_flashlight";
    camuri.handler   = handler_capture_with_ligth;
    camuri.user_ctx  = NULL; 
    httpd_register_uri_handler(server, &camuri);  

    camuri.uri       = "/save";
    camuri.handler   = handler_capture_save_to_file;
    camuri.user_ctx  = NULL; 
    httpd_register_uri_handler(server, &camuri);    
}
