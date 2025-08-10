#include <string>
#include <string.h>
#include <vector>
#include <functional>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

#include <sys/stat.h>

#include "server_GpioPin.h"

#include "ClassLogFile.h"
#include "configFile.h"

#ifdef ENABLE_MQTT
#include "interface_mqtt.h"
#include "server_mqtt.h"
#endif // ENABLE_MQTT

#include "basic_auth.h"

static const char *TAG = "GPIO";

extern QueueHandle_t gpio_queue_handle;

GpioPin::GpioPin(gpio_num_t gpio, const char *gpioName, gpio_pin_mode_t gpioMode, gpio_int_type_t interruptType, int LedcFrequency, LedType SmartLedType, int SmartLedQuantity, Rgb SmartLedColor, std::string mqttTopic, bool httpEnable)
{
    _gpio = gpio;
    _gpioName = gpioName;
    _gpioMode = gpioMode;

    _interruptType = interruptType;
    _LedcFrequency = LedcFrequency;

    _mqttTopic = mqttTopic;
    _httpEnable = httpEnable;

    _SmartLedType = SmartLedType;
    _SmartLedQuantity = SmartLedQuantity;
    _SmartLedColor = SmartLedColor;
}

GpioPin::~GpioPin(void)
{
    ESP_LOGD(TAG, "reset GPIO pin %d", _gpio);
    if (_interruptType != GPIO_INTR_DISABLE)
    {
        // hook isr handler for specific gpio pin
        gpio_isr_handler_remove(_gpio);
    }

    gpio_reset_pin(_gpio);
}

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    GpioResult gpioResult;
    gpioResult.gpio = *(gpio_num_t *)arg;
    gpioResult.state = gpio_get_level(gpioResult.gpio);
    BaseType_t ContextSwitchRequest = pdFALSE;

    xQueueSendToBackFromISR(gpio_queue_handle, (void *)&gpioResult, &ContextSwitchRequest);

    if (ContextSwitchRequest)
    {
        taskYIELD();
    }
}

void GpioPin::setGpioState(int status)
{
#ifdef ENABLE_MQTT
    if (_mqttTopic.compare("") != 0)
    {
        ESP_LOGD(TAG, "setGpioState %s %d", _mqttTopic.c_str(), status);

        MQTTPublish(_mqttTopic, status ? "true" : "false", 1);
    }
#endif // ENABLE_MQTT
    currentGpioState = status;
}

void GpioPin::init(void)
{
    gpio_config_t io_conf;

    // set interrupt
    io_conf.intr_type = _interruptType;

    // set as output mode
    io_conf.mode = (_gpioMode == GPIO_PIN_MODE_OUTPUT) || (_gpioMode == GPIO_PIN_MODE_OUTPUT_PWM) ? gpio_mode_t::GPIO_MODE_OUTPUT : gpio_mode_t::GPIO_MODE_INPUT;

    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << _gpio);

    // set pull-down mode
    io_conf.pull_down_en = _gpioMode == GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pulldown_t::GPIO_PULLDOWN_ENABLE : gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;

    // set pull-up mode
    io_conf.pull_up_en = _gpioMode == GPIO_PIN_MODE_INPUT_PULLDOWN ? gpio_pullup_t::GPIO_PULLUP_ENABLE : gpio_pullup_t::GPIO_PULLUP_DISABLE;

    // configure GPIO with the given settings
    gpio_config(&io_conf);

    if (_interruptType != GPIO_INTR_DISABLE)
    {
        // hook isr handler for specific gpio pin
        ESP_LOGD(TAG, "GpioPin::init add isr handler for GPIO %d", _gpio);
        gpio_isr_handler_add(_gpio, gpio_isr_handler, (void *)&_gpio);
    }

#ifdef ENABLE_MQTT
    if ((_mqttTopic.compare("") != 0) && ((_gpioMode == GPIO_PIN_MODE_OUTPUT) || (_gpioMode == GPIO_PIN_MODE_OUTPUT_PWM)))
    {
        std::function<bool(std::string, char *, int)> f = std::bind(&GpioPin::handleMQTT, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        MQTTregisterSubscribeFunction(_mqttTopic, f);
    }
#endif // ENABLE_MQTT
}

bool GpioPin::getGpioState(std::string *errorText)
{
    if ((_gpioMode != GPIO_PIN_MODE_INPUT) && (_gpioMode != GPIO_PIN_MODE_INPUT_PULLUP) && (_gpioMode != GPIO_PIN_MODE_INPUT_PULLDOWN))
    {
        (*errorText) = "GPIO is not in input mode";
    }

    return gpio_get_level(_gpio) == 1;
}

void GpioPin::setGpioState(bool status, gpio_set_source setSource, std::string *errorText)
{
    ESP_LOGD(TAG, "GpioPin::setState %d", status);

    if ((_gpioMode != GPIO_PIN_MODE_OUTPUT) && (_gpioMode != GPIO_PIN_MODE_OUTPUT_PWM))
    {
        (*errorText) = "GPIO is not in output mode";
    }
    else
    {
        gpio_set_level(_gpio, status);

#ifdef ENABLE_MQTT
        if ((_mqttTopic.compare("") != 0) && (setSource != GPIO_SET_SOURCE_MQTT))
        {
            MQTTPublish(_mqttTopic, status ? "true" : "false", 1);
        }
#endif // ENABLE_MQTT
    }
}

void GpioPin::publishGpioState(void)
{
    int newState = gpio_get_level(_gpio);
    if (newState != currentGpioState)
    {
        ESP_LOGD(TAG, "publish state of GPIO %d new state %d", _gpio, newState);
#ifdef ENABLE_MQTT
        if (_mqttTopic.compare("") != 0)
        {
            MQTTPublish(_mqttTopic, newState ? "true" : "false", 1);
        }
#endif // ENABLE_MQTT
        currentGpioState = newState;
    }
}

#ifdef ENABLE_MQTT
bool GpioPin::handleMQTT(std::string, char *data, int data_len)
{
    ESP_LOGD(TAG, "GpioPin::handleMQTT data %.*s", data_len, data);

    std::string dataStr(data, data_len);
    dataStr = toLower(dataStr);
    std::string errorText = "";

    if ((dataStr == "true") || (dataStr == "1"))
    {
        setGpioState(true, GPIO_SET_SOURCE_MQTT, &errorText);
    }
    else if ((dataStr == "false") || (dataStr == "0"))
    {
        setGpioState(false, GPIO_SET_SOURCE_MQTT, &errorText);
    }
    else
    {
        errorText = "wrong value ";
        errorText.append(data, data_len);
    }

    if (errorText != "")
    {
        ESP_LOGE(TAG, "%s", errorText.c_str());
    }

    return (errorText == "");
}
#endif // ENABLE_MQTT
