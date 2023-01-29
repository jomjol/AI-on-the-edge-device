#ifdef ENABLE_INFLUXDB
#include "interface_influxdb.h"

#include "esp_log.h"
#include <time.h>
#include "ClassLogFile.h"
#include "esp_http_client.h"
#include "../../include/defines.h"


static const char *TAG = "INFLUXDB";

std::string _influxDBURI;
std::string _influxDBDatabase;
std::string _influxDBMeasurement;
std::string _influxDBUser;
std::string _influxDBPassword;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "HTTP Client Error encountered");
//            ESP_LOGE(TAG, "HTTP Client Error encountered");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "HTTP Client Error encountered");
            ESP_LOGI(TAG, "HTTP Client Connected");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "HTTP Client sent all request headers");
//            ESP_LOGV(TAG, "HTTP Client sent all request headers");
            break;
        case HTTP_EVENT_ON_HEADER:
            LogFile.WriteToFile(ESP_LOG_VERBOSE, TAG, "Header: key=" + std::string(evt->header_key) + ", value="  + std::string(evt->header_value));
//            ESP_LOGV(TAG, "Header: key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            LogFile.WriteToFile(ESP_LOG_VERBOSE, TAG, "HTTP Client data recevied: len=" + std::to_string(evt->data_len));
//            ESP_LOGV(TAG, "HTTP Client data recevied: len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP Client finished");
//            ESP_LOGI(TAG, "HTTP Client finished");
            break;
         case HTTP_EVENT_DISCONNECTED:
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP Client Disconnected");
//            ESP_LOGI(TAG, "HTTP Client Disconnected");
            break;
    }
    return ESP_OK;
}

void InfluxDBPublish(std::string _key, std::string _content, std::string _timestamp) {
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

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "InfluxDBPublish - Key: " + _key + ", Content: " + _content + ", Timestamp: " + _timestamp);

    // generate timestamp (TODO: parse result timestamp passed as string and convert it to POSIX timestamp?)
    // Format:     #define PREVALUE_TIME_FORMAT_OUTPUT "%Y-%m-%dT%H:%M:%S%z"
    struct tm tm;
    strptime(_timestamp.c_str(), PREVALUE_TIME_FORMAT_OUTPUT, &tm);
    time_t t = mktime(&tm);  // t is now your desired time_t


//    time_t now = time(NULL);
    time_t now;
    time(&now);
    char nowTimestamp[21];
    // pad with zeroes to get nanoseconds
    sprintf(nowTimestamp,"%ld000000000", (long) now);
    

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Test Time Conversion - now: " + std::to_string(now) + ", timestamp: " + std::to_string(t)  + "(correct time not used yet)");

    std::string payload = _influxDBMeasurement + " " + _key + "=" + _content + " " + nowTimestamp;
    payload.shrink_to_fit();

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "sending line to influxdb:" + payload);

//    ESP_LOGI(TAG, "sending line to influxdb: %s\n", payload.c_str());

    // use the default retention policy of the database
    std::string apiURI = _influxDBURI + "/api/v2/write?bucket=" + _influxDBDatabase + "/";
    apiURI.shrink_to_fit();
    http_config.url = apiURI.c_str();

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "API URI: " + apiURI);
//    ESP_LOGI(TAG, "API URI: %s", apiURI.c_str());

    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "client is initialized");
//    ESP_LOGI(TAG, "client is initialized%s\n", "");

    esp_http_client_set_header(http_client, "Content-Type", "text/plain");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "header is set");
//    ESP_LOGI(TAG, "header is set%s\n", "");

    ESP_ERROR_CHECK(esp_http_client_set_post_field(http_client, payload.c_str(), payload.length()));
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "post payload is set");
//    ESP_LOGI(TAG, "post payload is set%s\n", "");

    esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_perform(http_client));

    if( err == ESP_OK ) {
      LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP request was performed");
//      ESP_LOGI(TAG, "HTTP request was performed%s\n", "");
      int status_code = esp_http_client_get_status_code(http_client);
      LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP status code" + std::to_string(status_code));
//      ESP_LOGI(TAG, "HTTP status code %d\n", status_code);
    } else {
      LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP request failed");
//      ESP_LOGW(TAG, "HTTP request failed%s\n", "");
    }
    esp_http_client_cleanup(http_client);
}


void InfluxDBInit(std::string _uri, std::string _database, std::string _measurement, std::string _user, std::string _password){
    _influxDBURI = _uri;
    _influxDBDatabase = _database;
    _influxDBMeasurement = _measurement;
    _influxDBUser = _user;
    _influxDBPassword = _password;
 
}

void InfluxDBdestroy() {
}

#endif //ENABLE_INFLUXDB
