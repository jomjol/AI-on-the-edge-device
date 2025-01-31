#ifdef ENABLE_INFLUXDB
#include "interface_influxdb.h"

#include "esp_log.h"
#include <time.h>
#include "ClassLogFile.h"
#include "esp_http_client.h"
#include "time_sntp.h"
#include "../../include/defines.h"

static const char *TAG = "INFLUXDB";

/**
 * @brief Buffer to store the HTTP response.
 * 
 * This character array is used to store the output of an HTTP response.
 * The size of the buffer is defined by the constant MAX_HTTP_OUTPUT_BUFFER.
 */
char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};


/**
 * @brief HTTP event handler callback function.
 *
 * This function handles various HTTP client events and logs appropriate messages.
 *
 * @param evt Pointer to the HTTP client event structure.
 * @return esp_err_t ESP_OK on success.
 *
 * Event types handled:
 * - HTTP_EVENT_ERROR: Logs an error message when an HTTP error is encountered.
 * - HTTP_EVENT_ON_CONNECTED: Logs a message when the HTTP client successfully connects.
 * - HTTP_EVENT_HEADERS_SENT: Logs a message when all request headers are sent.
 * - HTTP_EVENT_ON_HEADER: Logs the received header key and value.
 * - HTTP_EVENT_ON_DATA: Logs the length of the received data.
 * - HTTP_EVENT_ON_FINISH: Logs a message when the HTTP client finishes the request.
 * - HTTP_EVENT_DISCONNECTED: Logs a message when the HTTP client disconnects.
 * - HTTP_EVENT_REDIRECT: Logs a message when an HTTP redirect occurs.
 */
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



/**
 * @brief Initializes the InfluxDB connection with version 1 settings.
 * 
 * This function sets up the connection parameters for InfluxDB version 1.
 * 
 * @param _influxDBURI The URI of the InfluxDB server.
 * @param _database The name of the database to connect to.
 * @param _user The username for authentication.
 * @param _password The password for authentication.
 */
void InfluxDB::InfluxDBInitV1(std::string _influxDBURI, std::string _database, std::string _user, std::string _password) {
    version = INFLUXDB_V1;
    influxDBURI = _influxDBURI;
    database = _database;
    user = _user;
    password = _password;
}

/**
 * @brief Initializes the InfluxDB client with version 2 settings.
 * 
 * This function sets up the InfluxDB client to use InfluxDB version 2 by
 * configuring the URI, bucket, organization, and token.
 * 
 * @param _influxDBURI The URI of the InfluxDB server.
 * @param _bucket The bucket name to store data in.
 * @param _org The organization name associated with the bucket.
 * @param _token The authentication token for accessing the InfluxDB server.
 */
void InfluxDB::InfluxDBInitV2(std::string _influxDBURI, std::string _bucket, std::string _org, std::string _token) {
    version = INFLUXDB_V2;
    influxDBURI = _influxDBURI;
    bucket = _bucket;
    org = _org;
    token = _token;
}

/**
 * @brief Establishes an HTTP connection to the InfluxDB server.
 *
 * This function configures and initializes an HTTP client to connect to the InfluxDB server.
 * It sets up the necessary parameters such as the URL, event handler, buffer size, and user data.
 * Depending on the InfluxDB version, it also configures the authentication type and credentials.
 *
 * @note This function destroys any existing HTTP client before initializing a new one.
 *
 * @param None
 * @return None
 */
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


/**
 * @brief Destroys the InfluxDB instance by cleaning up the HTTP client.
 *
 * This function checks if the HTTP client is initialized. If it is, it cleans up the HTTP client
 * and logs the cleanup action. The HTTP client pointer is then set to NULL.
 */
void InfluxDB::InfluxDBdestroy() {
    if (httpClient) {
        esp_http_client_cleanup(httpClient);
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "HTTP client cleaned up");
        httpClient = NULL;
    }
}

/**
 * @brief Publishes data to an InfluxDB instance.
 *
 * This function sends a measurement, key, and content to an InfluxDB server.
 * It supports both InfluxDB v1 and v2 APIs.
 *
 * @param _measurement The measurement name to publish.
 * @param _key The key associated with the measurement.
 * @param _content The content or value to publish.
 * @param _timeUTC The timestamp in UTC. If greater than 0, it will be included in the payload.
 *
 * The function logs the process and handles HTTP communication with the InfluxDB server.
 * It constructs the appropriate API URI based on the InfluxDB version and sends the data
 * using an HTTP POST request.
 */
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
