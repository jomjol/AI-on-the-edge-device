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

GpioPin::GpioPin(gpio_num_t gpio, const char* name, gpio_pin_mode_t mode, gpio_int_type_t interruptType) 
{
    _gpio = gpio;
    _name = name; 
    _mode = mode;
    _interruptType = interruptType;    

    initGPIO();
}

void GpioPin::initGPIO()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = _interruptType;
    //set as output mode
    io_conf.mode = _mode == gpio_pin_mode_t::GPIO_PIN_MODE_OUTPUT ? gpio_mode_t::GPIO_MODE_OUTPUT : gpio_mode_t::GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << _gpio);
    //disable pull-down mode
    io_conf.pull_down_en = _mode == gpio_pin_mode_t::GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = _mode == gpio_pin_mode_t::GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pullup_t::GPIO_PULLUP_ENABLE : gpio_pullup_t::GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void GpioPin::setValue(bool value)
{   
    gpio_set_level(_gpio, value);
}

esp_err_t callHandleHttpRequest(httpd_req_t *req)
{
    return ((GpioHandler*)req->user_ctx)->handleHttpRequest(req);
}

GpioHandler::GpioHandler(std::string configFile, httpd_handle_t server) 
{
    _configFile = configFile;

    gpioMap = new std::map<gpio_num_t, GpioPin*>();

    readConfig();
    registerGpioUri(server);
}

bool GpioHandler::readConfig() 
{
    ConfigFile configFile(_configFile); 
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
            std::string gpioStr = zerlegt[0].substr(2, 2);
            gpio_num_t gpioNr = (gpio_num_t)atoi(gpioStr.c_str());
            gpio_pin_mode_t pinMode = resolvePinMode(zerlegt[1]);
            gpio_int_type_t intType = resolveIntType(zerlegt[2]);
            (*gpioMap)[gpio_num_t::GPIO_NUM_16] = new GpioPin(gpioNr, zerlegt[3].c_str(), pinMode, intType);
        
        }
    }

    return true;
}

 
void GpioHandler::registerGpioUri(httpd_handle_t server) {
    ESP_LOGI(TAGPARTGPIO, "server_GPIO - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;
    camuri.uri       = "/GPIO";
    camuri.handler   = callHandleHttpRequest;
    camuri.user_ctx  = (void*) this;    
    httpd_register_uri_handler(server, &camuri);
}

IRAM_ATTR esp_err_t GpioHandler::handleHttpRequest(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_switch_GPIO - Start");    
#endif

    LogFile.WriteToFile("handler_switch_GPIO");    
    char _query[200];
    char _valueGPIO[30];    
    char _valueStatus[30];    
    std::string gpio, status, zw;
    
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

    int gpionum = stoi(gpio);
    
    // frei: 16; 12-15; 2; 4  // nur 12 und 13 funktionieren 2: reboot, 4: BlitzLED, 14/15: DMA für SDKarte ???
    gpio_num_t gpio_num = resolvePinNr(gpionum);
    if (gpio_num == GPIO_NUM_NC)
    {
        zw = "GPIO" + std::to_string(gpionum) + " not support - only 12 & 13 free";
            httpd_resp_sendstr_chunk(req, zw.c_str());
            httpd_resp_sendstr_chunk(req, NULL);          
            return ESP_OK;
    }

    (*gpioMap)[gpio_num]->setValue((status == "HIGH") || (status == "TRUE") || (status == "1"));

    zw = "GPIO" + std::to_string(gpionum) + " switched to " + status;
    httpd_resp_sendstr_chunk(req, zw.c_str());
    httpd_resp_sendstr_chunk(req, NULL);          
    return ESP_OK;    
};

gpio_num_t GpioHandler::resolvePinNr(uint8_t pinNr) 
{
    switch(pinNr)  {
        case 0:
            return GPIO_NUM_0;
        case 4:
            return GPIO_NUM_4;
        case 12:
            return GPIO_NUM_12;
        case 13:
            return GPIO_NUM_13;
        case 16:
            return GPIO_NUM_16;
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
    if( input == "input-output" ) return GPIO_PIN_MODE_OUTPUT;
    if( input == "input-output-pwm" ) return GPIO_PIN_MODE_OUTPUT_PWM;

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
