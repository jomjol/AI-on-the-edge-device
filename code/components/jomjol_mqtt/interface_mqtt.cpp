#include "interface_mqtt.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "mqtt_client.h"
#include "ClassLogFile.h"

static const char *TAG_INTERFACEMQTT = "interface_mqtt";

std::map<std::string, std::function<void()>>* connectFunktionMap = NULL;  
std::map<std::string, std::function<bool(std::string, char*, int)>>* subscribeFunktionMap = NULL;  
bool debugdetail = true;

// #define CONFIG_BROKER_URL "mqtt://192.168.178.43:1883"

esp_mqtt_event_id_t esp_mmqtt_ID = MQTT_EVENT_ANY;

bool mqtt_connected = false;
esp_mqtt_client_handle_t client = NULL;

void MQTTPublish(std::string _key, std::string _content, int retained_flag){
    if (client && mqtt_connected) {
        int msg_id;
        std::string zw;
        msg_id = esp_mqtt_client_publish(client, _key.c_str(), _content.c_str(), 0, 1, retained_flag);
        zw = "sent publish successful in MQTTPublish, msg_id=" + std::to_string(msg_id) + ", " + _key + ", " + _content;
        if (debugdetail) LogFile.WriteToFile(zw);
        ESP_LOGD(TAG_INTERFACEMQTT, "sent publish successful in MQTTPublish, msg_id=%d, %s, %s", msg_id, _key.c_str(), _content.c_str());
    }
    else {
        ESP_LOGW(TAG_INTERFACEMQTT, "Problem with Publish, client=%d, mqtt_connected %d", (int) client, (int) mqtt_connected);
    }
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

void MQTTInit(std::string _mqttURI, std::string _clientid, std::string _user, std::string _password, std::string _LWTContext, int _keepalive){
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

    if (_user.length() && _password.length()){
        mqtt_cfg.username = _user.c_str();
        mqtt_cfg.password = _password.c_str();
        ESP_LOGI(TAG_INTERFACEMQTT, "Connect to MQTT: %s, %s", mqtt_cfg.username, mqtt_cfg.password);
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, esp_mmqtt_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    MQTTPublish(_LWTContext, "", 1);
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
        if (connectFunktionMap != NULL) {
            for(std::map<std::string, std::function<void()>>::iterator it = connectFunktionMap->begin(); it != connectFunktionMap->end(); ++it) {
                it->second();
                ESP_LOGD(TAG_INTERFACEMQTT, "call connect function %s", it->first.c_str());
            }
        }

        if (subscribeFunktionMap != NULL) {
            for(std::map<std::string, std::function<bool(std::string, char*, int)>>::iterator it = subscribeFunktionMap->begin(); it != subscribeFunktionMap->end(); ++it) {
                int msg_id = esp_mqtt_client_subscribe(client, it->first.c_str(), 0);
                ESP_LOGD(TAG_INTERFACEMQTT, "topic %s subscribe successful, msg_id=%d", it->first.c_str(), msg_id);
            }
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