#include "interface_mqtt.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "mqtt_client.h"
#include "ClassLogFile.h"

#define __HIDE_PASSWORD

static const char *TAG_INTERFACEMQTT = "interface_mqtt";

std::map<std::string, std::function<void()>>* connectFunktionMap = NULL;  
std::map<std::string, std::function<bool(std::string, char*, int)>>* subscribeFunktionMap = NULL;  


esp_mqtt_event_id_t esp_mmqtt_ID = MQTT_EVENT_ANY;
// ESP_EVENT_ANY_ID

bool mqtt_connected = false;
esp_mqtt_client_handle_t client = NULL;
std::string uri, client_id, lwt_topic, lwt_connected, lwt_disconnected, user, password, maintopic;
int keepalive, SetRetainFlag;
void (*callbackOnConnected)(std::string, int) = NULL;


bool MQTTPublish(std::string _key, std::string _content, int retained_flag) {
    int msg_id;
    std::string zw;

    if (!mqtt_connected) {
        LogFile.WriteToFile(ESP_LOG_WARN, "MQTT - Not connected, trying to re-connect...");
        if (!MQTT_Init()) {
            if (!MQTT_Init()) { // Retry
                LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Failed to init!");
                return false;
            }
        }
    }    

    msg_id = esp_mqtt_client_publish(client, _key.c_str(), _content.c_str(), 0, 1, retained_flag);
    if (msg_id < 0) {
        LogFile.WriteToFile(ESP_LOG_WARN, "MQTT - Failed to publish topic '" + _key + "', re-trying...");

        msg_id = esp_mqtt_client_publish(client, _key.c_str(), _content.c_str(), 0, 1, retained_flag);
        if (msg_id < 0) {
            LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Failed to publish topic '" + _key + "'!");
            mqtt_connected = false; // Force re-init on next call
            return false;
        }
    }

    zw = "MQTT - Published topic: " + _key + ", content: " + _content + " (msg_id=" + std::to_string(msg_id) + ")";
    LogFile.WriteToFile(ESP_LOG_DEBUG, zw);

    return true;
}


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    int msg_id;
    std::string topic = "";
    switch (event->event_id) {
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_BEFORE_CONNECT");
            break;
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_CONNECTED");
            mqtt_connected = true;
            MQTTconnected();
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_DISCONNECTED");
            mqtt_connected = false; // Force re-init on next call
            esp_mqtt_client_reconnect(client);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG_INTERFACEMQTT, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG_INTERFACEMQTT, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGI(TAG_INTERFACEMQTT, "DATA=%.*s\r\n", event->data_len, event->data);
            topic.assign(event->topic, event->topic_len);
            if (subscribeFunktionMap != NULL) {
                if (subscribeFunktionMap->find(topic) != subscribeFunktionMap->end()) {
                    ESP_LOGD(TAG_INTERFACEMQTT, "call handler function\r\n");
                    (*subscribeFunktionMap)[topic](topic, event->data, event->data_len);
                }
            } else {
                ESP_LOGW(TAG_INTERFACEMQTT, "no handler available\r\n");
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_INTERFACEMQTT, "MQTT_EVENT_ERROR");
            mqtt_connected = false; // Force re-init on next call
            break;
        default:
            ESP_LOGI(TAG_INTERFACEMQTT, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG_INTERFACEMQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb((esp_mqtt_event_handle_t) event_data);
}


void MQTT_Configure(std::string _mqttURI, std::string _clientid, std::string _user, std::string _password,
        std::string _maintopic, std::string _lwt, std::string _lwt_connected, std::string _lwt_disconnected,
        int _keepalive, int _SetRetainFlag, void *_callbackOnConnected){
#ifdef __HIDE_PASSWORD
    LogFile.WriteToFile(ESP_LOG_INFO, "MQTT Configuration: uri: " + _mqttURI + ", clientname: " + _clientid + 
            ", user: " + _user + ", password: XXXXXXXX, maintopic: " + _maintopic + ", last-will-topic: " + _maintopic + "/" + _lwt + ", keepAlive: " + std::to_string(_keepalive)); 
#else
    LogFile.WriteToFile(ESP_LOG_INFO, "MQTT Configuration: uri: " + _mqttURI + ", clientname: " + _clientid + 
            ", user: " + _user + ", password: " + _password + ", maintopic: " + _maintopic + ", last-will-topic: " + _maintopic + "/" + _lwt + ", keepAlive: " + std::to_string(_keepalive)); 
#endif

    uri = _mqttURI;
    client_id = _clientid;
    lwt_topic = _maintopic + "/" + _lwt;
    lwt_connected = _lwt_connected;
    lwt_disconnected = _lwt_disconnected;
    keepalive = _keepalive;
    SetRetainFlag = _SetRetainFlag;
    maintopic = _maintopic;
    callbackOnConnected = ( void (*)(std::string, int) )(_callbackOnConnected);

    if (_user.length() && _password.length()){
        user = _user;
        password = _password;
    }
}

bool MQTT_Init() {
    esp_err_t ret;
    LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - Init");

    MQTTdestroy_client();

    std::string lw = lwt_disconnected;

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = uri.c_str(),
        .client_id = client_id.c_str(),
        .lwt_topic = lwt_topic.c_str(),
        .lwt_msg = lw.c_str(),
        .lwt_retain = 1,
        .lwt_msg_len = (int)(lw.length()),
        .keepalive = keepalive
    };

    if (user.length() && password.length()){
        mqtt_cfg.username = user.c_str();
        mqtt_cfg.password = password.c_str();
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client)
    {
        ret = esp_mqtt_client_register_event(client, esp_mmqtt_ID, mqtt_event_handler, client);
        if (ret != ESP_OK)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Could not register event (ret=" + std::to_string(ret) + ")!");
            return false;
        }

        ret = esp_mqtt_client_start(client);
        if (ret != ESP_OK)
        {
            LogFile.WriteToFile(ESP_LOG_WARN, "MQTT - Could not start client (ret=" + std::to_string(ret) + "), retrying...");
            ret = esp_mqtt_client_start(client);
            if (ret != ESP_OK)
            {
                LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Could not start client (ret=" + std::to_string(ret) + ")!");
                return false;
            }
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, "MQTT - Could not init client!");
        return false;
    }

    LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - Init successful");
    return true;
}


void MQTTdestroy_client() {
    if (client != NULL) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
    }
}

bool MQTTisConnected() {
    return mqtt_connected;
}

void MQTTregisterConnectFunction(std::string name, std::function<void()> func){
    ESP_LOGD(TAG_INTERFACEMQTT, "MQTTregisteronnectFunction %s\r\n", name.c_str());
    if (connectFunktionMap == NULL) {
        connectFunktionMap = new std::map<std::string, std::function<void()>>();
    }

    if ((*connectFunktionMap)[name] != NULL) {
        ESP_LOGW(TAG_INTERFACEMQTT, "connect function %s already registred", name.c_str());
        return;
    }

    (*connectFunktionMap)[name] = func;

    if (mqtt_connected) {
        func();
    }
}

void MQTTunregisterConnectFunction(std::string name){
    ESP_LOGD(TAG_INTERFACEMQTT, "MQTTregisteronnectFunction %s\r\n", name.c_str());
    if ((connectFunktionMap != NULL) && (connectFunktionMap->find(name) != connectFunktionMap->end())) {
        connectFunktionMap->erase(name);
    }
}

void MQTTregisterSubscribeFunction(std::string topic, std::function<bool(std::string, char*, int)> func){
    ESP_LOGD(TAG_INTERFACEMQTT, "MQTTregisterSubscribeFunction %s\r\n", topic.c_str());
    if (subscribeFunktionMap == NULL) {
        subscribeFunktionMap = new std::map<std::string, std::function<bool(std::string, char*, int)>>();
    }

    if ((*subscribeFunktionMap)[topic] != NULL) {
        ESP_LOGW(TAG_INTERFACEMQTT, "topic %s already registred for subscription", topic.c_str());
        return;
    }

    (*subscribeFunktionMap)[topic] = func;

    if (mqtt_connected) {
        int msg_id = esp_mqtt_client_subscribe(client, topic.c_str(), 0);
        ESP_LOGD(TAG_INTERFACEMQTT, "topic %s subscribe successful, msg_id=%d", topic.c_str(), msg_id);
    }
}

void MQTTconnected(){
    if (mqtt_connected) {
        LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - Connected");

        MQTTPublish(lwt_topic, lwt_connected, true);

        if (connectFunktionMap != NULL) {
            for(std::map<std::string, std::function<void()>>::iterator it = connectFunktionMap->begin(); it != connectFunktionMap->end(); ++it) {
                it->second();
                ESP_LOGD(TAG_INTERFACEMQTT, "call connect function %s", it->first.c_str());
            }
        }

       if (subscribeFunktionMap != NULL) {
            for(std::map<std::string, std::function<bool(std::string, char*, int)>>::iterator it = subscribeFunktionMap->begin(); it != subscribeFunktionMap->end(); ++it) {
                int msg_id = esp_mqtt_client_subscribe(client, it->first.c_str(), 0);
                LogFile.WriteToFile(ESP_LOG_INFO, "MQTT - topic " + it->first + " subscribe successful, msg_id=" + std::to_string(msg_id));
            }
        }

        if (callbackOnConnected) {
            callbackOnConnected(maintopic, SetRetainFlag);
        }
    }
}

void MQTTdestroySubscribeFunction(){
    if (subscribeFunktionMap != NULL) {
        if (mqtt_connected) {
            for(std::map<std::string, std::function<bool(std::string, char*, int)>>::iterator it = subscribeFunktionMap->begin(); it != subscribeFunktionMap->end(); ++it) {
                int msg_id = esp_mqtt_client_unsubscribe(client, it->first.c_str());
                ESP_LOGI(TAG_INTERFACEMQTT, "topic %s unsubscribe successful, msg_id=%d", it->first.c_str(), msg_id);
            }
        }

        subscribeFunktionMap->clear();
        delete subscribeFunktionMap;
        subscribeFunktionMap = NULL;
    }
}
