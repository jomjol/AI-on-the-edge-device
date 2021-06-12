#include <esp_log.h>

#include <esp_http_server.h>
#include <map>

//#include "ClassControllCamera.h"

static const char *TAGPARTGPIO = "server_GPIO";

typedef enum {
    GPIO_PIN_MODE_DISABLED       = 0x0,
    GPIO_PIN_MODE_INPUT          = 0x1,
    GPIO_PIN_MODE_INPUT_PULLUP   = 0x2,
    GPIO_PIN_MODE_INPUT_PULLDOWN = 0x3,
    GPIO_PIN_MODE_OUTPUT         = 0x4,
    GPIO_PIN_MODE_OUTPUT_PWM     = 0x5,
} gpio_pin_mode_t;

class GpioPin {
public:
    GpioPin(gpio_num_t gpio, const char* name, gpio_pin_mode_t mode, gpio_int_type_t interruptType);
    void setValue(bool value);

private:
    gpio_num_t _gpio;
    const char* _name;
    gpio_pin_mode_t _mode;
    gpio_int_type_t _interruptType;

    void initGPIO();
};

esp_err_t callHandleHttpRequest(httpd_req_t *req);

class GpioHandler {
public:
    GpioHandler(std::string configFile, httpd_handle_t server);

    void registerGpioUri(httpd_handle_t server);
    esp_err_t handleHttpRequest(httpd_req_t *req);  

private:
    std::string _configFile;
    std::map<gpio_num_t, GpioPin*> *gpioMap = NULL;

    bool readConfig();
    gpio_num_t resolvePinNr(uint8_t pinNr);
    gpio_pin_mode_t resolvePinMode(std::string input);
    gpio_int_type_t resolveIntType(std::string input);
};