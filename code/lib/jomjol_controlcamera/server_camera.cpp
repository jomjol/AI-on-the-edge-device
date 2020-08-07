#include "server_camera.h"

#include <string>
#include "string.h"

#include "esp_camera.h"
#include "ClassControllCamera.h"

#include "ClassLogFile.h"

#define SCRATCH_BUFSIZE2  8192 
char scratch2[SCRATCH_BUFSIZE2];


esp_err_t handler_lightOn(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_lightOn");
    printf("handler_lightOn uri:\n"); printf(req->uri); printf("\n");
    Camera.LightOnOff(true);
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));      
    return ESP_OK;
};

esp_err_t handler_lightOff(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_lightOff");
    printf("handler_lightOff uri:\n"); printf(req->uri); printf("\n");
    Camera.LightOnOff(false);
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));       
    return ESP_OK;
};

esp_err_t handler_capture(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_capture");
    int quality;
    framesize_t res;

    Camera.GetCameraParameter(req, quality, res);
    printf("Size: %d", res); printf(" Quality: %d\n", quality);
    Camera.SetQualitySize(quality, res);

    esp_err_t ressult;
    ressult = Camera.CaptureToHTTP(req);
    return ressult;
};


esp_err_t handler_capture_with_ligth(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_capture_with_ligth");
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
            printf("Delay: "); printf(_delay); printf("\n");            
            delay = atoi(_delay);

            if (delay < 0)
                delay = 0;
        }
    };

    Camera.GetCameraParameter(req, quality, res);
    printf("Size: %d", res); printf(" Quality: %d\n", quality);
    Camera.SetQualitySize(quality, res);

    Camera.LightOnOff(true);
    const TickType_t xDelay = delay / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );

    esp_err_t ressult;
    ressult = Camera.CaptureToHTTP(req);  

    Camera.LightOnOff(false);
   
    return ressult;
};



esp_err_t handler_capture_save_to_file(httpd_req_t *req)
{
    LogFile.WriteToFile("handler_capture_save_to_file");
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
            printf("Filename: "); printf(fn.c_str()); printf("\n");            
        }
        else
            fn.append("noname.jpg");

        if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
        {
            printf("Delay: "); printf(_delay); printf("\n");            
            delay = atoi(_delay);

            if (delay < 0)
                delay = 0;
        }

    }
    else
        fn.append("noname.jpg");

    Camera.GetCameraParameter(req, quality, res);
    printf("Size: %d", res); printf(" Quality: %d\n", quality);
    Camera.SetQualitySize(quality, res);

    esp_err_t ressult;
    ressult = Camera.CaptureToFile(fn, delay);  

    const char* resp_str = (const char*) fn.c_str();
    httpd_resp_send(req, resp_str, strlen(resp_str));  
  
    return ressult;
};



void register_server_camera_uri(httpd_handle_t server)
{
    ESP_LOGI(TAGPARTCAMERA, "server_part_camera - Registering URI handlers");
    
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
