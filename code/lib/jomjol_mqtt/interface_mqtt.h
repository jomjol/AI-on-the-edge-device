#include <string>

void MQTTInit(std::string _mqttURI, std::string _clientid);
void MQTTPublish(std::string _key, std::string _content);