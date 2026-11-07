#include "api_response.h"

namespace
{

// Prints body with the given status line. Doesn't delete body.
esp_err_t send_body(httpd_req_t *req, const char *status, const cJSON *body)
{
    char *text = cJSON_PrintUnformatted(body);
    if (text == NULL)
    {
        // Memory ran out, so a JSON error body may not fit either
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
    }

    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, "application/json");
    esp_err_t result = httpd_resp_sendstr(req, text);
    cJSON_free(text);
    return result;
}

} // namespace

esp_err_t api_send_json(httpd_req_t *req, cJSON *body)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"); // Reads only, same as the old endpoints
    esp_err_t result = send_body(req, HTTPD_200, body);
    cJSON_Delete(body);
    return result;
}

esp_err_t api_send_error(httpd_req_t *req, const char *status, const char *code, const char *message)
{
    cJSON *body = cJSON_CreateObject();
    cJSON *error = cJSON_AddObjectToObject(body, "error");
    cJSON_AddStringToObject(error, "code", code);
    cJSON_AddStringToObject(error, "message", message);

    esp_err_t result = send_body(req, status, body);
    cJSON_Delete(body);
    return result;
}
