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
#include <vector>
//#include <regex>

#include "server_GPIO.h"

#include "ClassLogFile.h"
#include "configFile.h"
#include "Helper.h"

// #define DEBUG_DETAIL_ON 

GpioPin::GpioPin(gpio_num_t gpio, const char* name, gpio_pin_mode_t mode, gpio_int_type_t interruptType, uint8_t dutyResolution, bool mqttEnable, bool httpEnable) 
{
    _gpio = gpio;
    _name = name; 
    _mode = mode;
    _interruptType = interruptType;    

    //initGPIO();
}

GpioPin::~GpioPin()
{
    printf("reset GPIO pin %d", _gpio);
    gpio_reset_pin(_gpio);
}

void GpioPin::init()
{
    gpio_config_t io_conf;
    //set interrupt
    io_conf.intr_type = _interruptType;
    //set as output mode
    io_conf.mode = _mode == gpio_pin_mode_t::GPIO_PIN_MODE_OUTPUT ? gpio_mode_t::GPIO_MODE_OUTPUT : gpio_mode_t::GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << _gpio);
    //set pull-down mode
    io_conf.pull_down_en = _mode == gpio_pin_mode_t::GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
    //set pull-up mode
    io_conf.pull_up_en = _mode == gpio_pin_mode_t::GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pullup_t::GPIO_PULLUP_ENABLE : gpio_pullup_t::GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

bool GpioPin::getValue(std::string* errorText)
{   
    if ((_mode != GPIO_PIN_MODE_INPUT) && (_mode != GPIO_PIN_MODE_INPUT_PULLUP) && (_mode != GPIO_PIN_MODE_INPUT_PULLDOWN)) {
        (*errorText) = "GPIO is not in input mode";
    }

    return gpio_get_level(_gpio) == 1;
}

void GpioPin::setValue(bool value, std::string* errorText)
{   
    if ((_mode != GPIO_PIN_MODE_OUTPUT) && (_mode != GPIO_PIN_MODE_OUTPUT_PWM)) {
        (*errorText) = "GPIO is not in output mode";
    }

    gpio_set_level(_gpio, value);
}


esp_err_t callHandleHttpRequest(httpd_req_t *req)
{
    printf("callHandleHttpRequest\n");
    GpioHandler *gpioHandler = (GpioHandler*)req->user_ctx;
    return gpioHandler->handleHttpRequest(req);
}

void taskGpioHandler(void *pvParameter)
{
    printf("taskGpioHandler\n");
    ((GpioHandler*)pvParameter)->init();
}

GpioHandler::GpioHandler(std::string configFile, httpd_handle_t httpServer) 
{
    printf("---- start GpioHandler ----\n");
    _configFile = configFile;
    printf("-1-\n");
    _httpServer = httpServer;
    printf("-2-\n");

    printf("register GPIO Uri\n");
    registerGpioUri();

    //xTaskCreate((TaskFunction_t)&taskGpioHandler, "taskGpioHandler", configMINIMAL_STACK_SIZE * 64, this, tskIDLE_PRIORITY+1, &xHandletaskGpioHandler);
}

GpioHandler::~GpioHandler()  {
    if (gpioMap != NULL) {
        clear();
        delete gpioMap;
    }
}

void GpioHandler::init()
{
    printf("freemem -3.1-: %u\n", esp_get_free_heap_size());
    // TickType_t xDelay = 60000 / portTICK_PERIOD_MS;
    // printf("wait before start %ldms\n", (long) xDelay);
    // vTaskDelay( xDelay );

    if (gpioMap == NULL) {
        gpioMap = new std::map<gpio_num_t, GpioPin*>();
    } else {
        clear();
    }
    
    printf("read GPIO config and init GPIO\n");
    printf("freemem -3.2-: %u\n", esp_get_free_heap_size());
    readConfig();
    printf("freemem -3.3-: %u\n", esp_get_free_heap_size());

    for(std::map<gpio_num_t, GpioPin*>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it) {
        it->second->init();
    }
    printf("freemem -3.4-: %u\n", esp_get_free_heap_size());

    printf("GPIO init comleted\n");
}

bool GpioHandler::readConfig() 
{
    if (!gpioMap->empty())
        clear();

    ConfigFile configFile = ConfigFile(_configFile); 

    std::vector<std::string> zerlegt;
    std::string line = "";
    bool disabledLine = false;
    bool eof = false;
        
    while ((!configFile.GetNextParagraph(line, disabledLine, eof) || (line.compare("[GPIO]") != 0)) && !disabledLine && !eof) {}
    if (eof)
        return false;

    while (configFile.getNextLine(&line, disabledLine, eof) && !configFile.isNewParagraph(line))
    {
        zerlegt = configFile.ZerlegeZeile(line);
        // const std::regex pieces_regex("IO([0-9]{1,2})");
        // std::smatch pieces_match;
        // if (std::regex_match(zerlegt[0], pieces_match, pieces_regex) && (pieces_match.size() == 2))
        // {
        //     std::string gpioStr = pieces_match[1];
        if (zerlegt[0].rfind("IO", 0) == 0)
        {
            printf("Enable GP%s in %s mode\n", zerlegt[0].c_str(), zerlegt[1].c_str());
            std::string gpioStr = zerlegt[0].substr(2, 2);
            gpio_num_t gpioNr = (gpio_num_t)atoi(gpioStr.c_str());
            gpio_pin_mode_t pinMode = resolvePinMode(toLower(zerlegt[1]));
            gpio_int_type_t intType = resolveIntType(toLower(zerlegt[2]));
            uint16_t dutyResolution = (uint8_t)atoi(zerlegt[3].c_str());
            bool mqttEnabled = toLower(zerlegt[4]) == "true";
            bool httpEnabled = toLower(zerlegt[5]) == "true";
            (*gpioMap)[gpioNr] = new GpioPin(gpioNr, zerlegt[6].c_str(), pinMode, intType,dutyResolution, mqttEnabled, httpEnabled);
        }
    }

    return true;
}

void GpioHandler::clear() 
{
    for(std::map<gpio_num_t, GpioPin*>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it) {
        delete it->second;
    }
    gpioMap->clear();
}
 
void GpioHandler::registerGpioUri() 
{
    ESP_LOGI(TAGPARTGPIO, "server_GPIO - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;
    camuri.uri       = "/GPIO";
    camuri.handler   = callHandleHttpRequest;
    camuri.user_ctx  = (void*)this;    
    httpd_register_uri_handler(_httpServer, &camuri);
}

esp_err_t GpioHandler::handleHttpRequest(httpd_req_t *req)
{
    printf("handleHttpRequest\n");

#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_switch_GPIO - Start");    
#endif

    LogFile.WriteToFile("handler_switch_GPIO");    
    char _query[200];
    char _valueGPIO[30];    
    char _valueStatus[30];    
    std::string gpio, status;
    printf("-1-\n");

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK) {
        printf("Query: "); printf(_query); printf("\n");
        
        if (httpd_query_key_value(_query, "GPIO", _valueGPIO, 30) == ESP_OK)
        {
            printf("GPIO is found "); printf(_valueGPIO); printf("\n"); 
            gpio = std::string(_valueGPIO);
        } else {
            std::string resp_str = "GPIO No is not defined";
            httpd_resp_send(req, resp_str.c_str(), resp_str.length());    
            return ESP_OK;
        }
        if (httpd_query_key_value(_query, "Status", _valueStatus, 30) == ESP_OK)
        {
            printf("Status is found "); printf(_valueStatus); printf("\n"); 
            status = std::string(_valueStatus);
        }
    } else {
        char* resp_str = "Error in call. Use /GPIO?GPIO=12&Status=high";
        httpd_resp_send(req, resp_str, strlen(resp_str));    
        return ESP_OK;
    }
    printf("-2-\n");

    status = toUpper(status);
    if ((status != "HIGH") && (status != "LOW") && (status != "TRUE") && (status != "FALSE") && (status != "0") && (status != "1") && (status != ""))
    {
        std::string zw = "Status not valid: " + status;
        httpd_resp_sendstr_chunk(req, zw.c_str());
        httpd_resp_sendstr_chunk(req, NULL);          
        return ESP_OK;    
    }
    printf("-3-\n");

    int gpionum = stoi(gpio);

    // frei: 16; 12-15; 2; 4  // nur 12 und 13 funktionieren 2: reboot, 4: BlitzLED, 15: PSRAM, 14/15: DMA für SDKarte ???
    gpio_num_t gpio_num = resolvePinNr(gpionum);
    if (gpio_num == GPIO_NUM_NC)
    {
        std::string zw = "GPIO" + std::to_string(gpionum) + " not support - only 12 & 13 free";
            httpd_resp_sendstr_chunk(req, zw.c_str());
            httpd_resp_sendstr_chunk(req, NULL);          
            return ESP_OK;
    }

    if (gpioMap->count(gpio_num) == 0) {
        char resp_str [30];
        sprintf(resp_str, "GPIO%d is not registred", gpio_num);
        httpd_resp_send(req, resp_str, strlen(resp_str));  
        return ESP_OK;     
    }
    
    printf("-4-\n");

    if (status == "") 
    {
        std::string resp_str = "";
        status = (*gpioMap)[gpio_num]->getValue(&resp_str) ? "HIGH" : "LOW";
        if (resp_str == "") {
            resp_str = status;
        }
        httpd_resp_sendstr_chunk(req, resp_str.c_str());
        httpd_resp_sendstr_chunk(req, NULL);
    }
    else
    {
        std::string resp_str = "";
        (*gpioMap)[gpio_num]->setValue((status == "HIGH") || (status == "TRUE") || (status == "1"), &resp_str);
        if (resp_str == "") {
            resp_str = "GPIO" + std::to_string(gpionum) + " switched to " + status;
        }
        httpd_resp_sendstr_chunk(req, resp_str.c_str());
        httpd_resp_sendstr_chunk(req, NULL);
    }
    printf("-5-\n");
          
    return ESP_OK;    
};

gpio_num_t GpioHandler::resolvePinNr(uint8_t pinNr) 
{
    switch(pinNr)  {
        case 0:
            return GPIO_NUM_0;
        case 1:
            return GPIO_NUM_1;
        case 3:
            return GPIO_NUM_3;
        case 4:
            return GPIO_NUM_4;
        case 12:
            return GPIO_NUM_12;
        case 13:
            return GPIO_NUM_13;
        default: 
            return GPIO_NUM_NC;   
    }
}

gpio_pin_mode_t GpioHandler::resolvePinMode(std::string input) 
{
    if( input == "disabled" ) return GPIO_PIN_MODE_DISABLED;
    if( input == "input" ) return GPIO_PIN_MODE_INPUT;
    if( input == "input-pullup" ) return GPIO_PIN_MODE_INPUT_PULLUP;
    if( input == "input-pulldown" ) return GPIO_PIN_MODE_INPUT_PULLDOWN;
    if( input == "output" ) return GPIO_PIN_MODE_OUTPUT;
    if( input == "output-pwm" ) return GPIO_PIN_MODE_OUTPUT_PWM;
    if( input == "external-flash-pwm" ) return GPIO_PIN_MODE_EXTERNAL_FLASH_PWM;
    if( input == "external-flash-ws281x" ) return GPIO_PIN_MODE_EXTERNAL_FLASH_WS281X;

    return GPIO_PIN_MODE_DISABLED;
}

gpio_int_type_t GpioHandler::resolveIntType(std::string input) 
{
    if( input == "disabled" ) return GPIO_INTR_DISABLE;
    if( input == "rising-edge" ) return GPIO_INTR_POSEDGE;
    if( input == "falling-edge" ) return GPIO_INTR_NEGEDGE;
    if( input == "rising-and-falling" ) return GPIO_INTR_ANYEDGE ;
    if( input == "low-level-trigger" ) return GPIO_INTR_LOW_LEVEL;
    if( input == "high-level-trigger" ) return GPIO_INTR_HIGH_LEVEL;


    return GPIO_INTR_DISABLE;
}
