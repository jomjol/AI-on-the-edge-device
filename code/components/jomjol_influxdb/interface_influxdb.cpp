#include "interface_influxdb.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include <time.h>
#include "ClassLogFile.h"
#include "esp_http_client.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char *TAG_INTERFACEINFLUXDB = "interface_influxdb";

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
            ESP_LOGE(TAG_INTERFACEINFLUXDB, "HTTP Client Error encountered");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG_INTERFACEINFLUXDB, "HTTP Client Connected");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGV(TAG_INTERFACEINFLUXDB, "HTTP Client sent all request headers");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGV(TAG_INTERFACEINFLUXDB, "Header: key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGV(TAG_INTERFACEINFLUXDB, "HTTP Client data recevied: len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG_INTERFACEINFLUXDB, "HTTP Client finished");
            break;
         case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_INTERFACEINFLUXDB, "HTTP Client Disconnected");
            break;
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
	case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG_INTERFACEINFLUXDB, "HTTP Client Redirect");
	    break;
#endif
    }
    return ESP_OK;
}

void InfluxDBPublish(std::string _key, std::string _content, std::string _timestamp) {
    char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t http_config;

    http_config.user_agent = "ESP32 Meter reader";
    http_config.method = HTTP_METHOD_POST;
    http_config.event_handler = http_event_handler;
    http_config.buffer_size = MAX_HTTP_OUTPUT_BUFFER;
    http_config.user_data = response_buffer;

    if (_influxDBUser.length() && _influxDBPassword.length()){
       http_config.username = _influxDBUser.c_str();
       http_config.password = _influxDBPassword.c_str();
       http_config.auth_type = HTTP_AUTH_TYPE_BASIC;
    }

    // generate timestamp (TODO: parse result timestamp passed as string and convert it to POSIX timestamp?)
    time_t now = time(NULL);
    char nowTimestamp[21];
    // pad with zeroes to get nanoseconds
    sprintf(nowTimestamp,"%jd000000000", (intmax_t)now);
    
    std::string payload = _influxDBMeasurement + " " + _key + "=" + _content + " " + nowTimestamp;
    payload.shrink_to_fit();
    ESP_LOGI(TAG_INTERFACEINFLUXDB, "sending line to influxdb: %s\n", payload.c_str());

    // use the default retention policy of the database
    std::string apiURI = _influxDBURI + "/api/v2/write?bucket=" + _influxDBDatabase + "/";
    apiURI.shrink_to_fit();
    http_config.url = apiURI.c_str();
    ESP_LOGI(TAG_INTERFACEINFLUXDB, "API URI: %s", apiURI.c_str());

    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);
    ESP_LOGI(TAG_INTERFACEINFLUXDB, "client is initialized%s\n", "");

    esp_http_client_set_header(http_client, "Content-Type", "text/plain");
    ESP_LOGI(TAG_INTERFACEINFLUXDB, "header is set%s\n", "");

    ESP_ERROR_CHECK(esp_http_client_set_post_field(http_client, payload.c_str(), payload.length()));
    ESP_LOGI(TAG_INTERFACEINFLUXDB, "post payload is set%s\n", "");

    esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_perform(http_client));

    if( err == ESP_OK ) {
      ESP_LOGI(TAG_INTERFACEINFLUXDB, "HTTP request was performed%s\n", "");
      int status_code = esp_http_client_get_status_code(http_client);
      ESP_LOGI(TAG_INTERFACEINFLUXDB, "HTTP status code %d\n", status_code);
    } else {
      ESP_LOGW(TAG_INTERFACEINFLUXDB, "HTTP request failed%s\n", "");
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


