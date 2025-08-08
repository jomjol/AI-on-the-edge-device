#include <string>
#include <string.h>
#include <vector>
#include <functional>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include <sys/stat.h>
#include "esp_check.h"

#include "onewire_bus.h"
#include "ds18b20.h"
#include "server_GpioHandler.h"

#include "ClassLogFile.h"
#include "configFile.h"

#ifdef ENABLE_MQTT
#include "interface_mqtt.h"
#include "server_mqtt.h"
#endif // ENABLE_MQTT

#include "basic_auth.h"

#include "Helper.h"
#include "../../include/defines.h"

static const char *TAG = "GPIO";

static GpioHandler *gpioHandler = NULL;
QueueHandle_t gpio_queue_handle = NULL;

static void gpioHandlerTask(void *arg);

std::string default_mqttTopic = "";
bool default_httpEnable = false;

#if defined(FLASH_MODE)
gpio_pin_mode_t default_gpioMode = FLASH_MODE;
#else
gpio_pin_mode_t default_gpioMode = GPIO_PIN_MODE_OUTPUT_PWM;
#endif

#if defined(LEDC_FREQUENCY)
int default_LedcFrequency = LEDC_FREQUENCY;
#else
int default_LedcFrequency = 5000; // Frequency in Hertz. Set frequency at 5 kHz
#endif

#if defined(FLASH_SMARTLED_TYPE)
LedType default_SmartLedType = FLASH_SMARTLED_TYPE;
#else
LedType default_SmartLedType = LED_WS2812;
#endif

#if defined(FLASH_SMARTLED_COLOR)
Rgb default_SmartLedColor = FLASH_SMARTLED_COLOR;
#else
Rgb default_SmartLedColor = Rgb{0, 0, 0};
#endif

#if defined(FLASH_SMARTLED_QUANTITY)
int default_SmartLedQuantity = FLASH_SMARTLED_QUANTITY;
#else
int default_SmartLedQuantity = 1;
#endif

gpio_int_type_t default_interruptType = GPIO_INTR_DISABLE;

GpioHandler::GpioHandler(std::string configFile, httpd_handle_t httpServer)
{
    ESP_LOGI(TAG, "start GpioHandler");
    _configFile = configFile;
    _httpServer = httpServer;

    ESP_LOGI(TAG, "register GPIO Uri");
    registerGpioUri();
}

GpioHandler::~GpioHandler(void)
{
    if (gpioMap != NULL)
    {
        clear();
        delete gpioMap;
    }
}

// Warning:
// For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2, ESP32P4 targets,
// when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
// 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
void GpioHandler::ledc_init(int gpioNum, ledc_channel_t channel, int frequency)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.duty_resolution = LEDC_DUTY_RES;
    ledc_timer.timer_num = LEDC_TIMER;
    ledc_timer.freq_hz = (uint32_t)clipInt(frequency, 8000, 1); // LEDC_FREQUENCY

    ledc_timer.clk_cfg = LEDC_AUTO_CLK;
    // ledc_timer.clk_cfg = LEDC_USE_APB_CLK;
    // ledc_timer.clk_cfg = LEDC_USE_REF_TICK; // 1 MHz Dynamic Frequency Scaling compatible
    // ledc_timer.clk_cfg = LEDC_USE_RC_FAST_CLK; // ~ 8 MHz Dynamic Frequency Scaling compatible, Light sleep compatible

    // ledc_timer.deconfigure = true; // Set this field to de-configure a LEDC timer which has been configured before

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {};
    ledc_channel.gpio_num = gpioNum;
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel = channel;
    ledc_channel.intr_type = LEDC_INTR_DISABLE;
    ledc_channel.timer_sel = LEDC_TIMER;
    ledc_channel.duty = 0; // Set duty to 0%
    ledc_channel.hpoint = 0;
    ledc_channel.flags.output_invert = LEDC_OUTPUT_INVERT; // Enable (1) or disable (0) gpio output invert

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void GpioHandler::init(void)
{
    ESP_LOGD(TAG, "*************** Start GPIOHandler_Init *****************");

    if (gpioMap == NULL)
    {
        gpioMap = new std::map<gpio_num_t, GpioPin *>();
    }
    else
    {
        clear();
    }

    ESP_LOGI(TAG, "read GPIO config and init GPIO");
    if (!ReadParameter())
    {
        ESP_LOGI(TAG, "GPIO in config.ini is disabled, the onboard led is used");
    }

    for (std::map<gpio_num_t, GpioPin *>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it)
    {
        it->second->init();
    }

    int smartLedChannel = 0; // max. 8 channels
    int ledcChannel = 1;     // max 8 channels (CH0: camera, CH1 - CH7: spare)

    for (std::map<gpio_num_t, GpioPin *>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it)
    {
        it->second->init();

        if (it->second->getGpioMode() == GPIO_PIN_MODE_WS281X)
        {
            it->second->setSmartLed(new SmartLed(it->second->getSmartLedType(), it->second->getSmartLedQuantity(), it->second->getGPIO(), smartLedChannel, DoubleBuffer));

            smartLedChannel++;

            if (smartLedChannel == detail::CHANNEL_COUNT)
            {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Insufficient SmartLED channels");
            }
        }
        else if (it->second->getGpioMode() == GPIO_PIN_MODE_OUTPUT_PWM)
        {
            ledc_init(it->second->getGPIO(), (ledc_channel_t)ledcChannel, it->second->getLedcFrequency());
            it->second->setLedcChannel(static_cast<ledc_channel_t>(ledcChannel));
            ledcChannel++;

            if (ledcChannel == LEDC_CHANNEL_MAX)
            {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Insufficient LEDC channels");
            }
        }
        // Handler task is only needed to maintain input pin state (interrupt or polling)
        else if (it->second->getGpioMode() == GPIO_PIN_MODE_INPUT || it->second->getGpioMode() == GPIO_PIN_MODE_INPUT_PULLUP || it->second->getGpioMode() == GPIO_PIN_MODE_INPUT_PULLDOWN || it->second->getGpioMode() == GPIO_PIN_MODE_DS18B20)
        {
            ESP_LOGD(TAG, "input pin");

            if (it->second->getGpioMode() == GPIO_PIN_MODE_DS18B20)
            {
            }
        }
    }

#ifdef ENABLE_MQTT
    std::function<void()> f = std::bind(&GpioHandler::handleMQTTconnect, this);
    MQTTregisterConnectFunction("gpio-handler", f);
#endif // ENABLE_MQTT

    if (xHandleTaskGpio == NULL)
    {
        gpio_queue_handle = xQueueCreate(10, sizeof(GpioResult));
        BaseType_t xReturned = xTaskCreate(&gpioHandlerTask, "gpio_int", 3 * 1024, (void *)this, tskIDLE_PRIORITY + 4, &xHandleTaskGpio);
        if (xReturned == pdPASS)
        {
            ESP_LOGD(TAG, "xHandletaskGpioHandler started");
        }
        else
        {
            ESP_LOGD(TAG, "xHandletaskGpioHandler not started %d ", (int)xHandleTaskGpio);
        }
    }

    ESP_LOGI(TAG, "GPIO init completed, is enabled");
}

void GpioHandler::deinit(void)
{
#ifdef ENABLE_MQTT
    MQTTunregisterConnectFunction("gpio-handler");
#endif // ENABLE_MQTT
    clear();
    if (xHandleTaskGpio != NULL)
    {
        vTaskDelete(xHandleTaskGpio);
        xHandleTaskGpio = NULL;
    }
}

static void gpioHandlerTask(void *arg)
{
    ESP_LOGD(TAG, "start interrupt task");
    while (1)
    {
        if (uxQueueMessagesWaiting(gpio_queue_handle))
        {
            while (uxQueueMessagesWaiting(gpio_queue_handle))
            {
                GpioResult gpioResult;
                xQueueReceive(gpio_queue_handle, (void *)&gpioResult, 10);
                ESP_LOGD(TAG, "gpio: %d state: %d", gpioResult.gpio, gpioResult.state);
                ((GpioHandler *)arg)->setGpioState(&gpioResult);
            }
        }

        ((GpioHandler *)arg)->taskHandler();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void GpioHandler::taskHandler(void)
{
    if (gpioMap != NULL)
    {
        for (std::map<gpio_num_t, GpioPin *>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it)
        {
            if ((it->second->getGpioInterruptType() == GPIO_INTR_DISABLE))
            {
                it->second->publishGpioState();
            }
        }
    }
}

#ifdef ENABLE_MQTT
void GpioHandler::handleMQTTconnect(void)
{
    if (gpioMap != NULL)
    {
        for (std::map<gpio_num_t, GpioPin *>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it)
        {
            if ((it->second->getGpioMode() == GPIO_PIN_MODE_INPUT) || (it->second->getGpioMode() == GPIO_PIN_MODE_INPUT_PULLDOWN) || (it->second->getGpioMode() == GPIO_PIN_MODE_INPUT_PULLUP) || (it->second->getGpioMode() == GPIO_PIN_MODE_DS18B20))
            {
                if (it->second->getGpioMode() == GPIO_PIN_MODE_DS18B20)
                {
                }
                else
                {
                    it->second->publishGpioState();
                }
            }
        }
    }
}
#endif // ENABLE_MQTT

void GpioHandler::setGpioState(GpioResult *gpioResult)
{
    if ((gpioMap != NULL) && (gpioMap->find(gpioResult->gpio) != gpioMap->end()))
    {
        (*gpioMap)[gpioResult->gpio]->setGpioState(gpioResult->state);
    }
}

bool GpioHandler::ReadParameter(void)
{
    if (!gpioMap->empty())
    {
        clear();
    }

    ConfigFile configFile = ConfigFile(_configFile);
    std::string line = "";
    bool disabledLine = false;
    bool eof = false;

    while ((!configFile.GetNextParagraph(line, disabledLine, eof) || (toUpper(line).compare("[GPIO]") != 0)) && !eof)
    {
    }

    _isEnabled = !disabledLine;

    if (eof || disabledLine)
    {
        GpioPin *gpioPin = new GpioPin(FLASH_GPIO, "On_Board_LED", FLASH_MODE, default_interruptType, default_LedcFrequency, default_SmartLedType, default_SmartLedQuantity, default_SmartLedColor, default_mqttTopic, default_httpEnable);
        (*gpioMap)[FLASH_GPIO] = gpioPin;
        return false;
    }

#ifdef ENABLE_MQTT
    std::string mainTopicMQTT = mqttServer_getMainTopic();
    if (mainTopicMQTT.length() > 0)
    {
        mainTopicMQTT = mainTopicMQTT + "/GPIO";
        ESP_LOGD(TAG, "MAINTOPICMQTT found");
    }
#endif // ENABLE_MQTT

    std::vector<std::string> splitted;
    bool registerISR = false;

    while (configFile.getNextLine(&line, disabledLine, eof) && !configFile.isNewParagraph(line))
    {
        splitted = splitLine(line);
        // IOx = [pinMode, intType, pwmFrequency, ledType, ledQuantity, led_R,  led_G,  led_B, mqttEnabled, httpEnabled, gpioName]
        ESP_LOGD(TAG, "conf param %s", toUpper(splitted[0]).c_str());

        char gpioName[100];
        gpio_num_t gpioNum = FLASH_GPIO;
        gpio_pin_mode_t pinMode = GPIO_PIN_MODE_OUTPUT_PWM;
        gpio_int_type_t intType = GPIO_INTR_DISABLE;
        int LedcFrequency = 5000;
        bool httpEnabled = false;
        bool mqttEnabled = false;
        std::string mqttTopic = "";

        int LEDQuantity = 0;
        Rgb LEDColor = Rgb{0, 0, 0};
        LedType LEDType = LED_WS2812;

        if ((splitted[0].find_first_of("IO") > 0) || (splitted[0].find_first_of("IO") == std::string::npos))
        {
            // Parameter disabled
            continue;
        }
        else if ((splitted[0].rfind("IO", 0) == 0) && (splitted.size() >= 10))
        {
            ESP_LOGI(TAG, "Enable GP%s in %s mode", splitted[0].c_str(), splitted[1].c_str());
            std::string gpioStr = splitted[0].substr(2, 2);
            gpio_num_t gpioNum = resolvePinNum(std::atoi(gpioStr.c_str()));

            pinMode = resolvePinMode(toLower(splitted[1]));
            intType = resolveIntType(toLower(splitted[2]));

            if (isStringNumeric(splitted[3]))
            {
                LedcFrequency = std::stoi(splitted[3]);
            }

            LEDType = resolveLedType(splitted[4]);
            LEDQuantity = stoi(splitted[5]);

            uint8_t _red = 127, _green = 127, _blue = 127;
            if (isStringNumeric(splitted[6]))
            {
                _red = std::stoi(splitted[6]);
            }
            if (isStringNumeric(splitted[7]))
            {
                _green = std::stoi(splitted[7]);
            }
            if (isStringNumeric(splitted[8]))
            {
                _blue = std::stoi(splitted[8]);
            }
            LEDColor = Rgb{_red, _green, _blue};

#ifdef ENABLE_MQTT
            mqttEnabled = (toLower(splitted[9]) == "true");
#endif // ENABLE_MQTT

            httpEnabled = (toLower(splitted[10]) == "true");

            if (splitted.size() >= 11)
            {
                strcpy(gpioName, trim_string_left_right(splitted[11]).c_str());
            }
            else
            {
                sprintf(gpioName, "GPIO%d", gpioNum);
            }

#ifdef ENABLE_MQTT
            mqttTopic = mqttEnabled ? (mainTopicMQTT + "/" + gpioName) : "";
#else  // ENABLE_MQTT
            mqttTopic = "";
#endif // ENABLE_MQTT

            if (intType != GPIO_INTR_DISABLE)
            {
                registerISR |= true;
            }
        }

        GpioPin *gpioPin = new GpioPin(gpioNum, gpioName, pinMode, intType, LedcFrequency, LEDType, LEDQuantity, LEDColor, mqttTopic, httpEnabled);
        (*gpioMap)[gpioNum] = gpioPin;
    }

    if (registerISR)
    {
        // install gpio isr service
        gpio_install_isr_service(ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM);
    }

    return true;
}

void GpioHandler::clear(void)
{
    ESP_LOGD(TAG, "GpioHandler::clear");

    if (gpioMap != NULL)
    {
        for (std::map<gpio_num_t, GpioPin *>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it)
        {
            delete it->second;
        }
        gpioMap->clear();
    }
}

esp_err_t callHandleHttpRequest(httpd_req_t *req)
{
    ESP_LOGD(TAG, "callHandleHttpRequest");

    GpioHandler *gpioHandler = (GpioHandler *)req->user_ctx;
    return gpioHandler->handleHttpRequest(req);
}

esp_err_t GpioHandler::handleHttpRequest(httpd_req_t *req)
{
    ESP_LOGD(TAG, "handleHttpRequest");

    if (gpioMap == NULL)
    {
        std::string resp_str = "GPIO handler not initialized";
        httpd_resp_send(req, resp_str.c_str(), resp_str.length());
        return ESP_OK;
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_switch_GPIO - Start");
#endif

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "handler_switch_GPIO");
    char _query[200];
    char _valueGPIO[30];
    char _valueStatus[30];
    std::string gpio, status;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        ESP_LOGD(TAG, "Query: %s", _query);

        if (httpd_query_key_value(_query, "GPIO", _valueGPIO, 30) == ESP_OK)
        {
            ESP_LOGD(TAG, "GPIO is found %s", _valueGPIO);
            gpio = std::string(_valueGPIO);
        }
        else
        {
            std::string resp_str = "GPIO No is not defined";
            httpd_resp_send(req, resp_str.c_str(), resp_str.length());
            return ESP_OK;
        }

        if (httpd_query_key_value(_query, "Status", _valueStatus, 30) == ESP_OK)
        {
            ESP_LOGD(TAG, "Status is found %s", _valueStatus);
            status = std::string(_valueStatus);
        }
    }
    else
    {
        const char *resp_str = "Error in call. Use /GPIO?GPIO=12&Status=high";
        httpd_resp_send(req, resp_str, strlen(resp_str));
        return ESP_OK;
    }

    status = toUpper(status);
    if ((status != "HIGH") && (status != "LOW") && (status != "TRUE") && (status != "FALSE") && (status != "0") && (status != "1") && (status != ""))
    {
        std::string temp_bufer = "Status not valid: " + status;
        httpd_resp_sendstr_chunk(req, temp_bufer.c_str());
        httpd_resp_sendstr_chunk(req, NULL);
        return ESP_OK;
    }

    int gpioNum = stoi(gpio);

    // frei: 16; 12-15; 2; 4  // nur 12 und 13 funktionieren 2: reboot, 4: BlitzLED, 15: PSRAM, 14/15: DMA für SDKarte ???
    gpio_num_t gpio_num = resolvePinNum(gpioNum);
    if (gpio_num == GPIO_NUM_NC)
    {
        std::string temp_bufer = "GPIO" + std::to_string(gpioNum) + " unsupported - only 12 & 13 free";
        httpd_resp_sendstr_chunk(req, temp_bufer.c_str());
        httpd_resp_sendstr_chunk(req, NULL);
        return ESP_OK;
    }

    if (gpioMap->count(gpio_num) == 0)
    {
        char resp_str[30];
        sprintf(resp_str, "GPIO%d is not registred", gpio_num);
        httpd_resp_send(req, resp_str, strlen(resp_str));
        return ESP_OK;
    }

    if (status == "")
    {
        std::string resp_str = "";
        status = (*gpioMap)[gpio_num]->getGpioState(&resp_str) ? "HIGH" : "LOW";
        if (resp_str == "")
        {
            resp_str = status;
        }
        httpd_resp_sendstr_chunk(req, resp_str.c_str());
        httpd_resp_sendstr_chunk(req, NULL);
    }
    else
    {
        std::string resp_str = "";

        (*gpioMap)[gpio_num]->setGpioState((status == "HIGH") || (status == "TRUE") || (status == "1"), GPIO_SET_SOURCE_HTTP, &resp_str);

        if (resp_str == "")
        {
            resp_str = "GPIO" + std::to_string(gpioNum) + " switched to " + status;
        }

        httpd_resp_sendstr_chunk(req, resp_str.c_str());
        httpd_resp_sendstr_chunk(req, NULL);
    }

    return ESP_OK;
};

void GpioHandler::flashLightControl(bool status, int intensity)
{
    ESP_LOGD(TAG, "GpioHandler::flashLightEnable %s", status ? "true" : "false");

    if (gpioMap != NULL)
    {
        for (std::map<gpio_num_t, GpioPin *>::iterator it = gpioMap->begin(); it != gpioMap->end(); ++it)
        {
            if (it->second->getGpioMode() == GPIO_PIN_MODE_OUTPUT_PWM)
            {
                if (status)
                {
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, intensity);
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL);
                }
                else
                {
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, 0);
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL);
                }

                ESP_LOGD(TAG, "Flash light pin GPIO %d switched to %s", (int)it->first, (status ? "on" : "off"));
            }
            else if (it->second->getGpioMode() == GPIO_PIN_MODE_WS281X)
            {
                Rgb SmartLedColorTemp = it->second->getSmartLedColor();
                float _intensity = (float)intensity / 81.91; // intensity / 8191 * 100
                float SmartLedColorTempAverage = ((float)((SmartLedColorTemp.r + SmartLedColorTemp.g + SmartLedColorTemp.b) / 3) / 2.55);
                int SmartLedColorTempCorrected = ((int)(round((_intensity - SmartLedColorTempAverage) * 2.55)));
                SmartLedColorTemp.r = std::min(std::max(0, (SmartLedColorTemp.r + SmartLedColorTempCorrected)), 255);
                SmartLedColorTemp.g = std::min(std::max(0, (SmartLedColorTemp.g + SmartLedColorTempCorrected)), 255);
                SmartLedColorTemp.b = std::min(std::max(0, (SmartLedColorTemp.b + SmartLedColorTempCorrected)), 255);

                if (status)
                {
                    for (int i = 0; i < it->second->getSmartLedQuantity(); ++i)
                    {
                        (*it->second->getSmartLed())[i] = SmartLedColorTemp;
                    }
                }
                else
                {
                    for (int i = 0; i < it->second->getSmartLedQuantity(); ++i)
                    {
                        (*it->second->getSmartLed())[i] = Rgb{0, 0, 0};
                    }
                }
                it->second->getSmartLed()->show();
                it->second->setGpioState(status ? true : false);
            }
            else if (it->second->getGpioMode() == GPIO_PIN_MODE_OUTPUT)
            {
                std::string resp_str = "";
                it->second->setGpioState(status, GPIO_SET_SOURCE_INTERNAL, &resp_str);

                if (resp_str == "")
                {
                    ESP_LOGD(TAG, "Flash light pin GPIO %d switched to %s", (int)it->first, (status ? "on" : "off"));
                }
                else
                {
                    ESP_LOGE(TAG, "Can't set flash light pin GPIO %d.  Error: %s", (int)it->first, resp_str.c_str());
                }
            }
            else
            {
                ESP_LOGE(TAG, "GpioHandler::flashLightControl : incorrect Gpio Mode");
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "GpioHandler::flashLightControl : gpioMap empty");
    }
}

gpio_num_t GpioHandler::resolvePinNum(uint8_t gpioNum)
{
    switch (gpioNum)
    {
    case 1:
        return GPIO_IO1;
    case 2:
        return GPIO_IO2;
    case 3:
        return GPIO_IO3;
    case 4:
        return GPIO_IO4;
    }

    return GPIO_NUM_NC;
}

gpio_pin_mode_t GpioHandler::resolvePinMode(std::string input)
{
    if (input == "disabled")
    {
        return GPIO_PIN_MODE_DISABLED;
    }
    else if (input == "input")
    {
        return GPIO_PIN_MODE_INPUT;
    }
    else if (input == "input-pullup")
    {
        return GPIO_PIN_MODE_INPUT_PULLUP;
    }
    else if (input == "input-pulldown")
    {
        return GPIO_PIN_MODE_INPUT_PULLDOWN;
    }
    else if (input == "input-ds18b20")
    {
        return GPIO_PIN_MODE_DS18B20;
    }
    else if (input == "output")
    {
        return GPIO_PIN_MODE_OUTPUT;
    }
    else if (input == "output-pwm")
    {
        return GPIO_PIN_MODE_OUTPUT_PWM;
    }
    else if (input == "output-ws281x")
    {
        return GPIO_PIN_MODE_WS281X;
    }

    return GPIO_PIN_MODE_DISABLED;
}

gpio_int_type_t GpioHandler::resolveIntType(std::string input)
{
    if (input == "disabled")
    {
        return GPIO_INTR_DISABLE;
    }
    else if (input == "rising-edge")
    {
        return GPIO_INTR_POSEDGE;
    }
    else if (input == "falling-edge")
    {
        return GPIO_INTR_NEGEDGE;
    }
    else if (input == "rising-and-falling")
    {
        return GPIO_INTR_ANYEDGE;
    }
    else if (input == "low-level-trigger")
    {
        return GPIO_INTR_LOW_LEVEL;
    }
    else if (input == "high-level-trigger")
    {
        return GPIO_INTR_HIGH_LEVEL;
    }

    return GPIO_INTR_DISABLE;
}

LedType GpioHandler::resolveLedType(std::string input)
{
    if (input == "WS2812")
    {
        return LED_WS2812;
    }
    else if (input == "WS2812B")
    {
        return LED_WS2812B;
    }
    else if (input == "SK6812")
    {
        return LED_SK6812;
    }
    else if (input == "WS2813")
    {
        return LED_WS2813;
    }

    return LED_WS2812;
}

void GpioHandler::registerGpioUri(void)
{
    ESP_LOGI(TAG, "server_GPIO - Registering URI handlers");

    httpd_uri_t camuri = {};
    camuri.method = HTTP_GET;
    camuri.uri = "/GPIO";
    camuri.handler = APPLY_BASIC_AUTH_FILTER(callHandleHttpRequest);
    camuri.user_ctx = (void *)this;
    httpd_register_uri_handler(_httpServer, &camuri);
}

void taskGpioHandler(void *pvParameter)
{
    ESP_LOGD(TAG, "taskGpioHandler");
    ((GpioHandler *)pvParameter)->init();
}

void gpio_handler_create(httpd_handle_t server)
{
    if (gpioHandler == NULL)
    {
        gpioHandler = new GpioHandler(CONFIG_FILE, server);
    }
}

void gpio_handler_init(void)
{
    if (gpioHandler != NULL)
    {
        gpioHandler->init();
    }
}

void gpio_handler_deinit(void)
{
    if (gpioHandler != NULL)
    {
        gpioHandler->deinit();
    }
}

void gpio_handler_destroy(void)
{
    if (gpioHandler != NULL)
    {
        gpio_handler_deinit();
        delete gpioHandler;
        gpioHandler = NULL;
    }
}

GpioHandler *gpio_handler_get(void)
{
    return gpioHandler;
}
