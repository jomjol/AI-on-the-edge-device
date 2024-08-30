#ifdef ENABLE_WEBHOOK
#include "interface_webhook.h"

#include "esp_log.h"
#include <time.h>
#include "ClassLogFile.h"
#include "esp_http_client.h"
#include "time_sntp.h"
#include "../../include/defines.h"
#include <cJSON.h>
#include <ClassFlowDefineTypes.h>


static const char *TAG = "WEBHOOK";

std::string _webhookURI;
std::string _webhookApiKey;
long _lastTimestamp;

static esp_err_t http_event_handler(esp_http_client_event_t *evt);

void WebhookInit(std::string _uri, std::string _apiKey)
{
    _webhookURI = _uri;
    _webhookApiKey = _apiKey;
    _lastTimestamp = 0L;
}

bool WebhookPublish(std::vector<NumberPost*>* numbers)
{
    bool numbersWithError = false;
    cJSON *jsonArray = cJSON_CreateArray();

    for (int i = 0; i < (*numbers).size(); ++i)
    {
        string timezw = "";
        char buffer[80];
        time_t &lastPreValue = (*numbers)[i]->timeStampLastPreValue;
        struct tm* timeinfo = localtime(&lastPreValue);
        _lastTimestamp = static_cast<long>(lastPreValue);
        strftime(buffer, 80, PREVALUE_TIME_FORMAT_OUTPUT, timeinfo);
        timezw = std::string(buffer);

        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "timestamp", timezw.c_str());
        cJSON_AddStringToObject(json, "timestampLong", std::to_string(_lastTimestamp).c_str());
        cJSON_AddStringToObject(json, "name", (*numbers)[i]->name.c_str());
        cJSON_AddStringToObject(json, "rawValue", (*numbers)[i]->ReturnRawValue.c_str());
        cJSON_AddStringToObject(json, "value", (*numbers)[i]->ReturnValue.c_str());
        cJSON_AddStringToObject(json, "preValue", (*numbers)[i]->ReturnPreValue.c_str());
        cJSON_AddStringToObject(json, "rate", (*numbers)[i]->ReturnRateValue.c_str());
        cJSON_AddStringToObject(json, "changeAbsolute", (*numbers)[i]->ReturnChangeAbsolute.c_str());
        cJSON_AddStringToObject(json, "error", (*numbers)[i]->ErrorMessageText.c_str());
        
        cJSON_AddItemToArray(jsonArray, json);

        if ((*numbers)[i]->ErrorMessage) {
            numbersWithError = true;
        }
    }

    char *jsonString = cJSON_PrintUnformatted(jsonArray);

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "sending webhook");
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "sending JSON: " + std::string(jsonString));

    char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t http_config = {
        .url = _webhookURI.c_str(),
        .user_agent = "ESP32 Meter reader",
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .buffer_size = MAX_HTTP_OUTPUT_BUFFER,
        .user_data = response_buffer
    };

    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);

    esp_http_client_set_header(http_client, "Content-Type", "application/json");
    esp_http_client_set_header(http_client, "APIKEY", _webhookApiKey.c_str());

    ESP_ERROR_CHECK(esp_http_client_set_post_field(http_client, jsonString, strlen(jsonString)));

    esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_perform(http_client));

    if(err == ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP request was performed");
        int status_code = esp_http_client_get_status_code(http_client);
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP status code: " + std::to_string(status_code));
    } else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "HTTP request failed");
    } 

    esp_http_client_cleanup(http_client);
    cJSON_Delete(jsonArray);
    free(jsonString);
    return numbersWithError;    
}

void WebhookUploadPic(ImageData *Img) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Starting WebhookUploadPic");

    std::string fullURI = _webhookURI + "?timestamp=" + std::to_string(_lastTimestamp);
    char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t http_config = {
        .url = fullURI.c_str(),
        .user_agent = "ESP32 Meter reader",
        .method = HTTP_METHOD_PUT,
        .event_handler = http_event_handler,
        .buffer_size = MAX_HTTP_OUTPUT_BUFFER,
        .user_data = response_buffer
    };

    esp_http_client_handle_t http_client = esp_http_client_init(&http_config);

    esp_http_client_set_header(http_client, "Content-Type", "image/jpeg");
    esp_http_client_set_header(http_client, "APIKEY", _webhookApiKey.c_str());

    esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_set_post_field(http_client, (const char *)Img->data, Img->size));

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_http_client_perform(http_client));

    if (err == ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP PUT request was performed successfully");
        int status_code = esp_http_client_get_status_code(http_client);
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "HTTP status code: " + std::to_string(status_code));
    } else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "HTTP PUT request failed");
    }

    esp_http_client_cleanup(http_client);

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "WebhookUploadPic finished");
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

#endif //ENABLE_WEBHOOK
