#include <string>

void MQTTInit(std::string _mqttURI, std::string _clientid, std::string _user = "", std::string _password = "");
void MQTTPublish(std::string _key, std::string _content);