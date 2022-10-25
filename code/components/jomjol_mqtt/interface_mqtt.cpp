#include "interface_mqtt.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "mqtt_client.h"
#include "ClassLogFile.h"

#define __HIDE_PASSWORD

static const char *TAG_INTERFACEMQTT = "interface_mqtt";

std::map<std::string, std::function<void()>>* connectFunktionMap = NULL;  
std::map<std::string, std::function<bool(std::string, char*, int)>>* subscribeFunktionMap = NULL;  

// #define CONFIG_BROKER_URL "mqtt://192.168.178.43:1883"

esp_mqtt_event_id_t esp_mmqtt_ID = MQTT_EVENT_ANY;
// ESP_EVENT_ANY_ID

bool mqtt_connected = false;
esp_mqtt_client_handle_t client = NULL;

bool MQTTPublish(std::string _key, std::string _content, int retained_flag){
  
    int msg_id;
    std::string zw;
    msg_id = esp_mqtt_client_publish(client, _key.c_str(), _content.c_str(), 0, 1, retained_flag);
    if (msg_id < 0) {
        LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Failed to publish topic '" + _key + "'!");
        return false;
    }
    zw = "MQTT - Published topic: " + _key + ", content: " + _content + " (msg_id=" + std::to_string(msg_id) + ")";
    LogFile.WriteToFile(ESP_LOG_DEBUG, zw);
    return true;
}


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    int msg_id;
    std::string topic = "";
    std::string zw;
    switch (event->event_id) {
        case MQTT_EVENT_BEFORE_CONNECT:
            LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTT_EVENT_BEFORE_CONNECT");
            break;
        case MQTT_EVENT_CONNECTED:
            LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTT_EVENT_CONNECTED");
            mqtt_connected = true;
            MQTTconnected();
            break;
        case MQTT_EVENT_DISCONNECTED:
            LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTT_EVENT_DISCONNECTED");
            esp_mqtt_client_reconnect(client);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            LogFile.WriteToFile(ESP_LOG_DEBUG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            zw = "MQTT_EVENT_DATA, TOPIC=" + std::string(event->topic) + ", DATA=" + std::string(event->data);
            LogFile.WriteToFile(ESP_LOG_DEBUG, zw);
            topic.assign(event->topic, event->topic_len);
            if (subscribeFunktionMap != NULL) {
                if (subscribeFunktionMap->find(topic) != subscribeFunktionMap->end()) {
                    LogFile.WriteToFile(ESP_LOG_DEBUG, "call handler function");
                    (*subscribeFunktionMap)[topic](topic, event->data, event->data_len);
                }
            } else {
                LogFile.WriteToFile(ESP_LOG_WARN, "no handler available");
            }
            break;
        case MQTT_EVENT_ERROR:
            LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT_EVENT_ERROR");
            break;
        default:
            LogFile.WriteToFile(ESP_LOG_DEBUG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    LogFile.WriteToFile(ESP_LOG_DEBUG, "Event dispatched from event loop base=" + std::string(base) + ", event_id=" + std::to_string(event_id));
    mqtt_event_handler_cb((esp_mqtt_event_handle_t) event_data);
}


bool MQTTInit(std::string _mqttURI, std::string _clientid, std::string _user, std::string _password, std::string _LWTContext, int _keepalive){
    std::string _zwmessage = "connection lost";

    int _lzw = _zwmessage.length();

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = _mqttURI.c_str(),
        .client_id = _clientid.c_str(),
        .lwt_topic = _LWTContext.c_str(),
        .lwt_msg = _zwmessage.c_str(),
        .lwt_retain = 1,
        .lwt_msg_len = _lzw,
        .keepalive = _keepalive
    };

    LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - Init (client ID: " +  _clientid + ")");

    if (_user.length() && _password.length()){
        mqtt_cfg.username = _user.c_str();
        mqtt_cfg.password = _password.c_str();

#ifdef __HIDE_PASSWORD
        LogFile.WriteToFile(ESP_LOG_INFO, "Connect to MQTT: %s, XXXXXXXX", mqtt_cfg.username);
#else
        LogFile.WriteToFile(ESP_LOG_INFO, "Connect to MQTT: %s, %s", mqtt_cfg.username, mqtt_cfg.password);
#endif        
    };

    MQTTdestroy();
    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client)
    {
        if (esp_mqtt_client_register_event(client, esp_mmqtt_ID, mqtt_event_handler, client) != ESP_OK)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Could not register event!");
            return false;
        }
        if (esp_mqtt_client_start(client) != ESP_OK)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Could not start client!");
            return false;
        }

       /* if(!MQTTPublish(_LWTContext, "", 1))
        {
            LogFile.WriteToFile("MQTT - Could not publish LWT!");
            return false;
        }*/
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Could not Init client!");
        return false;
    }

    LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Init successful");
    return true;
}


void MQTTdestroy() {
    if (client != NULL) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
    }
}

bool MQTTisConnected() {
    return mqtt_connected;
}

void MQTTregisterConnectFunction(std::string name, std::function<void()> func){
    LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTTregisteronnectFunction %s", name.c_str());
    if (connectFunktionMap == NULL) {
        connectFunktionMap = new std::map<std::string, std::function<void()>>();
    }

    if ((*connectFunktionMap)[name] != NULL) {
        LogFile.WriteToFile(ESP_LOG_WARN, "connect function %s already registred", name.c_str());
        return;
    }

    (*connectFunktionMap)[name] = func;

    if (mqtt_connected) {
        func();
    }
}

void MQTTunregisterConnectFunction(std::string name){
    LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTTregisteronnectFunction %s", name.c_str());
    if ((connectFunktionMap != NULL) && (connectFunktionMap->find(name) != connectFunktionMap->end())) {
        connectFunktionMap->erase(name);
    }
}

void MQTTregisterSubscribeFunction(std::string topic, std::function<bool(std::string, char*, int)> func){
    LogFile.WriteToFile(ESP_LOG_DEBUG, "MQTTregisterSubscribeFunction %s", topic.c_str());
    if (subscribeFunktionMap == NULL) {
        subscribeFunktionMap = new std::map<std::string, std::function<bool(std::string, char*, int)>>();
    }

    if ((*subscribeFunktionMap)[topic] != NULL) {
        LogFile.WriteToFile(ESP_LOG_WARN, "topic %s already registred for subscription", topic.c_str());
        return;
    }

    (*subscribeFunktionMap)[topic] = func;

    if (mqtt_connected) {
        int msg_id = esp_mqtt_client_subscribe(client, topic.c_str(), 0);
        LogFile.WriteToFile(ESP_LOG_DEBUG, "topic " + std::string(topic.c_str()) + " subscribe successful, msg_id=" + std::to_string(msg_id));
    }
}

void MQTTconnected(){
    if (mqtt_connected) {
        LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - Connected");
        if (connectFunktionMap != NULL) {
            for(std::map<std::string, std::function<void()>>::iterator it = connectFunktionMap->begin(); it != connectFunktionMap->end(); ++it) {
                it->second();
                LogFile.WriteToFile(ESP_LOG_DEBUG, "call connect function %s", it->first.c_str());
            }
        }

       if (subscribeFunktionMap != NULL) {
            for(std::map<std::string, std::function<bool(std::string, char*, int)>>::iterator it = subscribeFunktionMap->begin(); it != subscribeFunktionMap->end(); ++it) {
                int msg_id = esp_mqtt_client_subscribe(client, it->first.c_str(), 0);
                LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - topic " + it->first + " subscribe successful, msg_id=" + std::to_string(msg_id));
            }
        }
    }
}

void MQTTdestroySubscribeFunction(){
    if (subscribeFunktionMap != NULL) {
        if (mqtt_connected) {
            for(std::map<std::string, std::function<bool(std::string, char*, int)>>::iterator it = subscribeFunktionMap->begin(); it != subscribeFunktionMap->end(); ++it) {
                int msg_id = esp_mqtt_client_unsubscribe(client, it->first.c_str());
                LogFile.WriteToFile(ESP_LOG_INFO, "topic " + std::string(it->first.c_str()) + " unsubscribe successful, msg_id=" + std::to_string(msg_id));
            }
        }

        subscribeFunktionMap->clear();
        delete subscribeFunktionMap;
        subscribeFunktionMap = NULL;
    }
}