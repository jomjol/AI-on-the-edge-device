#include "server_camera.h"

#include <string>
#include "string.h"

#include "esp_camera.h"
#include "ClassControllCamera.h"
#include "MainFlowControl.h"

#include "ClassLogFile.h"
#include "esp_log.h"

#include "basic_auth.h"

#include "../../include/defines.h"

static const char *TAG = "server_cam";

void PowerResetCamera()
{
#if CAM_PIN_PWDN == GPIO_NUM_NC // Use reset only if pin is available
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "No power down pin availbale to reset camera");
#else
    ESP_LOGD(TAG, "Resetting camera by power down line");
    gpio_config_t conf;
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.pin_bit_mask = 1LL << CAM_PIN_PWDN;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&conf);

    // carefull, logic is inverted compared to reset pin
    gpio_set_level(CAM_PIN_PWDN, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(CAM_PIN_PWDN, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif
}

esp_err_t handler_lightOn(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_lightOn - Start");
    ESP_LOGD(TAG, "handler_lightOn uri: %s", req->uri);
#endif

    if (Camera.getCameraInitSuccessful())
    {
        Camera.LightOnOff(true);
        const char *resp_str = (const char *)req->user_ctx;
        httpd_resp_send(req, resp_str, strlen(resp_str));
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /lighton not available!");
        return ESP_ERR_NOT_FOUND;
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_lightOn - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_lightOff(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_lightOff - Start");
    ESP_LOGD(TAG, "handler_lightOff uri: %s", req->uri);
#endif

    if (Camera.getCameraInitSuccessful())
    {
        Camera.LightOnOff(false);
        const char *resp_str = (const char *)req->user_ctx;
        httpd_resp_send(req, resp_str, strlen(resp_str));
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /lightoff not available!");
        return ESP_ERR_NOT_FOUND;
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_lightOff - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_capture(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_capture - Start");
#endif

    if (Camera.getCameraInitSuccessful())
    {
        // If the camera settings were changed by creating a new reference image, they must be reset
        if (CFstatus.changedCameraSettings)
        {
            Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
            Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize, CCstatus.ImageVflip);
            Camera.LedIntensity = CCstatus.ImageLedIntensity;
            CFstatus.changedCameraSettings = false;
        }

#ifdef DEBUG_DETAIL_ON
        ESP_LOGD(TAG, "Size: %d, Quality: %d", CCstatus.ImageFrameSize, CCstatus.ImageQuality);
#endif        

        esp_err_t result;
        result = Camera.CaptureToHTTP(req);

#ifdef DEBUG_DETAIL_ON
        LogFile.WriteHeapInfo("handler_capture - Done");
#endif

        return result;
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /capture not available!");
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t handler_capture_with_light(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_capture_with_light - Start");
#endif

    if (Camera.getCameraInitSuccessful())
    {
        char _query[100];
        char _delay[10];
        int delay = 2500;

        if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "Query: %s", _query);

            if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
            {
#ifdef DEBUG_DETAIL_ON
                ESP_LOGD(TAG, "Delay: %s", _delay);
#endif
                delay = atoi(_delay);

                if (delay < 0)
                {
                    delay = 0;
                }
            }
        }

        // If the camera settings were changed by creating a new reference image, they must be reset
        if (CFstatus.changedCameraSettings)
        {
            Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
            Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize, CCstatus.ImageVflip);
            Camera.LedIntensity = CCstatus.ImageLedIntensity;
            CFstatus.changedCameraSettings = false;
        }

#ifdef DEBUG_DETAIL_ON
        ESP_LOGD(TAG, "Size: %d, Quality: %d", CCstatus.ImageFrameSize, CCstatus.ImageQuality);
#endif        

        Camera.LightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);

        esp_err_t result;
        result = Camera.CaptureToHTTP(req);

        Camera.LightOnOff(false);

#ifdef DEBUG_DETAIL_ON
        LogFile.WriteHeapInfo("handler_capture_with_light - Done");
#endif

        return result;
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /capture_with_flashlight not available!");
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t handler_capture_save_to_file(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_capture_save_to_file - Start");
#endif

    if (Camera.getCameraInitSuccessful())
    {
        char _query[100];
        char _delay[10];
        int delay = 0;
        char filename[100];
        std::string fn = "/sdcard/";

        if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "Query: %s", _query);

            if (httpd_query_key_value(_query, "filename", filename, 100) == ESP_OK)
            {
                fn.append(filename);
#ifdef DEBUG_DETAIL_ON
                ESP_LOGD(TAG, "Filename: %s", fn.c_str());
#endif
            }
            else
            {
                fn.append("noname.jpg");
            }

            if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
            {
#ifdef DEBUG_DETAIL_ON
                ESP_LOGD(TAG, "Delay: %s", _delay);
#endif
                delay = atoi(_delay);

                if (delay < 0)
                {
                    delay = 0;
                }
            }
        }
        else
        {
            fn.append("noname.jpg");
        }

        // If the camera settings were changed by creating a new reference image, they must be reset
        if (CFstatus.changedCameraSettings)
        {
            Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
            Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize, CCstatus.ImageVflip);
            Camera.LedIntensity = CCstatus.ImageLedIntensity;
            CFstatus.changedCameraSettings = false;
        }

#ifdef DEBUG_DETAIL_ON
        ESP_LOGD(TAG, "Size: %d, Quality: %d", CCstatus.ImageFrameSize, CCstatus.ImageQuality);
#endif        

        esp_err_t result;
        result = Camera.CaptureToFile(fn, delay);

        const char *resp_str = (const char *)fn.c_str();
        httpd_resp_send(req, resp_str, strlen(resp_str));

#ifdef DEBUG_DETAIL_ON
        LogFile.WriteHeapInfo("handler_capture_save_to_file - Done");
#endif

        return result;
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /save not available!");
        return ESP_ERR_NOT_FOUND;
    }
}

void register_server_camera_uri(httpd_handle_t server)
{
#ifdef DEBUG_DETAIL_ON
    ESP_LOGI(TAG, "server_part_camera - Registering URI handlers");
#endif

    httpd_uri_t camuri = {};
    camuri.method = HTTP_GET;

    camuri.uri = "/lighton";
    camuri.handler = APPLY_BASIC_AUTH_FILTER(handler_lightOn);
    camuri.user_ctx = (void *)"Light On";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/lightoff";
    camuri.handler = APPLY_BASIC_AUTH_FILTER(handler_lightOff);
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/capture";
    camuri.handler = APPLY_BASIC_AUTH_FILTER(handler_capture);
    camuri.user_ctx = NULL;
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/capture_with_flashlight";
    camuri.handler = APPLY_BASIC_AUTH_FILTER(handler_capture_with_light);
    camuri.user_ctx = NULL;
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/save";
    camuri.handler = APPLY_BASIC_AUTH_FILTER(handler_capture_save_to_file);
    camuri.user_ctx = NULL;
    httpd_register_uri_handler(server, &camuri);
}
