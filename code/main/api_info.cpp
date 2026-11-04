#include "api_info.h"

#include "api_response.h"
#include "api_router.h"

esp_err_t api_info_get_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "api_version", API_VERSION);

    return api_send_json(req, root);
}
