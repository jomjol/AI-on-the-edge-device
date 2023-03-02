#ifdef ENABLE_MQTT

#pragma once

#ifndef SERVERMQTT_H
#define SERVERMQTT_H

#include "ClassFlowDefineTypes.h"

void SetHomeassistantDiscoveryEnabled(bool enabled);
void mqttServer_setParameter(std::vector<NumberPost*>* _NUMBERS, int interval, float roundInterval);
void mqttServer_setMeterType(std::string meterType, std::string valueUnit, std::string timeUnit,std::string rateUnit);
void setMqtt_Server_Retain(bool SetRetainFlag);
void mqttServer_setMainTopic( std::string maintopic);
std::string mqttServer_getMainTopic();

void register_server_mqtt_uri(httpd_handle_t server);

bool publishSystemData(int qos);

std::string getTimeUnit(void);
void GotConnected(std::string maintopic, bool SetRetainFlag);
esp_err_t sendDiscovery_and_static_Topics(void);


#endif //SERVERMQTT_H
#endif //ENABLE_MQTT