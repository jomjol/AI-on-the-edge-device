#ifdef ENABLE_MQTT
#include "interface_mqtt.h"

#include "esp_log.h"
#include "connect_wlan.h"
#include "mqtt_client.h"
#include "ClassLogFile.h"
#include "server_tflite.h"
#include "../../include/defines.h"

static const char *TAG = "MQTT IF";

std::map<std::string, std::function<void()>>* connectFunktionMap = NULL;  
std::map<std::string, std::function<bool(std::string, char*, int)>>* subscribeFunktionMap = NULL;

int failedOnRound = -1;
 
esp_mqtt_event_id_t esp_mqtt_ID = MQTT_EVENT_ANY;
// ESP_EVENT_ANY_ID

bool mqtt_enabled = false;
bool mqtt_configOK = false;
bool mqtt_initialized = false;
bool mqtt_connected = false;

esp_mqtt_client_handle_t client = NULL;
std::string uri, client_id, lwt_topic, lwt_connected, lwt_disconnected, user, password, maintopic;
int keepalive, SetRetainFlag;
void (*callbackOnConnected)(std::string, int) = NULL;


bool MQTTPublish(std::string _key, std::string _content, int retained_flag) 
{
    if (!mqtt_enabled) {                            // MQTT sevice not started / configured (MQTT_Init not called before)      
        return false;
    }

    if (failedOnRound == getCountFlowRounds()) {    // we already failed in this round, do not retry until the next round
        return true; // Fail quietly
    }

    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("MQTT Publish");
    #endif

    MQTT_Init(); // Re-Init client if not initialized yet/anymore

    if (mqtt_initialized && mqtt_connected) {
        #ifdef DEBUG_DETAIL_ON 
            long long int starttime = esp_timer_get_time();
        #endif
        int msg_id = esp_mqtt_client_publish(client, _key.c_str(), _content.c_str(), 0, 1, retained_flag);
        #ifdef DEBUG_DETAIL_ON 
            ESP_LOGD(TAG, "Publish msg_id %d in %lld ms", msg_id, (esp_timer_get_time() - starttime)/1000);
        #endif
        if (msg_id == -1) {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Failed to publish topic '" + _key + "', re-trying...");   
            #ifdef DEBUG_DETAIL_ON 
                starttime = esp_timer_get_time();
            #endif
            msg_id = esp_mqtt_client_publish(client, _key.c_str(), _content.c_str(), 0, 1, retained_flag);
            #ifdef DEBUG_DETAIL_ON 
                ESP_LOGD(TAG, "Publish msg_id %d in %lld ms", msg_id, (esp_timer_get_time() - starttime)/1000);
            #endif
            if (msg_id == -1) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to publish topic '" + _key + "', skipping all MQTT publishings in this round!");
                failedOnRound = getCountFlowRounds();
                return false;
            }
        }

        if (_content.length() > 80) { // Truncate message if too long
            _content.resize(80);
            _content.append("..");
        }

        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Published topic: " + _key + ", content: " + _content + " (msg_id=" + std::to_string(msg_id) + ")");
        return true;
    }
    else {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Publish skipped. Client not initalized or not connected. (topic: " + _key + ")");
        return false;
    }
}


static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    std::string topic = "";
    switch (event->event_id) {
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGD(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            mqtt_initialized = true;
            break;
        case MQTT_EVENT_CONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_initialized = true;
            mqtt_connected = true;
            MQTTconnected();
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Disconnected from broker");
            mqtt_connected = false;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGD(TAG, "MQTT_EVENT_DATA");
            ESP_LOGD(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGD(TAG, "DATA=%.*s", event->data_len, event->data);
            topic.assign(event->topic, event->topic_len);
            if (subscribeFunktionMap != NULL) {
                if (subscribeFunktionMap->find(topic) != subscribeFunktionMap->end()) {
                    ESP_LOGD(TAG, "call subcribe function for topic %s", topic.c_str());
                    (*subscribeFunktionMap)[topic](topic, event->data, event->data_len);
                }
            } else {
                ESP_LOGW(TAG, "no handler available\r\n");
            }
            break;
        case MQTT_EVENT_ERROR:
            #ifdef DEBUG_DETAIL_ON 
                ESP_LOGD(TAG, "MQTT_EVENT_ERROR - esp_mqtt_error_codes:");
                ESP_LOGD(TAG, "error_type:%d", event->error_handle->error_type);
                ESP_LOGD(TAG, "connect_return_code:%d", event->error_handle->connect_return_code);
                ESP_LOGD(TAG, "esp_transport_sock_errno:%d", event->error_handle->esp_transport_sock_errno);
                ESP_LOGD(TAG, "esp_tls_last_esp_err:%d", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGD(TAG, "esp_tls_stack_err:%d", event->error_handle->esp_tls_stack_err);
                ESP_LOGD(TAG, "esp_tls_cert_verify_flags:%d", event->error_handle->esp_tls_cert_verify_flags);
            #endif
            mqtt_connected = false;
            break;
        default:
            ESP_LOGD(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb((esp_mqtt_event_handle_t) event_data);
}


bool MQTT_Configure(std::string _mqttURI, std::string _clientid, std::string _user, std::string _password,
        std::string _maintopic, std::string _lwt, std::string _lwt_connected, std::string _lwt_disconnected,
                    int _keepalive, int _SetRetainFlag, void *_callbackOnConnected) {
    if ((_mqttURI.length() == 0) || (_maintopic.length() == 0) || (_clientid.length() == 0)) 
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Init aborted! Config error (URI, MainTopic or ClientID missing)");
        return false;
    }

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

    #ifdef __HIDE_PASSWORD
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "URI: " + uri + ", clientname: " + client_id + ", user: " + user + ", password: XXXXXXXX, maintopic: "
                            + maintopic + ", last-will-topic: " + lwt_topic + ", keepAlive: " + std::to_string(keepalive) + ", RetainFlag: " + std::to_string(SetRetainFlag)); 
    #else
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "URI: " + uri + ", clientname: " + client_id + ", user: " + user + ", password: " + password  + ", maintopic: "
                            + maintopic + ", last-will-topic: " + lwt_topic + ", keepAlive: " + std::to_string(keepalive)  + ", RetainFlag: " + std::to_string(SetRetainFlag)); 
     #endif

    mqtt_configOK = true;
    return true;
}


int MQTT_Init() { 
    if (mqtt_initialized) {
        return 0;
    }

    if (mqtt_configOK) {                           
        mqtt_enabled = true;
    } else {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init called, but client is not yet configured.");
        return 0;
    }

    if (!getWIFIisConnected()) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init called, but WIFI is not yet connected.");
        return 0;
    }

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Init");
    MQTTdestroy_client(false);

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = uri.c_str(),
        .client_id = client_id.c_str(),
        .lwt_topic = lwt_topic.c_str(),
        .lwt_msg = lwt_disconnected.c_str(),
        .lwt_retain = 1,
        .lwt_msg_len = (int)(lwt_disconnected.length()),
        .keepalive = keepalive,
        .disable_auto_reconnect = false,        // Reconnection routine active (Default: false)
        .buffer_size = 1536,                    // size of MQTT send/receive buffer (Default: 1024)
        .reconnect_timeout_ms = 15000,          // Try to reconnect to broker (Default: 10000ms)
        .network_timeout_ms = 20000,            // Network Timeout (Default: 10000ms)
        .message_retransmit_timeout = 3000      // Tiem after message resent when broker not acknowledged (QoS1, QoS2)

    };

    if (user.length() && password.length()){
        mqtt_cfg.username = user.c_str();
        mqtt_cfg.password = password.c_str();
    }

    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("MQTT Client Init");
    #endif

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client)
    {
        esp_err_t ret = esp_mqtt_client_register_event(client, esp_mqtt_ID, mqtt_event_handler, client);
        if (ret != ESP_OK)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Could not register event (ret=" + std::to_string(ret) + ")!");
            mqtt_initialized = false;
            return -1;
        }

        #ifdef DEBUG_DETAIL_ON  
            LogFile.WriteHeapInfo("MQTT Client Start");
        #endif
        ret = esp_mqtt_client_start(client);
        if (ret != ESP_OK)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Client start failed (retval=" + std::to_string(ret) + ")!");
            mqtt_initialized = false;
            return -1;
        }
        else {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Client started, waiting for established connection...");
            mqtt_initialized = true;
            return 1;
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Init failed, no handle created!");
        mqtt_initialized = false;
        return -1;
    }

}


void MQTTdestroy_client(bool _disable = false) {
    if (client) {
        if (mqtt_connected) {
            MQTTdestroySubscribeFunction();      
            esp_mqtt_client_disconnect(client);
            mqtt_connected = false;
        }
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = NULL;
        mqtt_initialized = false;
    }

    if (_disable) // Disable MQTT service, avoid restart with MQTTPublish
        mqtt_configOK = false;
}


bool getMQTTisEnabled() {
    return mqtt_enabled;
}


bool getMQTTisConnected() {
    return mqtt_connected;
}


bool mqtt_handler_flow_start(std::string _topic, char* _data, int _data_len) {
    ESP_LOGD(TAG, "Handler called: topic %s, data %.*s", _topic.c_str(), _data_len, _data);

    if (_data_len > 0) {
        MQTTCtrlFlowStart(_topic);
    }

    return ESP_OK;
}


void MQTTconnected(){
    if (mqtt_connected) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Connected to broker");
        MQTTPublish(lwt_topic, lwt_connected, true);                        // Publish "connected" to maintopic/connection

        if (connectFunktionMap != NULL) {
            for(std::map<std::string, std::function<void()>>::iterator it = connectFunktionMap->begin(); it != connectFunktionMap->end(); ++it) {
                it->second();
                ESP_LOGD(TAG, "call connect function %s", it->first.c_str());
            }
        }

        /* Subcribe to topics */
        std::function<bool(std::string topic, char* data, int data_len)> subHandler = mqtt_handler_flow_start;     
        MQTTregisterSubscribeFunction(maintopic + "/ctrl/flow_start", subHandler);        // subcribe to maintopic/ctrl/flow_start

       if (subscribeFunktionMap != NULL) {
            for(std::map<std::string, std::function<bool(std::string, char*, int)>>::iterator it = subscribeFunktionMap->begin(); it != subscribeFunktionMap->end(); ++it) {
                int msg_id = esp_mqtt_client_subscribe(client, it->first.c_str(), 0);
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "topic " + it->first + " subscribe successful, msg_id=" + std::to_string(msg_id));
            }
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);                 // Delay execution of callback routine after connection got established   
        if (callbackOnConnected) {                              // Call onConnected callback routine --> mqtt_server
            callbackOnConnected(maintopic, SetRetainFlag);
        }
    }
}


void MQTTregisterConnectFunction(std::string name, std::function<void()> func){
    ESP_LOGD(TAG, "MQTTregisteronnectFunction %s\r\n", name.c_str());
    if (connectFunktionMap == NULL) {
        connectFunktionMap = new std::map<std::string, std::function<void()>>();
    }

    if ((*connectFunktionMap)[name] != NULL) {
        ESP_LOGW(TAG, "connect function %s already registred", name.c_str());
        return;
    }

    (*connectFunktionMap)[name] = func;

    if (mqtt_connected) {
        func();
    }
}


void MQTTunregisterConnectFunction(std::string name){
    ESP_LOGD(TAG, "unregisterConnnectFunction %s\r\n", name.c_str());
    if ((connectFunktionMap != NULL) && (connectFunktionMap->find(name) != connectFunktionMap->end())) {
        connectFunktionMap->erase(name);
    }
}


void MQTTregisterSubscribeFunction(std::string topic, std::function<bool(std::string, char*, int)> func){
    ESP_LOGD(TAG, "registerSubscribeFunction %s", topic.c_str());
    if (subscribeFunktionMap == NULL) {
        subscribeFunktionMap = new std::map<std::string, std::function<bool(std::string, char*, int)>>();
    }

    if ((*subscribeFunktionMap)[topic] != NULL) {
        ESP_LOGW(TAG, "topic %s already registered for subscription", topic.c_str());
        return;
    }

    (*subscribeFunktionMap)[topic] = func;
}


void MQTTdestroySubscribeFunction(){
    if (subscribeFunktionMap != NULL) {
        if (mqtt_connected) {
            for(std::map<std::string, std::function<bool(std::string, char*, int)>>::iterator it = subscribeFunktionMap->begin(); it != subscribeFunktionMap->end(); ++it) {
                int msg_id = esp_mqtt_client_unsubscribe(client, it->first.c_str());
                ESP_LOGD(TAG, "topic %s unsubscribe successful, msg_id=%d", it->first.c_str(), msg_id);
            }
        }

        subscribeFunktionMap->clear();
        delete subscribeFunktionMap;
        subscribeFunktionMap = NULL;
    }
}
#endif //ENABLE_MQTT
