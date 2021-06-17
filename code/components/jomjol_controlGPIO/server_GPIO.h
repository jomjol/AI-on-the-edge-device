#include <esp_log.h>

#include <esp_http_server.h>
#include <map>

//#include "ClassControllCamera.h"

static const char *TAGPARTGPIO = "server_GPIO";

typedef enum {
    GPIO_PIN_MODE_DISABLED              = 0x0,
    GPIO_PIN_MODE_INPUT                 = 0x1,
    GPIO_PIN_MODE_INPUT_PULLUP          = 0x2,
    GPIO_PIN_MODE_INPUT_PULLDOWN        = 0x3,
    GPIO_PIN_MODE_OUTPUT                = 0x4,
    GPIO_PIN_MODE_OUTPUT_PWM            = 0x5,
    GPIO_PIN_MODE_EXTERNAL_FLASH_PWM    = 0x5,
    GPIO_PIN_MODE_EXTERNAL_FLASH_WS281X = 0x5,
} gpio_pin_mode_t;

class GpioPin {
public:
    GpioPin(gpio_num_t gpio, const char* name, gpio_pin_mode_t mode, gpio_int_type_t interruptType, uint8_t dutyResolution, bool mqttEnable, bool httpEnable);
    ~GpioPin();

    bool getValue(std::string* errorText);
    void setValue(bool value, std::string* errorText);
    void init();

private:
    gpio_num_t _gpio;
    const char* _name;
    gpio_pin_mode_t _mode;
    gpio_int_type_t _interruptType;
};

esp_err_t callHandleHttpRequest(httpd_req_t *req);
void taskGpioHandler(void *pvParameter);

class GpioHandler {
public:
    GpioHandler(std::string configFile, httpd_handle_t httpServer);
    ~GpioHandler();
    
    void init();
    void clear();
    void registerGpioUri();
    esp_err_t handleHttpRequest(httpd_req_t *req);  

private:
    std::string _configFile;
    httpd_handle_t _httpServer;
    std::map<gpio_num_t, GpioPin*> *gpioMap = NULL;
    TaskHandle_t xHandletaskGpioHandler = NULL;

    bool readConfig();
    gpio_num_t resolvePinNr(uint8_t pinNr);
    gpio_pin_mode_t resolvePinMode(std::string input);
    gpio_int_type_t resolveIntType(std::string input);
};

void GpioHandlerStart();