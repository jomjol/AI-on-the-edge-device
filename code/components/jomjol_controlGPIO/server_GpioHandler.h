#pragma once

#ifndef SERVER_GPIOHANDLER_H
#define SERVER_GPIOHANDLER_H

#include "server_GpioPin.h"

#include <map>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <esp_log.h>
#include <esp_http_server.h>

#include "SmartLeds.h"

#include "Helper.h"
#include "defines.h"

class GpioHandler
{
private:
  std::string _configFile;
  httpd_handle_t _httpServer;
  std::map<gpio_num_t, GpioPin *> *gpioMap = NULL;
  TaskHandle_t xHandleTaskGpio = NULL;
  bool _isEnabled = false;

  bool ReadParameter(void);
  void clear(void);

  gpio_num_t resolvePinNum(uint8_t gpioNum);
  gpio_pin_mode_t resolvePinMode(std::string input);
  gpio_int_type_t resolveIntType(std::string input);
  LedType resolveLedType(std::string input);

public:
  GpioHandler(std::string configFile, httpd_handle_t httpServer);
  ~GpioHandler(void);

  void init(void);
  void deinit(void);
  void registerGpioUri(void);
  esp_err_t handleHttpRequest(httpd_req_t *req);
  void taskHandler(void);
  void setGpioState(GpioResult *gpioResult);
  void ledc_init(int gpioNum, ledc_channel_t channel, int frequency);
  void flashLightControl(bool status, int intensity);
  void getDs18b20Temperature(int gpioNum, int quantity);

  bool isEnabled(void) { return _isEnabled; }

#ifdef ENABLE_MQTT
  void handleMQTTconnect();
#endif // ENABLE_MQTT
};

esp_err_t callHandleHttpRequest(httpd_req_t *req);
void taskGpioHandler(void *pvParameter);

void gpio_handler_create(httpd_handle_t server);
void gpio_handler_init(void);
void gpio_handler_deinit(void);
void gpio_handler_destroy(void);
GpioHandler *gpio_handler_get(void);

#endif // SERVER_GPIO_H
