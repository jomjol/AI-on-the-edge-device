#include "server_main.h"

#include <string>

#include "server_help.h"
#include "ClassLogFile.h"

#include "time_sntp.h"

#include "connect_wlan.h"
#include "read_wlanini.h"

#include "version.h"

#include "esp_wifi.h"
#include <netdb.h>

#include "MainFlowControl.h"
#include "esp_log.h"

#include <stdio.h>

#include "Helper.h"

httpd_handle_t server = NULL;   
std::string starttime = "";

static const char *TAG = "MAIN SERVER";

/* An HTTP GET handler */
esp_err_t info_get_handler(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("info_get_handler - Start");    
#endif

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "info_get_handler");    
    char _query[200];
    char _valuechar[30];    
    std::string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) != ESP_OK)
    {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid query string");
    }

    ESP_LOGD(TAG, "Query: %s", _query);

    if (httpd_query_key_value(_query, "type", _valuechar, 30) != ESP_OK)
    {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "missing or invalid 'type' query parameter (too long value?)");
    }

    ESP_LOGD(TAG, "type is found: %s", _valuechar);
    _task = std::string(_valuechar);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (_task.compare("GitBranch") == 0)
    {
        httpd_resp_sendstr(req, libfive_git_branch());
        return ESP_OK;        
    }
    else if (_task.compare("GitTag") == 0)
    {
        httpd_resp_sendstr(req, libfive_git_version());
        return ESP_OK;        
    }
    else if (_task.compare("GitRevision") == 0)
    {
        httpd_resp_sendstr(req, libfive_git_revision());
        return ESP_OK;        
    }
    else if (_task.compare("BuildTime") == 0)
    {
        httpd_resp_sendstr(req, build_time());
        return ESP_OK;        
    }
    else if (_task.compare("FirmwareVersion") == 0)
    {
        httpd_resp_sendstr(req, getFwVersion().c_str());
        return ESP_OK;        
    }
    else if (_task.compare("HTMLVersion") == 0)
    {
        httpd_resp_sendstr(req, getHTMLversion().c_str());
        return ESP_OK;        
    }
    else if (_task.compare("Hostname") == 0)
    {
        std::string zw;
        zw = std::string(wlan_config.hostname);
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("IP") == 0)
    {
        std::string *zw;
        zw = getIPAddress();
        httpd_resp_sendstr(req, zw->c_str());
        return ESP_OK;        
    }
    else if (_task.compare("SSID") == 0)
    {
        std::string *zw;
        zw = getSSID();
        httpd_resp_sendstr(req, zw->c_str());
        return ESP_OK;        
    }
    else if (_task.compare("FlowStatus") == 0)
    {
        std::string zw;
        zw = std::string("FlowStatus");
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("Round") == 0)
    {
        char formated[10] = "";    
        snprintf(formated, sizeof(formated), "%d", getCountFlowRounds());
        httpd_resp_sendstr(req, formated);
        return ESP_OK;        
    }
    else if (_task.compare("SDCardPartitionSize") == 0)
    {
        std::string zw;
        zw = getSDCardPartitionSize();
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("SDCardFreePartitionSpace") == 0)
    {
        std::string zw;
        zw = getSDCardFreePartitionSpace();
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("SDCardPartitionAllocationSize") == 0)
    {
        std::string zw;
        zw = getSDCardPartitionAllocationSize();
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("SDCardManufacturer") == 0)
    {
        std::string zw;
        zw = getSDCardManufacturer(); 
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("SDCardName") == 0)
    {
        std::string zw;
        zw = getSDCardName(); 
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("SDCardCapacity") == 0)
    {
        std::string zw;
        zw = getSDCardCapacity();
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else if (_task.compare("SDCardSectorSize") == 0)
    {
        std::string zw;
        zw = getSDCardSectorSize();
        httpd_resp_sendstr(req, zw.c_str());
        return ESP_OK;        
    }
    else
    {
        char formatted[256];
        snprintf(formatted, sizeof(formatted), "Unknown value for parameter info 'type': '%s'\n", _task.c_str());
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, formatted);
    }

    return ESP_OK;
}


esp_err_t starttime_get_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    httpd_resp_send(req, starttime.c_str(), starttime.length()); 

    return ESP_OK;
}


esp_err_t hello_main_handler(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("hello_main_handler - Start");
#endif

    char filepath[50];
    ESP_LOGD(TAG, "uri: %s\n", req->uri);
    int _pos;
    esp_err_t res;

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = get_path_from_uri(filepath, base_path,
                                             req->uri - 1, sizeof(filepath));    
    ESP_LOGD(TAG, "1 uri: %s, filename: %s, filepath: %s", req->uri, filename, filepath);

    if ((strcmp(req->uri, "/") == 0))
    {
        {
            filetosend = filetosend + "/html/index.html";
        }
    }
    else
    {
        filetosend = filetosend + "/html" + std::string(req->uri);
        _pos = filetosend.find("?");
        if (_pos > -1){
            filetosend = filetosend.substr(0, _pos);
        }
    }

    if (filetosend == "/sdcard/html/index.html") {
        if (isSetSystemStatusFlag(SYSTEM_STATUS_PSRAM_BAD) || // Initialization failed with crritical errors!
            isSetSystemStatusFlag(SYSTEM_STATUS_CAM_BAD) ||
            isSetSystemStatusFlag(SYSTEM_STATUS_SDCARD_CHECK_BAD) ||
            isSetSystemStatusFlag(SYSTEM_STATUS_FOLDER_CHECK_BAD)) 
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "We have a critical error, not serving main page!");

            char buf[20];
            std::string message = "<h1>AI on the Edge Device</h1><b>We have one or more critical errors:</b><br>";

            for (int i = 0; i < 32; i++) {
                if (isSetSystemStatusFlag((SystemStatusFlag_t)(1<<i))) {
                    snprintf(buf, sizeof(buf), "0x%08X", 1<<i);
                    message += std::string(buf) + "<br>";
                }
            }

            message += "<br>Please check logs with log viewer and/or <a href=\"https://jomjol.github.io/AI-on-the-edge-device-docs/Error-Codes\" target=_blank>jomjol.github.io/AI-on-the-edge-device-docs/Error-Codes</a> for more information!";
            message += "<br><br><button onclick=\"window.location.href='/reboot';\">Reboot</button>";
            message += "&nbsp;<button onclick=\"window.open('/ota_page.html');\">OTA Update</button>";
            message += "&nbsp;<button onclick=\"window.open('/log.html');\">Log Viewer</button>";
            message += "&nbsp;<button onclick=\"window.open('/info.html');\">Show System Info</button>";
            httpd_resp_send(req, message.c_str(), message.length());
            return ESP_OK;
        }
        else if (isSetupModusActive()) {
            ESP_LOGD(TAG, "System is in setup mode --> index.html --> setup.html");
            filetosend = "/sdcard/html/setup.html";
        }
    }

    ESP_LOGD(TAG, "Filename: %s", filename);
    
    ESP_LOGD(TAG, "File requested: %s", filetosend.c_str());

    if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 414 Error */
        httpd_resp_send_err(req, HTTPD_414_URI_TOO_LONG, "Filename too long");
        return ESP_FAIL;
    }

    res = send_file(req, filetosend);
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);

    if (res != ESP_OK)
        return res;

    /* Respond with an empty chunk to signal HTTP response completion */
//    httpd_resp_sendstr(req, "");
//    httpd_resp_send_chunk(req, NULL, 0);

#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("hello_main_handler - Stop");   
#endif

    return ESP_OK;
}


esp_err_t img_tmp_handler(httpd_req_t *req)
{
    char filepath[50];
    ESP_LOGD(TAG, "uri: %s", req->uri);

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = get_path_from_uri(filepath, base_path,
                                             req->uri  + sizeof("/img_tmp/") - 1, sizeof(filepath));    
    ESP_LOGD(TAG, "1 uri: %s, filename: %s, filepath: %s", req->uri, filename, filepath);

    filetosend = filetosend + "/img_tmp/" + std::string(filename);
    ESP_LOGD(TAG, "File to upload: %s", filetosend.c_str());

    esp_err_t res = send_file(req, filetosend); 
    if (res != ESP_OK)
        return res;

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


esp_err_t img_tmp_virtual_handler(httpd_req_t *req)
{
    #ifdef DEBUG_DETAIL_ON      
        LogFile.WriteHeapInfo("img_tmp_virtual_handler - Start");  
    #endif

    char filepath[50];

    ESP_LOGD(TAG, "uri: %s", req->uri);

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = get_path_from_uri(filepath, base_path,
                                             req->uri  + sizeof("/img_tmp/") - 1, sizeof(filepath));    
    ESP_LOGD(TAG, "1 uri: %s, filename: %s, filepath: %s", req->uri, filename, filepath);

    filetosend = std::string(filename);
    ESP_LOGD(TAG, "File to upload: %s", filetosend.c_str());

    // Serve raw.jpg
    if (filetosend == "raw.jpg")
        return GetRawJPG(req); 

    // Serve alg.jpg, alg_roi.jpg or digital and analog ROIs
    if (ESP_OK == GetJPG(filetosend, req))
        return ESP_OK;

    #ifdef DEBUG_DETAIL_ON      
        LogFile.WriteHeapInfo("img_tmp_virtual_handler - Done");   
    #endif

    // File was not served already --> serve with img_tmp_handler
    return img_tmp_handler(req);
}


esp_err_t sysinfo_handler(httpd_req_t *req)
{
    std::string zw;
    std::string cputemp = std::to_string((int)temperatureRead());
    std::string gitversion = libfive_git_version();
    std::string buildtime = build_time();
    std::string gitbranch = libfive_git_branch();
    std::string gittag = libfive_git_version();
    std::string gitrevision = libfive_git_revision();
    std::string htmlversion = getHTMLversion();
    char freeheapmem[11];
    sprintf(freeheapmem, "%lu", (long) getESPHeapSize());
    
    zw = string("[{") + 
        "\"firmware\": \"" + gitversion + "\"," +
        "\"buildtime\": \"" + buildtime + "\"," +
        "\"gitbranch\": \"" + gitbranch + "\"," +
        "\"gittag\": \"" + gittag + "\"," +
        "\"gitrevision\": \"" + gitrevision + "\"," +
        "\"html\": \"" + htmlversion + "\"," +
        "\"cputemp\": \"" + cputemp + "\"," +
        "\"hostname\": \"" + *getHostname() + "\"," +
        "\"IPv4\": \"" + *getIPAddress() + "\"," +
        "\"freeHeapMem\": \"" + freeheapmem + "\"" +
        "}]";

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, zw.c_str(), zw.length());

    return ESP_OK;
}


void register_server_main_uri(httpd_handle_t server, const char *base_path)
{
    httpd_uri_t info_get_handle = {
        .uri       = "/info",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = info_get_handler,
        .user_ctx  = (void*) base_path    // Pass server data as context
    };
    httpd_register_uri_handler(server, &info_get_handle);

    httpd_uri_t sysinfo_handle = {
        .uri       = "/sysinfo",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = sysinfo_handler,
        .user_ctx  = (void*) base_path    // Pass server data as context
    };
    httpd_register_uri_handler(server, &sysinfo_handle);

    httpd_uri_t starttime_tmp_handle = {
        .uri       = "/starttime",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = starttime_get_handler,
        .user_ctx  = NULL    // Pass server data as context
    };
    httpd_register_uri_handler(server, &starttime_tmp_handle);

    httpd_uri_t img_tmp_handle = {
        .uri       = "/img_tmp/*",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = img_tmp_virtual_handler,
        .user_ctx  = (void*) base_path    // Pass server data as context
    };
    httpd_register_uri_handler(server, &img_tmp_handle);

    httpd_uri_t main_rest_handle = {
        .uri       = "/*",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = hello_main_handler,
        .user_ctx  = (void*) base_path    // Pass server data as context
    };
    httpd_register_uri_handler(server, &main_rest_handle);

}


httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = { };

    config.task_priority = tskIDLE_PRIORITY+3; // previously -> 2022-12-11: tskIDLE_PRIORITY+1; 2021-09-24: tskIDLE_PRIORITY+5
    config.stack_size = 12288; // previously -> 2023-01-02: 32768
    config.core_id = 1; // previously -> 2023-01-02: 0, 2022-12-11: tskNO_AFFINITY;
    config.server_port = 80;
    config.ctrl_port = 32768;
    config.max_open_sockets = 5; //20210921 --> previously 7   
    config.max_uri_handlers = 39; // previously 24, 20220511: 35, 20221220: 37, 2023-01-02:38             
    config.max_resp_headers = 8;                        
    config.backlog_conn = 5;                        
    config.lru_purge_enable = true; // this cuts old connections if new ones are needed.               
    config.recv_wait_timeout = 5; // default: 5 20210924 --> previously 30              
    config.send_wait_timeout = 5; // default: 5 20210924 --> previously 30                    
    config.global_user_ctx = NULL;                        
    config.global_user_ctx_free_fn = NULL;                
    config.global_transport_ctx = NULL;                   
    config.global_transport_ctx_free_fn = NULL;           
    config.open_fn = NULL;                                
    config.close_fn = NULL;     
//    config.uri_match_fn = NULL;                            
    config.uri_match_fn = httpd_uri_match_wildcard;

    starttime = getCurrentTimeString("%Y%m%d-%H%M%S");

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}


void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server);
}


void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}


void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}
