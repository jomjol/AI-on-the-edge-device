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

// #define DEBUG_DETAIL_ON 

esp_err_t handler_switch_GPIO(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_switch_GPIO - Start");    
#endif

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
    
    // frei: 16; 12-15; 2; 4  // nur 12 und 13 funktionieren 2: reboot, 4: BlitzLED, 14/15: DMA für SDKarte ???

    switch (gpionum) {
        case 12:
            gpio_num = GPIO_NUM_12;
            break;
        case 13:
            gpio_num = GPIO_NUM_13;
            break;
        default:
            zw = "GPIO" + std::to_string(gpionum) + " not support - only 12 & 13 free";
            httpd_resp_sendstr_chunk(req, zw.c_str());
            httpd_resp_sendstr_chunk(req, NULL);          
            return ESP_OK;    
    }

    if (status == "HIGH")  
        gpio_set_level(gpio_num, 1);
    else
        gpio_set_level(gpio_num, 0); 


    zw = "GPIO" + std::to_string(gpionum) + " switched to " + status;
    httpd_resp_sendstr_chunk(req, zw.c_str());
    httpd_resp_sendstr_chunk(req, NULL);          
    return ESP_OK;    
};

void initGPIO()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
//    io_conf.pin_bit_mask = ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1));
//    io_conf.pin_bit_mask = ((1ULL << GPIO_NUM_12) | (1ULL << GPIO_NUM_2) | (1ULL << GPIO_NUM_4) | (1ULL << GPIO_NUM_12) | (1ULL << GPIO_NUM_13) | (1ULL << GPIO_NUM_14) | (1ULL << GPIO_NUM_15));
    io_conf.pin_bit_mask = ((1ULL << GPIO_NUM_12) | (1ULL << GPIO_NUM_13));
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t) 0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}


void register_server_GPIO_uri(httpd_handle_t server)
{
    ESP_LOGI(TAGPARTGPIO, "server_GPIO - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;
    camuri.uri       = "/GPIO";
    camuri.handler   = handler_switch_GPIO;
    camuri.user_ctx  = (void*) "switch GPIO";    
    httpd_register_uri_handler(server, &camuri);

    initGPIO();
}
