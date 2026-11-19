#include "server_camera.h"

#include <string>
#include "string.h"

#include "esp_camera.h"
#include "ClassControlCamera.h"
#include "MainFlowControl.h"

#include "ClassLogFile.h"
#include "esp_log.h"

#include "basic_auth.h"

#include "defines.h"

static const char *TAG = "server_cam";

esp_err_t handler_lightOn(httpd_req_t *req)
{
    if (Camera.get_camera_init_successful())
    {
        Camera.set_flash_light(true);
        const char *resp_str = (const char *)req->user_ctx;
        httpd_resp_send(req, resp_str, strlen(resp_str));
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /lighton not available!");
        return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;
}

esp_err_t handler_lightOff(httpd_req_t *req)
{
    if (Camera.get_camera_init_successful())
    {
        Camera.set_flash_light(false);
        const char *resp_str = (const char *)req->user_ctx;
        httpd_resp_send(req, resp_str, strlen(resp_str));
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /lightoff not available!");
        return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;
}

esp_err_t handler_capture(httpd_req_t *req)
{
    if (Camera.get_camera_init_successful())
    {
        return Camera.capture_to_http(req, 0);
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /capture not available!");
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t handler_capture_with_light(httpd_req_t *req)
{
    if (Camera.get_camera_init_successful())
    {
        char _query[100];
        char _delay[10];

        int delay = 2500;

        if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
        {
            if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
            {
                delay = atoi(_delay);
                if (delay < 0)
                {
                    delay = 0;
                }
            }
        }

        return Camera.capture_to_http(req, delay);
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Camera not initialized: REST API /capture_with_flashlight not available!");
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t handler_capture_save_to_file(httpd_req_t *req)
{
    if (Camera.get_camera_init_successful())
    {
        char _query[100];
        char _delay[10];
        char filename[100];

        int delay = 0;
        std::string fn = "/sdcard/";

        if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
        {
            if (httpd_query_key_value(_query, "filename", filename, 100) == ESP_OK)
            {
                fn.append(filename);
            }
            else
            {
                fn.append("noname.jpg");
            }

            if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
            {
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

        esp_err_t result = Camera.capture_to_file(fn, delay);

        const char *resp_str = (const char *)fn.c_str();
        httpd_resp_send(req, resp_str, strlen(resp_str));

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
