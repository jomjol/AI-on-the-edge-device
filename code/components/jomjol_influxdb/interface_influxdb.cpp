#ifdef ENABLE_INFLUXDB
#include "interface_influxdb.h"

#include "esp_log.h"
#include <time.h>
#include "ClassLogFile.h"
#include "esp_http_client.h"
#include "time_sntp.h"
#include "../../include/defines.h"


static const char *TAG = "INFLUXDB";

std::string _influxDBURI;
std::string _influxDBDatabase;
std::string _influxDBUser;
std::string _influxDBPassword;

std::string _influxDB_V2_URI;
std::string _influxDB_V2_Bucket;
std::string _influxDB_V2_Token;
std::string _influxDB_V2_Org;

static esp_err_t http_event_handler(esp_http_client_event_t *evt);

void InfluxDB_V2_Init(std::string _uri, std::string _bucket, std::string _org, std::string _token)
{
    _influxDB_V2_URI = _uri;
    _influxDB_V2_Bucket = _bucket;
    _influxDB_V2_Org = _org;
    _influxDB_V2_Token = _token;
}

void InfluxDB_V2_Publish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC) 
{
    char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t http_config = {
       .user_agent = "ESP32 Meter reader",
       .method = HTTP_METHOD_POST,
       .event_handler = http_event_handler,
       .buffer_size = MAX_HTTP_OUTPUT_BUFFER,
       .user_data = response_buffer
    };

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDB_V2_Publish - Key: " + _key + ", Content: " + _content + ", timeUTC: " + std::to_string(_timeUTC));

    std::string payload;
    char nowTimestamp[21];

    if (_timeUTC > 0)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Timestamp (UTC): " + std::to_string(_timeUTC));
        sprintf(nowTimestamp,"%ld000000000", _timeUTC);           // UTC
        payload = _measurement + " " + _key + "=" + _content + " " + nowTimestamp;
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "no timestamp given");
        payload = _measurement + " " + _key + "=" + _content;
    }

    payload.shrink_to_fit();

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "sending line to influxdb:" + payload);

    std::string apiURI = _influxDB_V2_URI + "/api/v2/write?org=" + _influxDB_V2_Org + "&bucket=" + _influxDB_V2_Bucket;
    apiURI.shrink_to_fit();
    http_config.url = apiURI.c_str();
    ESP_LOGI(TAG, "http_config: %s", http_config.url); // Add mark on log to see when it restarted

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "API URI: " + apiURI);

    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "client is initialized");

    esp_http_client_set_header(http_client, "Content-Type", "text/plain");
    std::string _zw = "Token " + _influxDB_V2_Token;
    //    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Tokenheader: %s\n", _zw.c_str());
    esp_http_client_set_header(http_client, "Authorization", _zw.c_str());

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "header is set");

    ESP_ERROR_CHECK(esp_http_client_set_post_field(http_client, payload.c_str(), payload.length()));
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "post payload is set");

    esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_perform(http_client));

    if( err == ESP_OK ) {
      LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP request was performed");
      int status_code = esp_http_client_get_status_code(http_client);
      LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP status code" + std::to_string(status_code));
    } else {
      LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP request failed");
    }
    esp_http_client_cleanup(http_client);
}



static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP Client Error encountered");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP Client connected");
            ESP_LOGI(TAG, "HTTP Client Connected");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP Client sent all request headers");
            break;
        case HTTP_EVENT_ON_HEADER:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Header: key=" + std::string(evt->header_key) + ", value="  + std::string(evt->header_value));
            break;
        case HTTP_EVENT_ON_DATA:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP Client data recevied: len=" + std::to_string(evt->data_len));
            break;
        case HTTP_EVENT_ON_FINISH:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP Client finished");
            break;
         case HTTP_EVENT_DISCONNECTED:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP Client Disconnected");
            break;
        case HTTP_EVENT_REDIRECT:
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP Redirect");
            break;
    }
    return ESP_OK;
}

void InfluxDBPublish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC) {
    char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t http_config = {
       .user_agent = "ESP32 Meter reader",
       .method = HTTP_METHOD_POST,
       .event_handler = http_event_handler,
       .buffer_size = MAX_HTTP_OUTPUT_BUFFER,
       .user_data = response_buffer
    };

    if (_influxDBUser.length() && _influxDBPassword.length()){
       http_config.username = _influxDBUser.c_str();
       http_config.password = _influxDBPassword.c_str();
       http_config.auth_type = HTTP_AUTH_TYPE_BASIC;
    }

    std::string payload;
    char nowTimestamp[21];

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDBPublish - Key: " + _key + ", Content: " + _content + ", timeUTC: " + std::to_string(_timeUTC));

    if (_timeUTC > 0)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Timestamp (UTC): " + std::to_string(_timeUTC));
        sprintf(nowTimestamp,"%ld000000000", _timeUTC);           // UTC
        payload = _measurement + " " + _key + "=" + _content + " " + nowTimestamp;
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "no timestamp given");
        payload = _measurement + " " + _key + "=" + _content;
    }

    payload.shrink_to_fit();

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "sending line to influxdb:" + payload);


    // use the default retention policy of the bucket
    std::string apiURI = _influxDBURI + "/write?db=" + _influxDBDatabase;
//    std::string apiURI = _influxDBURI + "/api/v2/write?bucket=" + _influxDBDatabase + "/";

    apiURI.shrink_to_fit();
    http_config.url = apiURI.c_str();

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "API URI: " + apiURI);

    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "client is initialized");

    esp_http_client_set_header(http_client, "Content-Type", "text/plain");
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "header is set");

    ESP_ERROR_CHECK(esp_http_client_set_post_field(http_client, payload.c_str(), payload.length()));
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "post payload is set");

    esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_perform(http_client));

    if( err == ESP_OK ) {
      LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP request was performed");
      int status_code = esp_http_client_get_status_code(http_client);
      LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP status code" + std::to_string(status_code));
    } else {
      LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP request failed");
    }
    esp_http_client_cleanup(http_client);
}


void InfluxDBInit(std::string _uri, std::string _database, std::string _user, std::string _password){
    _influxDBURI = _uri;
    _influxDBDatabase = _database;
    _influxDBUser = _user;
    _influxDBPassword = _password;
 
}

void InfluxDBdestroy() {
}

#endif //ENABLE_INFLUXDB
