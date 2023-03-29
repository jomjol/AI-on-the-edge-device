#include "esp_log.h"
#include "esp_http_server.h"

#include "../../include/defines.h"

#include "ClassLogFile.h"
#include "freertos/ringbuf.h"
#include "psram.h"

#include "websocket.h"


#define MAX_MESSAGE_LENGTH  100

static const char *TAG = "WEBSOCKET";

static httpd_handle_t server = NULL;
static httpd_handle_t websocket_handle = NULL;

/*
 * Structure holding server handle and message
 * in order to use out of request send */
struct async_resp_arg {
    httpd_handle_t hd;
    char message[MAX_MESSAGE_LENGTH];
};


/*
 * async send function, which we put into the httpd work queue
 */
static void websocket_send_pending_message(void *arg) {
    esp_err_t ret;
    struct async_resp_arg *resp_arg = (struct async_resp_arg *)arg;

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Sending Websocket message: '" + std::string(resp_arg->message) + "'");
    
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
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to get Websocket client ist: " + std::to_string(ret) + "!");
        free_psram_heap("websocket msg", resp_arg);
        return;
    }

    /* Send it to all websocket clients */
    for (int i = 0; i < fds; i++) {
        int client_info = httpd_ws_get_fd_info(server, client_fds[i]);
        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET) {
            httpd_ws_send_frame_async(websocket_handle, client_fds[i], &ws_pkt);
        }
    }

    free_psram_heap("websocket msg", resp_arg);
}


esp_err_t schedule_websocket_message(std::string message) {
   // return 0;
    esp_err_t ret;

    if (websocket_handle == NULL) { // No websocket connecten open
        return ESP_OK;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Scheduled websocket message: '" + message + "'");

    struct async_resp_arg *resp_arg = (struct async_resp_arg *)malloc_psram_heap("websocket msg", 
            sizeof(struct async_resp_arg), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (resp_arg == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to malloc memory for scheduled websocket message!");
        return ESP_ERR_NO_MEM;
    }
   
    strncpy(resp_arg->message, message.c_str(), MAX_MESSAGE_LENGTH);

    ret = httpd_queue_work(websocket_handle, websocket_send_pending_message, resp_arg);
    if (ret != ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Websocket Scheduling failed: " + std::to_string(ret) + "!");
        free_psram_heap("websocket msg", resp_arg);
    }

    return ret;
}


static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Handshake done, the new websocket connection was opened");
        websocket_handle = req->handle;
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
    esp_err_t ret;

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init Websocket Server");

    server = _server;

    // Registering the ws handler
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Registering URI handler");
    ret = httpd_register_uri_handler(server, &ws_uri);
    if (ret != ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Registering Websocket URI handler failed: " + std::to_string(ret));
    }
    return ret;
}
