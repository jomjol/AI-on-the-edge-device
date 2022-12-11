#ifdef ENABLE_MQTT

#include "ClassFlowDefineTypes.h"

#define LWT_TOPIC        "connection"
#define LWT_CONNECTED    "connected"
#define LWT_DISCONNECTED "connection lost"


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

#endif //ENABLE_MQTT