#pragma once

#ifndef SERVER_GPIOPIN_H
#define SERVER_GPIOPIN_H

#include <map>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include "SmartLeds.h"

#include "ClassControllCamera.h"

#include "Helper.h"
#include "../../include/defines.h"

typedef enum {
    GPIO_PIN_MODE_DISABLED = 0x0,
    GPIO_PIN_MODE_INPUT = 0x1,
    GPIO_PIN_MODE_INPUT_PULLUP = 0x2,
    GPIO_PIN_MODE_INPUT_PULLDOWN = 0x3,
    GPIO_PIN_MODE_OUTPUT = 0x4,
    GPIO_PIN_MODE_OUTPUT_PWM = 0x5,
    GPIO_PIN_MODE_WS281X = 0x6,
    GPIO_PIN_MODE_DS18B20 = 0x7,
} gpio_pin_mode_t;

struct GpioResult {
    gpio_num_t gpio;
    int state;
};

typedef enum {
    GPIO_SET_SOURCE_INTERNAL = 0,
    GPIO_SET_SOURCE_MQTT = 1,
    GPIO_SET_SOURCE_HTTP = 2,
} gpio_set_source;

class GpioPin
{
  private:
    gpio_num_t _gpio;
    const char *_gpioName;
    gpio_pin_mode_t _gpioMode;
    gpio_int_type_t _interruptType;

    std::string _mqttTopic;
    bool _httpEnable;

    SmartLed *_SmartLed = NULL;
    ledc_channel_t _LedcChannel = LEDC_CHANNEL_MAX;
    int _LedcFrequency;

    int _SmartLedQuantity = 2;
    Rgb _SmartLedColor = Rgb{255, 255, 255};
    LedType _SmartLedType = LED_WS2812;

    int currentGpioState = -1;

  public:
    GpioPin(gpio_num_t gpio, const char *gpioName, gpio_pin_mode_t gpioMode, gpio_int_type_t interruptType, int LedcFrequency, LedType SmartLedType, int SmartLedQuantity, Rgb SmartLedColor, std::string mqttTopic, bool httpEnable);
    ~GpioPin(void);

    void init(void);
    bool getGpioState(std::string *errorText);
    void setGpioState(bool status, gpio_set_source setSource, std::string *errorText);

#ifdef ENABLE_MQTT
    bool handleMQTT(std::string, char *data, int data_len);
#endif // ENABLE_MQTT

    void publishGpioState(void);
    void setGpioState(int status);

    gpio_num_t getGPIO(void) { return _gpio; };
    gpio_pin_mode_t getGpioMode(void) { return _gpioMode; }
    gpio_int_type_t getGpioInterruptType(void) { return _interruptType; }

    void setLedcChannel(ledc_channel_t _ledcChannel_) { _LedcChannel = _ledcChannel_; };
    int getLedcFrequency(void) { return _LedcFrequency; };
    ledc_channel_t getLedcChannel(void) { return _LedcChannel; };

    SmartLed *getSmartLed(void) { return _SmartLed; };
    void setSmartLed(SmartLed *_smartLed) { _SmartLed = _smartLed; };

    LedType getSmartLedType(void) { return _SmartLedType; };
    int getSmartLedQuantity(void) { return _SmartLedQuantity; };
    Rgb getSmartLedColor(void) { return _SmartLedColor; };
};

#endif // SERVER_GPIOPIN_H
