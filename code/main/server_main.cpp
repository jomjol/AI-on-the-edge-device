#include "server_main.h"

#include <string>

#include "server_help.h"
#include "ClassLogFile.h"

#include "time_sntp.h"

#include "connect_wlan.h"

#include "version.h"

#include "esp_wifi.h"

#include "server_tflite.h"

//#define DEBUG_DETAIL_ON      



httpd_handle_t server = NULL;   
std::string starttime = "";

static const char *TAG_SERVERMAIN = "server-main";

/* An HTTP GET handler */
esp_err_t info_get_handler(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("info_get_handler - Start");    
#endif

    LogFile.WriteToFile("info_get_handler");    
    char _query[200];
    char _valuechar[30];    
    std::string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        printf("Query: "); printf(_query); printf("\n");
        
        if (httpd_query_key_value(_query, "type", _valuechar, 30) == ESP_OK)
        {
            printf("type is found"); printf(_valuechar); printf("\n"); 
            _task = std::string(_valuechar);
        }
    };

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (_task.compare("GitBranch") == 0)
    {
        httpd_resp_sendstr_chunk(req, libfive_git_branch());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }


    if (_task.compare("GitTag") == 0)
    {
        httpd_resp_sendstr_chunk(req, libfive_git_version());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }



    if (_task.compare("GitRevision") == 0)
    {
        httpd_resp_sendstr_chunk(req, libfive_git_revision());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_task.compare("BuildTime") == 0)
    {
        httpd_resp_sendstr_chunk(req, build_time());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_task.compare("GitBaseBranch") == 0)
    {
        httpd_resp_sendstr_chunk(req, git_base_branch());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_task.compare("HTMLVersion") == 0)
    {
//        std::string zw;
//        zw = std::string(getHTMLversion());
        httpd_resp_sendstr_chunk(req, getHTMLversion());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_task.compare("Hostname") == 0)
    {
        std::string zw;
        zw = std::string(hostname);
        httpd_resp_sendstr_chunk(req, zw.c_str());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_task.compare("IP") == 0)
    {
        std::string *zw;
        zw = getIPAddress();
        httpd_resp_sendstr_chunk(req, zw->c_str());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_task.compare("SSID") == 0)
    {
        std::string *zw;
        zw = getSSID();
        httpd_resp_sendstr_chunk(req, zw->c_str());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_task.compare("FlowStatus") == 0)
    {
        std::string zw;
        zw = std::string("FlowStatus");
        httpd_resp_sendstr_chunk(req, zw.c_str());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    return ESP_OK;
}

esp_err_t starttime_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, starttime.c_str(), strlen(starttime.c_str())); 
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);  

    return ESP_OK;
}

esp_err_t hello_main_handler(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("hello_main_handler - Start");
#endif

    char filepath[50];
    printf("uri: %s\n", req->uri);
    int _pos;
    esp_err_t res;

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = get_path_from_uri(filepath, base_path,
                                             req->uri - 1, sizeof(filepath));    
    printf("1 uri: %s, filename: %s, filepath: %s\n", req->uri, filename, filepath);

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

    if (filetosend == "/sdcard/html/index.html" && isSetupModusActive()) {
        printf("System ist im Setupmodus --> index.html --> setup.html");
        filetosend = "/sdcard/html/setup.html";
    }

    printf("Filename: %s\n", filename);
    
    printf("File requested: %s\n", filetosend.c_str());    

    if (!filename) {
        ESP_LOGE(TAG_SERVERMAIN, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
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
    printf("uri: %s\n", req->uri);

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = get_path_from_uri(filepath, base_path,
                                             req->uri  + sizeof("/img_tmp/") - 1, sizeof(filepath));    
    printf("1 uri: %s, filename: %s, filepath: %s\n", req->uri, filename, filepath);

    filetosend = filetosend + "/img_tmp/" + std::string(filename);
    printf("File to upload: %s\n", filetosend.c_str());    

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

    printf("uri: %s\n", req->uri);

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = get_path_from_uri(filepath, base_path,
                                             req->uri  + sizeof("/img_tmp/") - 1, sizeof(filepath));    
    printf("1 uri: %s, filename: %s, filepath: %s\n", req->uri, filename, filepath);

    filetosend = std::string(filename);
    printf("File to upload: %s\n", filetosend.c_str()); 

    if (filetosend == "raw.jpg")
    {
        return GetRawJPG(req); 
    } 

    esp_err_t zw = GetJPG(filetosend, req);

    if (zw == ESP_OK)
        return ESP_OK;

    // File wird nicht intern bereit gestellt --> klassischer weg:
#ifdef DEBUG_DETAIL_ON      
    LogFile.WriteHeapInfo("img_tmp_virtual_handler - Done");   
#endif

    return img_tmp_handler(req);
}





esp_err_t sysinfo_handler(httpd_req_t *req)
{
    const char* resp_str; 
    std::string zw;
    std::string cputemp = std::to_string(temperatureRead());
    std::string gitversion = libfive_git_version();
    std::string buildtime = build_time();
    std::string gitbranch = libfive_git_branch();
    std::string gitbasebranch = git_base_branch();
    std::string htmlversion = getHTMLversion();
    char freeheapmem[11];
    sprintf(freeheapmem, "%zu", esp_get_free_heap_size());
    
    tcpip_adapter_ip_info_t ip_info;
    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
    const char *hostname;
    ESP_ERROR_CHECK(tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, &hostname));
    
    zw = "[\
            {\
                \"firmware\" : \"" + gitversion + "\",\
                \"buildtime\" : \"" + buildtime + "\",\
                \"gitbranch\" : \"" + gitbranch + "\",\
                \"gitbasebranch\" : \"" + gitbasebranch + "\",\
                \"html\" : \"" + htmlversion + "\",\
                \"cputemp\" : \"" + cputemp + "\",\
                \"hostname\" : \"" + hostname + "\",\
                \"IPv4\" : \"" + ip4addr_ntoa(&ip_info.ip) + "\",\
                \"freeHeapMem\" : \"" + freeheapmem + "\"\
            }\
        ]";

    resp_str = zw.c_str();

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp_str, strlen(resp_str));   
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);  

    return ESP_OK;
}

void register_server_main_uri(httpd_handle_t server, const char *base_path)
{
    httpd_uri_t info_get_handle = {
        .uri       = "/version",  // Match all URIs of type /path/to/file
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

    config.task_priority      = tskIDLE_PRIORITY+1;         // 20210924 --> vorher +5
    config.stack_size         = 32768;      //20210921 --> vorher 32768             // bei 32k stürzt das Programm beim Bilderaufnehmen ab
    config.core_id            = tskNO_AFFINITY;
    config.server_port        = 80;
    config.ctrl_port          = 32768;
    config.max_open_sockets   = 5;          //20210921 --> vorher 7   
    config.max_uri_handlers   = 24;                       
    config.max_resp_headers   = 8;                        
    config.backlog_conn       = 5;                        
    config.lru_purge_enable   = true;       // dadurch werden alte Verbindungen gekappt, falls neue benögt werden.               
    config.recv_wait_timeout  = 5;         // default: 5         20210924 --> vorher 30              
    config.send_wait_timeout  = 5;         // default: 5         20210924 --> vorher 30                   
    config.global_user_ctx = NULL;                        
    config.global_user_ctx_free_fn = NULL;                
    config.global_transport_ctx = NULL;                   
    config.global_transport_ctx_free_fn = NULL;           
    config.open_fn = NULL;                                
    config.close_fn = NULL;     
//    config.uri_match_fn = NULL;                            
    config.uri_match_fn = httpd_uri_match_wildcard;

    starttime = gettimestring("%Y%m%d-%H%M%S");

    // Start the httpd server
    ESP_LOGI(TAG_SERVERMAIN, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG_SERVERMAIN, "Registering URI handlers");
        return server;
    }

    ESP_LOGI(TAG_SERVERMAIN, "Error starting server!");
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
        ESP_LOGI(TAG_SERVERMAIN, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG_SERVERMAIN, "Starting webserver");
        *server = start_webserver();
    }
}
