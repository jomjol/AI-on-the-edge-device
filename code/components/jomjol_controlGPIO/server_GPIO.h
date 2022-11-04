#ifndef SERVER_GPIO_H
#define SERVER_GPIO_H

#include <esp_log.h>

#include <esp_http_server.h>
#include <map>
#include "driver/gpio.h"

#include "SmartLeds.h"

//#include "ClassControllCamera.h"

// wenn __LEDGLOBAL definiert ist, wird eine globale Variable für die LED-Ansteuerung verwendet, ansonsten lokal und jedesmal neu
#define __LEDGLOBAL

typedef enum {
    GPIO_PIN_MODE_DISABLED              = 0x0,
    GPIO_PIN_MODE_INPUT                 = 0x1,
    GPIO_PIN_MODE_INPUT_PULLUP          = 0x2,
    GPIO_PIN_MODE_INPUT_PULLDOWN        = 0x3,
    GPIO_PIN_MODE_OUTPUT                = 0x4,
    GPIO_PIN_MODE_BUILT_IN_FLASH_LED    = 0x5,
    GPIO_PIN_MODE_OUTPUT_PWM            = 0x6,
    GPIO_PIN_MODE_EXTERNAL_FLASH_PWM    = 0x7,
    GPIO_PIN_MODE_EXTERNAL_FLASH_WS281X = 0x8,
} gpio_pin_mode_t;

struct GpioResult {
    gpio_num_t gpio;
    int value;
};

typedef enum {
    GPIO_SET_SOURCE_INTERNAL  = 0,
    GPIO_SET_SOURCE_MQTT  = 1,
    GPIO_SET_SOURCE_HTTP  = 2,
} gpio_set_source;

class GpioPin {
public:
    GpioPin(gpio_num_t gpio, const char* name, gpio_pin_mode_t mode, gpio_int_type_t interruptType, uint8_t dutyResolution, std::string mqttTopic, bool httpEnable);
    ~GpioPin();

    void init();
    bool getValue(std::string* errorText);
    void setValue(bool value, gpio_set_source setSource, std::string* errorText);
    bool handleMQTT(std::string, char* data, int data_len);
    void publishState();
    void gpioInterrupt(int value);
    gpio_int_type_t getInterruptType() { return _interruptType; }
    gpio_pin_mode_t getMode() { return _mode; }
    gpio_num_t getGPIO(){return _gpio;};

private:
    gpio_num_t _gpio;
    const char* _name;
    gpio_pin_mode_t _mode;
    gpio_int_type_t _interruptType;
    std::string _mqttTopic;
    int currentState = -1;
};

esp_err_t callHandleHttpRequest(httpd_req_t *req);
void taskGpioHandler(void *pvParameter);

class GpioHandler {
public:
    GpioHandler(std::string configFile, httpd_handle_t httpServer);
    ~GpioHandler();
    
    void init();
    void deinit();
    void registerGpioUri();
    esp_err_t handleHttpRequest(httpd_req_t *req);
    void taskHandler();
    void gpioInterrupt(GpioResult* gpioResult);  
    void flashLightEnable(bool value);
    bool isEnabled() { return _isEnabled; }
    void handleMQTTconnect();

private:
    std::string _configFile;
    httpd_handle_t _httpServer;
    std::map<gpio_num_t, GpioPin*> *gpioMap = NULL;
    TaskHandle_t xHandleTaskGpio = NULL;
    bool _isEnabled = false;

    int LEDNumbers = 2;
    Rgb LEDColor = Rgb{ 255, 255, 255 };
    LedType LEDType = LED_WS2812;
#ifdef __LEDGLOBAL
    SmartLed *leds_global = NULL;
#endif

    bool readConfig();
    void clear();
    
    gpio_num_t resolvePinNr(uint8_t pinNr);
    gpio_pin_mode_t resolvePinMode(std::string input);
    gpio_int_type_t resolveIntType(std::string input);
};

void gpio_handler_create(httpd_handle_t server);
void gpio_handler_init();
void gpio_handler_deinit();
void gpio_handler_destroy();
GpioHandler* gpio_handler_get();



#endif //SERVER_GPIO_H