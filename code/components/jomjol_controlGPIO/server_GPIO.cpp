#include <string>
#include "string.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
//#include "errno.h"

#include <sys/stat.h>

#include "server_GPIO.h"

#include "ClassLogFile.h"

#include "Helper.h"

esp_err_t handler_switch_GPIO(httpd_req_t *req)
{
    if (debug_detail_heap) LogFile.WriteHeapInfo("handler_switch_GPIO - Start");    

    LogFile.WriteToFile("handler_switch_GPIO");    
    char _query[200];
    char _valueGPIO[30];    
    char _valueStatus[30];    
    std::string gpio, status, zw;
    int gpionum = 0;
    gpio_num_t gpio_num;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        printf("Query: "); printf(_query); printf("\n");
        
        if (httpd_query_key_value(_query, "GPIO", _valueGPIO, 30) == ESP_OK)
        {
            printf("GPIO is found"); printf(_valueGPIO); printf("\n"); 
            gpio = std::string(_valueGPIO);
        }
        if (httpd_query_key_value(_query, "Status", _valueStatus, 30) == ESP_OK)
        {
            printf("Status is found"); printf(_valueStatus); printf("\n"); 
            status = std::string(_valueStatus);
        }
    };

    status = toUpper(status);
    if (!(status == "HIGH") && !(status == "LOW"))
    {
        zw = "Status not valid: " + status;;
        httpd_resp_sendstr_chunk(req, zw.c_str());
        httpd_resp_sendstr_chunk(req, NULL);          
        return ESP_OK;    
    }

    gpionum = stoi(gpio);
    
    // frei: 16; 12-15; 2; 4

    switch (gpionum) {
        case 2:
            gpio_num = GPIO_NUM_2;
            break;
        case 4:
            gpio_num = GPIO_NUM_4;
            break;
        case 12:
            gpio_num = GPIO_NUM_12;
            break;
        case 13:
            gpio_num = GPIO_NUM_13;
            break;
        case 14:
            gpio_num = GPIO_NUM_14;
            break;
        case 15:
            gpio_num = GPIO_NUM_15;
            break;
        case 16:
            gpio_num = (gpio_num_t) 16;
            break;
        default:
            zw = "GPIO" + std::to_string(gpionum) + " not support - only 2, 4, 12-16 free";
            httpd_resp_sendstr_chunk(req, zw.c_str());
            httpd_resp_sendstr_chunk(req, NULL);          
            return ESP_OK;    
    }

	// Init the GPIO
    gpio_pad_select_gpio(gpio_num);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);  

    if (status == "HIGH")  
        gpio_set_level(gpio_num, 1);
    else
        gpio_set_level(gpio_num, 0); 

    zw = "GPIO" + std::to_string(gpionum) + " switched to " + status;
    httpd_resp_sendstr_chunk(req, zw.c_str());
    httpd_resp_sendstr_chunk(req, NULL);          
    return ESP_OK;    
};



void register_server_GPIO_uri(httpd_handle_t server)
{
    ESP_LOGI(TAGPARTGPIO, "server_GPIO - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;
    camuri.uri       = "/GPIO";
    camuri.handler   = handler_switch_GPIO;
    camuri.user_ctx  = (void*) "switch GPIO";    
    httpd_register_uri_handler(server, &camuri);
}
