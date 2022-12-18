#ifdef ENABLE_MQTT
#ifndef INTERFACE_MQTT_H
#define INTERFACE_MQTT_H

#include <string>
#include <map>
#include <functional>

bool MQTT_Configure(std::string _mqttURI, std::string _clientid, std::string _user, std::string _password,
                    std::string _maintopic, std::string _lwt, std::string _lwt_connected, std::string _lwt_disconnected,
                    int _keepalive, int SetRetainFlag, void *callbackOnConnected);
int MQTT_Init();
void MQTTdestroy_client();

bool MQTTPublish(std::string _key, std::string _content, int retained_flag = 1);            // retained Flag as Standart

bool getMQTTisEnabled();
bool getMQTTisConnected();

void MQTTregisterConnectFunction(std::string name, std::function<void()> func);
void MQTTunregisterConnectFunction(std::string name);
void MQTTregisterSubscribeFunction(std::string topic, std::function<bool(std::string, char*, int)> func);
void MQTTdestroySubscribeFunction();
void MQTTconnected();

void MQTTdisable();
#endif //INTERFACE_MQTT_H
#endif //#ENABLE_MQTT
