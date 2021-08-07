#include <string>
#include <functional>
#include "string.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"

#include "server_tflite.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
//#include "errno.h"

#include <sys/stat.h>
#include <vector>
//#include <regex>

#include "defines.h"

#include "server_GPIO.h"

#include "ClassLogFile.h"
#include "configFile.h"
#include "Helper.h"
#include "interface_mqtt.h"

static const char *TAG_SERVERGPIO = "server_GPIO";
QueueHandle_t gpio_queue_handle = NULL;

#define DEBUG_DETAIL_ON 

GpioPin::GpioPin(gpio_num_t gpio, const char* name, gpio_pin_mode_t mode, gpio_int_type_t interruptType, uint8_t dutyResolution, std::string mqttTopic, bool httpEnable) 
{
    _gpio = gpio;
    _name = name; 
    _mode = mode;
    _interruptType = interruptType;    
    _mqttTopic = mqttTopic;
}

GpioPin::~GpioPin()
{
    ESP_LOGD(TAG_SERVERGPIO,"reset GPIO pin %d", _gpio);
    if (_interruptType != GPIO_INTR_DISABLE) {
        //hook isr handler for specific gpio pin
        gpio_isr_handler_remove(_gpio);
    }
    gpio_reset_pin(_gpio);
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    GpioResult gpioResult;
    gpioResult.gpio = *(gpio_num_t*) arg;
    gpioResult.value = gpio_get_level(gpioResult.gpio);
    BaseType_t ContextSwitchRequest = pdFALSE;
 
    xQueueSendToBackFromISR(gpio_queue_handle,(void*)&gpioResult,&ContextSwitchRequest);
   
    if(ContextSwitchRequest){
        taskYIELD();
    }
}

static void gpioHandlerTask(void *arg) {
    ESP_LOGD(TAG_SERVERGPIO,"start interrupt task");
    while(1){
        if(uxQueueMessagesWaiting(gpio_queue_handle)){
            while(uxQueueMessagesWaiting(gpio_queue_handle)){
                GpioResult gpioResult;
                xQueueReceive(gpio_queue_handle,(void*)&gpioResult,10);
                ESP_LOGD(TAG_SERVERGPIO,"gpio: %d state: %d", gpioResult.gpio, gpioResult.value);
                ((GpioHandler*)arg)->gpioInterrupt(&gpioResult);
            }  
        }

        ((GpioHandler*)arg)->taskHandler();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void GpioPin::gpioInterrupt(int value) {
    if (_mqttTopic != "") {
        ESP_LOGD(TAG_SERVERGPIO, "gpioInterrupt %s %d", _mqttTopic.c_str(), value);

        MQTTPublish(_mqttTopic, value ? "true" : "false");
        currentState = value;
    }
}

void GpioPin::init()
{
    gpio_config_t io_conf;
    //set interrupt
    io_conf.intr_type = _interruptType;
    //set as output mode
    io_conf.mode = (_mode == GPIO_PIN_MODE_OUTPUT) || (_mode == GPIO_PIN_MODE_BUILT_IN_FLASH_LED) ? gpio_mode_t::GPIO_MODE_OUTPUT : gpio_mode_t::GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << _gpio);
    //set pull-down mode
    io_conf.pull_down_en = _mode == GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
    //set pull-up mode
    io_conf.pull_up_en = _mode == GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pullup_t::GPIO_PULLUP_ENABLE : gpio_pullup_t::GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    if (_interruptType != GPIO_INTR_DISABLE) {
        //hook isr handler for specific gpio pin
        ESP_LOGD(TAG_SERVERGPIO, "GpioPin::init add isr handler for GPIO %d\r\n", _gpio);
        gpio_isr_handler_add(_gpio, gpio_isr_handler, (void*)&_gpio);
    }

    if ((_mqttTopic != "") && ((_mode == GPIO_PIN_MODE_OUTPUT) || (_mode == GPIO_PIN_MODE_OUTPUT_PWM) || (_mode == GPIO_PIN_MODE_BUILT_IN_FLASH_LED))) {
        std::function<bool(std::string, char*, int)> f = std::bind(&GpioPin::handleMQTT, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        MQTTregisterSubscribeFunction(_mqttTopic, f);
    }
}

bool GpioPin::getValue(std::string* errorText)
{   
    if ((_mode != GPIO_PIN_MODE_INPUT) && (_mode != GPIO_PIN_MODE_INPUT_PULLUP) && (_mode != GPIO_PIN_MODE_INPUT_PULLDOWN)) {
        (*errorText) = "GPIO is not in input mode";
    }

    return gpio_get_level(_gpio) == 1;
}

void GpioPin::setValue(bool value, gpio_set_source setSource, std::string* errorText)
{
    ESP_LOGD(TAG_SERVERGPIO, "GpioPin::setValue %d\r\n", value);

    if ((_mode != GPIO_PIN_MODE_OUTPUT) && (_mode != GPIO_PIN_MODE_OUTPUT_PWM) && (_mode != GPIO_PIN_MODE_BUILT_IN_FLASH_LED)) {
        (*errorText) = "GPIO is not in output mode";
    } else {
        gpio_set_level(_gpio, value);

        if ((_mqttTopic != "") && (setSource != GPIO_SET_SOURCE_MQTT)) {
            MQTTPublish(_mqttTopic, value ? "true" : "false");
        }
    }
}

void GpioPin::publishState() {
    int newState = gpio_get_level(_gpio);
    if (newState != currentState) {
        ESP_LOGD(TAG_SERVERGPIO,"publish state of GPIO %d new state %d", _gpio, newState);
        MQTTPublish(_mqttTopic, newState ? "true" : "false");
        currentState = newState;
    }
}

bool GpioPin::handleMQTT(std::string, char* data, int data_len) {
    ESP_LOGD(TAG_SERVERGPIO, "GpioPin::handleMQTT data %.*s\r\n", data_len, data);

    std::string dataStr(data, data_len);
    dataStr = toLower(dataStr);
    std::string errorText = "";
    if ((dataStr == "true") || (dataStr == "1")) {
        setValue(true, GPIO_SET_SOURCE_MQTT, &errorText);
    } else if ((dataStr == "false") || (dataStr == "0")) {
        setValue(false, GPIO_SET_SOURCE_MQTT, &errorText);    
    } else {
        errorText = "wrong value ";
        errorText.append(data, data_len);
    }

    if (errorText != "") {
        ESP_LOGE(TAG_SERVERGPIO, "%s", errorText.c_str());
    }

    return (errorText == "");
}


esp_err_t callHandleHttpRequest(httpd_req_t *req)
{
    ESP_LOGD(TAG_SERVERGPIO,"callHandleHttpRequest");

    GpioHandler *gpioHandler = (GpioHandler*)req->user_ctx;
    return gpioHandler->handleHttpRequest(req);
}

void taskGpioHandler(void *pvParameter)
{
    ESP_LOGD(TAG_SERVERGPIO,"taskGpioHandler");
    ((GpioHandler*)pvParameter)->init();
}

GpioHandler::GpioHandler(std::string configFile, httpd_handle_t httpServer) 
{
    ESP_LOGI(TAG_SERVERGPIO,"start GpioHandler");
    _configFile = configFile;
    _httpServer = httpServer;

    ESP_LOGI(TAG_SERVERGPIO, "register GPIO Uri");
    registerGpioUri();
}

GpioHandler::~GpioHandler()  {
    if (gpioMap != NULL) {
        clear();
        delete gpioMap;
    }
}

void GpioHandler::init()
{
    // TickType_t xDelay = 60000 / portTICK_PERIOD_MS;
    // printf("wait before start %ldms\r\n", (long) xDelay);
    // vTaskDelay( xDelay );

    if (gpioMap == NULL) {
        gpioMap = new std::map<gpio_num_t, GpioPin*>();
    } else {
        clear();
    }
    
    
    ESP_LOGI(TAG_SERVERGPIO, "read GPIO config and init GPIO");
    if (!readConfig()) {
        clear();
        delete gpioMap;
        gpioMap = NULL;
        ESP_LOGI(TAG_SERVERGPIO, "GPIO init comleted, handler is disabled");
        return;
    }


    for(std::map<gpio_num_t, GpioPin*>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it) {
        it->second->init();
    }

    std::function<void()> f = std::bind(&GpioHandler::handleMQTTconnect, this);
    MQTTregisterConnectFunction("gpio-handler", f);

    if (xHandleTaskGpio == NULL) {
        gpio_queue_handle = xQueueCreate(10,sizeof(GpioResult));
        BaseType_t  xReturned = xTaskCreate(&gpioHandlerTask, "gpio_int", configMINIMAL_STACK_SIZE * 8, (void *)this, tskIDLE_PRIORITY + 2, &xHandleTaskGpio);
        if(xReturned == pdPASS ) {
            ESP_LOGD(TAG_SERVERGPIO, "xHandletaskGpioHandler started");
        } else {
            ESP_LOGD(TAG_SERVERGPIO, "xHandletaskGpioHandler not started %d ", (int)xHandleTaskGpio);
        }
    }

    ESP_LOGI(TAG_SERVERGPIO, "GPIO init comleted, is enabled");
}

void GpioHandler::taskHandler() {
    if (gpioMap != NULL) {
        for(std::map<gpio_num_t, GpioPin*>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it) {
            if ((it->second->getInterruptType() == GPIO_INTR_DISABLE))
                it->second->publishState();
        }
    }
}


void GpioHandler::handleMQTTconnect()
{
    if (gpioMap != NULL) {
        for(std::map<gpio_num_t, GpioPin*>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it) {
            if ((it->second->getMode() == GPIO_PIN_MODE_INPUT) || (it->second->getMode() == GPIO_PIN_MODE_INPUT_PULLDOWN) || (it->second->getMode() == GPIO_PIN_MODE_INPUT_PULLUP))
                it->second->publishState();
        }
    }
}

void GpioHandler::deinit() {
    MQTTunregisterConnectFunction("gpio-handler");
    clear();
    if (xHandleTaskGpio != NULL) {
        vTaskDelete(xHandleTaskGpio);
        xHandleTaskGpio = NULL;
    }
}

void GpioHandler::gpioInterrupt(GpioResult* gpioResult) {
    if ((gpioMap != NULL) && (gpioMap->find(gpioResult->gpio) != gpioMap->end())) {
        (*gpioMap)[gpioResult->gpio]->gpioInterrupt(gpioResult->value);
    }
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
    
    _isEnabled = !disabledLine;

    if (!_isEnabled)
        return false;

//    std::string mainTopicMQTT = "";
    std::string mainTopicMQTT = GetMQTTMainTopic();
    if (mainTopicMQTT.length() > 0)
    {
        mainTopicMQTT = mainTopicMQTT + "/GPIO";
        ESP_LOGD(TAG_SERVERGPIO, "MAINTOPICMQTT found\r\n");
    }

    bool registerISR = false;
    while (configFile.getNextLine(&line, disabledLine, eof) && !configFile.isNewParagraph(line))
    {
        zerlegt = configFile.ZerlegeZeile(line);
        // const std::regex pieces_regex("IO([0-9]{1,2})");
        // std::smatch pieces_match;
        // if (std::regex_match(zerlegt[0], pieces_match, pieces_regex) && (pieces_match.size() == 2))
        // {
        //     std::string gpioStr = pieces_match[1];
        ESP_LOGD(TAG_SERVERGPIO, "conf param %s\r\n", toUpper(zerlegt[0]).c_str());
        if (toUpper(zerlegt[0]) == "MAINTOPICMQTT") {
//            ESP_LOGD(TAG_SERVERGPIO, "MAINTOPICMQTT found\r\n");
//            mainTopicMQTT = zerlegt[1];
        } else if ((zerlegt[0].rfind("IO", 0) == 0) && (zerlegt.size() >= 6))
        {
            ESP_LOGI(TAG_SERVERGPIO,"Enable GP%s in %s mode", zerlegt[0].c_str(), zerlegt[1].c_str());
            std::string gpioStr = zerlegt[0].substr(2, 2);
            gpio_num_t gpioNr = (gpio_num_t)atoi(gpioStr.c_str());
            gpio_pin_mode_t pinMode = resolvePinMode(toLower(zerlegt[1]));
            gpio_int_type_t intType = resolveIntType(toLower(zerlegt[2]));
            uint16_t dutyResolution = (uint8_t)atoi(zerlegt[3].c_str());
            bool mqttEnabled = toLower(zerlegt[4]) == "true";
            bool httpEnabled = toLower(zerlegt[5]) == "true";
            char gpioName[100];
            if (zerlegt.size() >= 7) {
                strcpy(gpioName, trim(zerlegt[6]).c_str());
            } else {
                sprintf(gpioName, "GPIO%d", gpioNr);
            }
            std::string mqttTopic = mqttEnabled ? (mainTopicMQTT + "/" + gpioName) : "";
            GpioPin* gpioPin = new GpioPin(gpioNr, gpioName, pinMode, intType,dutyResolution, mqttTopic, httpEnabled);
            (*gpioMap)[gpioNr] = gpioPin;

            if (intType != GPIO_INTR_DISABLE) {
                registerISR = true;
            }
        }
    }

    if (registerISR) {
        //install gpio isr service
        gpio_install_isr_service(ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM);
    }

    return true;
}

void GpioHandler::clear() 
{
    ESP_LOGD(TAG_SERVERGPIO, "GpioHandler::clear\r\n");

    if (gpioMap != NULL) {
        for(std::map<gpio_num_t, GpioPin*>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it) {
            delete it->second;
        }
        gpioMap->clear();
    }

    // gpio_uninstall_isr_service(); can't uninstall, isr service is used by camera
}
 
void GpioHandler::registerGpioUri() 
{
    ESP_LOGI(TAG_SERVERGPIO, "server_GPIO - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;
    camuri.uri       = "/GPIO";
    camuri.handler   = callHandleHttpRequest;
    camuri.user_ctx  = (void*)this;    
    httpd_register_uri_handler(_httpServer, &camuri);
}

esp_err_t GpioHandler::handleHttpRequest(httpd_req_t *req)
{
    ESP_LOGD(TAG_SERVERGPIO, "handleHttpRequest");

    if (gpioMap == NULL) {
        std::string resp_str = "GPIO handler not initialized";
        httpd_resp_send(req, resp_str.c_str(), resp_str.length());    
        return ESP_OK;
    }

#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_switch_GPIO - Start");    
#endif

    LogFile.WriteToFile("handler_switch_GPIO");    
    char _query[200];
    char _valueGPIO[30];    
    char _valueStatus[30];    
    std::string gpio, status;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK) {
        ESP_LOGD(TAG_SERVERGPIO, "Query: %s", _query);
        
        if (httpd_query_key_value(_query, "GPIO", _valueGPIO, 30) == ESP_OK)
        {
            ESP_LOGD(TAG_SERVERGPIO, "GPIO is found %s", _valueGPIO); 
            gpio = std::string(_valueGPIO);
        } else {
            std::string resp_str = "GPIO No is not defined";
            httpd_resp_send(req, resp_str.c_str(), resp_str.length());    
            return ESP_OK;
        }
        if (httpd_query_key_value(_query, "Status", _valueStatus, 30) == ESP_OK)
        {
            ESP_LOGD(TAG_SERVERGPIO, "Status is found %s", _valueStatus); 
            status = std::string(_valueStatus);
        }
    } else {
        const char* resp_str = "Error in call. Use /GPIO?GPIO=12&Status=high";
        httpd_resp_send(req, resp_str, strlen(resp_str));    
        return ESP_OK;
    }

    status = toUpper(status);
    if ((status != "HIGH") && (status != "LOW") && (status != "TRUE") && (status != "FALSE") && (status != "0") && (status != "1") && (status != ""))
    {
        std::string zw = "Status not valid: " + status;
        httpd_resp_sendstr_chunk(req, zw.c_str());
        httpd_resp_sendstr_chunk(req, NULL);          
        return ESP_OK;    
    }

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
        (*gpioMap)[gpio_num]->setValue((status == "HIGH") || (status == "TRUE") || (status == "1"), GPIO_SET_SOURCE_HTTP, &resp_str);
        if (resp_str == "") {
            resp_str = "GPIO" + std::to_string(gpionum) + " switched to " + status;
        }
        httpd_resp_sendstr_chunk(req, resp_str.c_str());
        httpd_resp_sendstr_chunk(req, NULL);
    }
          
    return ESP_OK;    
};

void GpioHandler::flashLightEnable(bool value) 
{
    ESP_LOGD(TAG_SERVERGPIO, "GpioHandler::flashLightEnable %s\r\n", value ? "true" : "false");

    if (gpioMap != NULL) {
        for(std::map<gpio_num_t, GpioPin*>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it) 
        {
            if (it->second->getMode() == GPIO_PIN_MODE_BUILT_IN_FLASH_LED) //|| (it->second->getMode() == GPIO_PIN_MODE_EXTERNAL_FLASH_PWM) || (it->second->getMode() == GPIO_PIN_MODE_EXTERNAL_FLASH_WS281X))
            {
                std::string resp_str = "";
                it->second->setValue(value, GPIO_SET_SOURCE_INTERNAL, &resp_str);

                if (resp_str == "") {
                    ESP_LOGD(TAG_SERVERGPIO, "Flash light pin GPIO %d switched to %s\r\n", (int)it->first, (value ? "on" : "off"));
                } else {
                    ESP_LOGE(TAG_SERVERGPIO, "Can't set flash light pin GPIO %d.  Error: %s\r\n", (int)it->first, resp_str.c_str());
                }
            }
        }
    }
}

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
    if( input == "built-in-led" ) return GPIO_PIN_MODE_BUILT_IN_FLASH_LED;
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

static GpioHandler *gpioHandler = NULL;

void gpio_handler_create(httpd_handle_t server) 
{
    if (gpioHandler == NULL)
        gpioHandler = new GpioHandler(CONFIG_FILE, server);
}

void gpio_handler_init() 
{
    if (gpioHandler != NULL) {
        gpioHandler->init();
    }
}

void gpio_handler_deinit() {
    if (gpioHandler != NULL) {
        gpioHandler->deinit();
   }
}

void gpio_handler_destroy()
{
    if (gpioHandler != NULL) {
        delete gpioHandler;
        gpioHandler = NULL;
    }
}

GpioHandler* gpio_handler_get()
{
    return gpioHandler;
}
