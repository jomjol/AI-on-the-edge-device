#ifdef ENABLE_INFLUXDB
#include "interface_influxdb.h"

#include "esp_log.h"
#include <time.h>
#include "ClassLogFile.h"
#include "esp_http_client.h"
#include "time_sntp.h"
#include "../../include/defines.h"

static const char *TAG = "INFLUXDB";

char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};


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



    void InfluxDB::InfluxDBInitV1(std::string _influxDBURI, std::string _database, std::string _user, std::string _password) {
        version = INFLUXDB_V1;
        influxDBURI = _influxDBURI;
        database = _database;
        user = _user;
        password = _password;
    }

    void InfluxDB::InfluxDBInitV2(std::string _influxDBURI, std::string _bucket, std::string _org, std::string _token) {
        version = INFLUXDB_V2;
        influxDBURI = _influxDBURI;
        bucket = _bucket;
        org = _org;
        token = _token;
    }

    void InfluxDB::connectHTTP() {
        esp_http_client_config_t config = {};

        config.url = influxDBURI.c_str();
        config.event_handler = http_event_handler;
        config.buffer_size = MAX_HTTP_OUTPUT_BUFFER;
        config.user_data = response_buffer;


        switch (version) {
            case INFLUXDB_V1:
                config.auth_type = HTTP_AUTH_TYPE_BASIC;
                config.username = user.c_str();
                config.password = password.c_str();
                break;
            case INFLUXDB_V2:
                break;
        }

        InfluxDBdestroy();
        httpClient = esp_http_client_init(&config);
        if (!httpClient) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to initialize HTTP client");
        } else {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP client initialized successfully");
        }
    }


    // Destroy the InfluxDB connection
    void InfluxDB::InfluxDBdestroy() {
        if (httpClient) {
            esp_http_client_cleanup(httpClient);
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP client cleaned up");
            httpClient = NULL;
        }
    }

    // Publish data to the InfluxDB server
    void InfluxDB::InfluxDBPublish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC) {
        std::string apiURI;        
        std::string payload;
        char nowTimestamp[21];

        connectHTTP();


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

        esp_err_t err;

        switch (version) {
            case INFLUXDB_V1: 
                apiURI = influxDBURI + "/write?db=" + database;
                apiURI.shrink_to_fit();

                esp_http_client_set_url(httpClient, apiURI.c_str());
                esp_http_client_set_method(httpClient, HTTP_METHOD_POST);
                esp_http_client_set_header(httpClient, "Content-Type", "text/plain");
                esp_http_client_set_post_field(httpClient, payload.c_str(), payload.length());

                err = esp_http_client_perform(httpClient);
                if (err == ESP_OK) {
                    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Data published successfully: " + payload);
                } else {
                    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to publish data: " + std::string(esp_err_to_name(err)));
                }
                break;

            case INFLUXDB_V2:        
                apiURI = influxDBURI + "/api/v2/write?org=" + org + "&bucket=" + bucket;
                apiURI.shrink_to_fit();
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "apiURI: " + apiURI);

                esp_http_client_set_url(httpClient, apiURI.c_str());
                esp_http_client_set_method(httpClient, HTTP_METHOD_POST);
                esp_http_client_set_header(httpClient, "Content-Type", "text/plain");
                std::string _zw = "Token " + token;
                esp_http_client_set_header(httpClient, "Authorization", _zw.c_str());
                esp_http_client_set_post_field(httpClient, payload.c_str(), payload.length());
                err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_perform(httpClient));
                if (err == ESP_OK) {
                    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Data published successfully: " + payload);
                } else {
                    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Failed to publish data: " + std::string(esp_err_to_name(err)));
                }
            break;
        }
    }

#endif //ENABLE_INFLUXDB
