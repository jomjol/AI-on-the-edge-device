#ifndef INTERFACE_MQTT_H
#define INTERFACE_MQTT_H

#include <string>
#include <map>
#include <functional>

void MQTTInit(std::string _mqttURI, std::string _clientid, std::string _user, std::string _password, std::string _LWTContext, int _keepalive);
void MQTTdestroy();

//void MQTTInit(std::string _mqttURI, std::string _clientid, std::string _user = "", std::string _password = "");

void MQTTPublish(std::string _key, std::string _content, int retained_flag = 0);

bool MQTTisConnected();

void MQTTregisterConnectFunction(std::string name, std::function<void()> func);
void MQTTunregisterConnectFunction(std::string name);
void MQTTregisterSubscribeFunction(std::string topic, std::function<bool(std::string, char*, int)> func);
void MQTTdestroySubscribeFunction();
void MQTTconnected();

#endif //INTERFACE_MQTT_H