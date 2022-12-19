#ifdef ENABLE_MQTT

#pragma once

#ifndef SERVERMQTT_H
#define SERVERMQTT_H

#include "ClassFlowDefineTypes.h"

void SetHomeassistantDiscoveryEnabled(bool enabled);
void mqttServer_setParameter(std::vector<NumberPost*>* _NUMBERS, int interval, float roundInterval);
void mqttServer_setMeterType(std::string meterType, std::string valueUnit, std::string timeUnit,std::string rateUnit);
void setMqtt_Server_Retain(int SetRetainFlag);
void mqttServer_setMainTopic( std::string maintopic);
std::string mqttServer_getMainTopic();

void register_server_mqtt_uri(httpd_handle_t server);

void publishSystemData();

std::string getTimeUnit(void);
void GotConnected(std::string maintopic, int SetRetainFlag);


#endif //SERVERMQTT_H
#endif //ENABLE_MQTT