#include "esp_log.h"
#include "esp_http_server.h"

#include "../../include/defines.h"

#include "ClassLogFile.h"
#include "freertos/ringbuf.h"

#include "websocket.h"


#define MAX_MESSAGE_LENGTH  100

static const char *TAG = "WEBSOCKET";


static httpd_handle_t server = NULL;
static httpd_handle_t my_hd = NULL;

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
    char message[MAX_MESSAGE_LENGTH];
};


/*
 * async send function, which we put into the httpd work queue
 */
static void websocket_send_pending_message(void *arg) {
    
    esp_err_t ret;
    struct async_resp_arg *resp_arg = (struct async_resp_arg *)arg;

    ESP_LOGI(TAG, "websocket_send_pending_message: '%s'", resp_arg->message);
    
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)resp_arg->message;
    ws_pkt.len = strlen(resp_arg->message);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    ret = httpd_get_client_list(server, &fds, client_fds);

    if (ret != ESP_OK) {
        return;
    }

    /* Send it to all websocket clients */
    for (int i = 0; i < fds; i++) {
        int client_info = httpd_ws_get_fd_info(server, client_fds[i]);
        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET) {
            httpd_ws_send_frame_async(my_hd, client_fds[i], &ws_pkt);
        }
    }

    free(resp_arg);
}


esp_err_t schedule_websocket_message(std::string message) {
    esp_err_t ret;

    ESP_LOGI(TAG, "schedule_websocket_message: '%s'", message.c_str());

    struct async_resp_arg *resp_arg = (struct async_resp_arg *)malloc(sizeof(struct async_resp_arg));
   
    strncpy(resp_arg->message, message.c_str(), MAX_MESSAGE_LENGTH);

    ret = httpd_queue_work(my_hd, websocket_send_pending_message, resp_arg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_queue_work failed: %d", ret);
    }

    return ret;
}


static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");

         my_hd = req->handle;

        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len)
    {
        buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }

    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt.payload, "toggle") == 0) {
        free(buf);
    }

    return ESP_OK;
}


static const httpd_uri_t ws_uri = {
    .uri        = "/ws",
    .method     = HTTP_GET,
    .handler    = ws_handler,
    .user_ctx   = NULL,
    .is_websocket = true
};


esp_err_t start_websocket_server(httpd_handle_t _server) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Init Websocket Server");

    server = _server;

    // Registering the ws handler
    ESP_LOGI(TAG, "Registering URI handlers");
    return httpd_register_uri_handler(server, &ws_uri);
}
