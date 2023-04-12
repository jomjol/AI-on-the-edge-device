#ifdef ENABLE_SOFTAP
//if ENABLE_SOFTAP = disabled, set CONFIG_ESP_WIFI_SOFTAP_SUPPORT=n in sdkconfig.defaults to save 28k of flash
#include "../../include/defines.h"


#include "softAP.h"

/*  WiFi softAP Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "stdio.h"

#include "ClassLogFile.h"
#include "server_help.h"
#include "defines.h"
#include "Helper.h"
#include "statusled.h"
#include "server_ota.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu.
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

bool isConfigINI = false;
bool isWlanINI = false;

static const char *TAG = "WIFI AP";


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}


void wifi_init_softAP(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = { };

    strcpy((char*)wifi_config.ap.ssid, (const char*) EXAMPLE_ESP_WIFI_SSID);
    strcpy((char*)wifi_config.ap.password, (const char*) EXAMPLE_ESP_WIFI_PASS);
    wifi_config.ap.channel = EXAMPLE_ESP_WIFI_CHANNEL;
    wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "started with SSID \"%s\", password: \"%s\", channel: %d. Connect to AP and open http://192.168.4.1",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}


void SendHTTPResponse(httpd_req_t *req)
{
    std::string message = "<h1>AI-on-the-edge - BASIC SETUP</h1><p>This is an access point with a minimal server to setup the minimum required files and information on the device and the SD-card. ";
    message += "This mode is always startet if one of the following files is missing: /wlan.ini or the /config/config.ini.<p>";
    message += "The setup is done in 3 steps: 1. upload full inital configuration (sd-card content), 2. store WLAN acces information, 3. reboot (and connect to WLANs)<p><p>";
    message += "Please follow the below instructions.<p>";
    httpd_resp_send_chunk(req, message.c_str(), strlen(message.c_str()));

    isWlanINI = FileExists(WLAN_CONFIG_FILE);

    if (!isConfigINI)
    {
        message = "<h3>1. Upload initial configuration to sd-card</h3><p>";
        message += "The configuration file config.ini is missing and most propably the full configuration and html folder on the sd-card. ";
        message += "This is normal after the first flashing of the firmware and an empty sd-card. Please upload \"remote_setup.zip\", which contains an full inital configuration.<p>";
        message += "<input id=\"newfile\" type=\"file\"><br>";
        message += "<button class=\"button\" style=\"width:300px\" id=\"doUpdate\" type=\"button\" onclick=\"upload()\">Upload File</button><p>";
        message += "The upload might take up to 60s. After a succesfull upload the page will be updated.";
        httpd_resp_send_chunk(req, message.c_str(), strlen(message.c_str()));

        message = "<script language=\"JavaScript\">";
        message += "function upload() {";
        message += "var xhttp = new XMLHttpRequest();";
        message += "xhttp.onreadystatechange = function() {if (xhttp.readyState == 4) {if (xhttp.status == 200) {location.reload();}}};";
        message += "var filePath = document.getElementById(\"newfile\").value.split(/[\\\\/]/).pop();";
        message += "var file = document.getElementById(\"newfile\").files[0];";
        message += "if (!file.name.includes(\"remote-setup\")){if (!confirm(\"The zip file name should contain '...remote-setup...'. Are you sure that you have downloaded the correct file?\"))return;};";
        message += "var upload_path = \"/upload/firmware/\" + filePath; xhttp.open(\"POST\", upload_path, true); xhttp.send(file);document.reload();";
        message += "document.getElementById(\"doUpdate\").disabled = true;}";
        message += "</script>";
        httpd_resp_send_chunk(req, message.c_str(), strlen(message.c_str()));
        return;
    }
    if (!isWlanINI)
    {
        message = "<h3>2. WLAN access credentials</h3><p>";
        message += "<table>";
        message += "<tr><td>WLAN-SSID</td><td><input type=\"text\" name=\"ssid\" id=\"ssid\"></td><td>SSID of the WLAN</td></tr>";
        message += "<tr><td>WLAN-Password</td><td><input type=\"text\" name=\"password\" id=\"password\"></td><td>ATTENTION: the password will not be encrypted during the sending.</td><tr>";
        message += "</table><p>";
        message += "<h4>ATTENTION:<h4>Be sure about the WLAN settings. They cannot be reseted afterwards. If ssid or password is wrong, you need to take out the sd-card and manually change them in \"wlan.ini\"!<p>";
        httpd_resp_send_chunk(req, message.c_str(), strlen(message.c_str()));

//        message = "</tr><tr><td> Hostname</td><td><input type=\"text\" name=\"hostname\" id=\"hostname\"></td><td></td>";
//        message += "</tr><tr><td>Fixed IP</td><td><input type=\"text\" name=\"ip\" id=\"ip\"></td><td>Leave emtpy if set by router (DHCP)</td></tr>";
//        message += "<tr><td>Gateway</td><td><input type=\"text\" name=\"gateway\" id=\"gateway\"></td><td>Leave emtpy if set by router (DHCP)</td></tr>";
//        message += "<tr><td>Netmask</td><td><input type=\"text\" name=\"netmask\" id=\"netmask\"></td><td>Leave emtpy if set by router (DHCP)</td>";
//        message += "</tr><tr><td>DNS</td><td><input type=\"text\" name=\"dns\" id=\"dns\"></td><td>Leave emtpy if set by router (DHCP)</td></tr>";
//        message += "<tr><td>RSSI Threshold</td><td><input type=\"number\" name=\"name\" id=\"threshold\" min=\"-100\"  max=\"0\" step=\"1\" value = \"0\"></td><td>WLAN Mesh Parameter: Threshold for RSSI value to check for start switching access point in a mesh system (if actual RSSI is lower). Possible values: -100 to 0, 0 = disabled - Value will be transfered to wlan.ini at next startup)</td></tr>";
//        httpd_resp_send_chunk(req, message.c_str(), strlen(message.c_str()));


        message = "<button class=\"button\" type=\"button\" onclick=\"wr()\">Write wlan.ini</button>";
        message += "<script language=\"JavaScript\">async function wr(){";
        message += "api = \"/config?\"+\"ssid=\"+document.getElementById(\"ssid\").value+\"&pwd=\"+document.getElementById(\"password\").value;";
//        message += "api = \"/config?\"+\"ssid=\"+document.getElementById(\"ssid\").value+\"&pwd=\"+document.getElementById(\"password\").value+\"&hn=\"+document.getElementById(\"hostname\").value+\"&ip=\"+document.getElementById(\"ip\").value+\"&gw=\"+document.getElementById(\"gateway\").value+\"&nm=\"+document.getElementById(\"netmask\").value+\"&dns=\"+document.getElementById(\"dns\").value+\"&rssithreshold=\"+document.getElementById(\"threshold\").value;";
        message += "fetch(api);await new Promise(resolve => setTimeout(resolve, 1000));location.reload();}</script>";
        httpd_resp_send_chunk(req, message.c_str(), strlen(message.c_str()));
        return;
    }

    message = "<h3>3. Reboot</h3><p>";
    message += "After triggering the reboot, the zip-files gets extracted and written to the sd-card.<br>The ESP32 will restart two times and then connect to your access point. Please find the IP in your router settings and access it with the new ip-address.<p>";
    message += "The first update and initialization process can take up to 3 minutes before you find it in the wlan. Error logs can be found on the console / serial logout.<p>Have fun!<p>";
    message += "<button class=\"button\" type=\"button\" onclick=\"rb()\")>Reboot to first setup.</button>";
    message += "<script language=\"JavaScript\">async function rb(){";
    message += "api = \"/reboot\";";
    message += "fetch(api);await new Promise(resolve => setTimeout(resolve, 1000));location.reload();}</script>";
    httpd_resp_send_chunk(req, message.c_str(), strlen(message.c_str()));
}


esp_err_t test_handler(httpd_req_t *req)
{
    SendHTTPResponse(req);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


esp_err_t reboot_handlerAP(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON     
    LogFile.WriteHeapInfo("handler_ota_update - Start");    
#endif
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Trigger reboot due to firmware update.");
    doRebootOTA();
    return ESP_OK;
}


esp_err_t config_ini_handler(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON     
    LogFile.WriteHeapInfo("handler_ota_update - Start");    
#endif

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "config_ini_handler");
    char _query[400];
    char _valuechar[100];    
    std::string fn = "/sdcard/firmware/";
    std::string _task = "";
    std::string ssid = "";
    std::string pwd = "";
    std::string hn = "";    // hostname
    std::string ip = "";
    std::string gw = "";    // gateway
    std::string nm = "";    // netmask
    std::string dns = "";
    std::string rssithreshold = ""; //rssi threshold for WIFI roaming
    std::string text = "";


    if (httpd_req_get_url_query_str(req, _query, 400) == ESP_OK)
    {
        ESP_LOGD(TAG, "Query: %s", _query);
        
        if (httpd_query_key_value(_query, "ssid", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "ssid is found: %s", _valuechar);
            ssid = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "pwd", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "pwd is found: %s", _valuechar);
            pwd = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "ssid", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "ssid is found: %s", _valuechar);
            ssid = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "hn", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "hostname is found: %s", _valuechar);
            hn = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "ip", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "ip is found: %s", _valuechar);
            ip = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "gw", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "gateway is found: %s", _valuechar);
            gw = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "nm", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "netmask is found: %s", _valuechar);
            nm = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "dns", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "dns is found: %s", _valuechar);
            dns = UrlDecode(std::string(_valuechar));
        }

        if (httpd_query_key_value(_query, "rssithreshold", _valuechar, 100) == ESP_OK)
        {
            ESP_LOGD(TAG, "rssithreshold is found: %s", _valuechar);
            rssithreshold = UrlDecode(std::string(_valuechar));
        }
    }

    FILE* configfilehandle = fopen(WLAN_CONFIG_FILE, "w");

    text  = ";++++++++++++++++++++++++++++++++++\n";
    text += "; AI on the edge - WLAN configuration\n";
    text += "; ssid: Name of WLAN network (mandatory), e.g. \"WLAN-SSID\"\n";
    text += "; password: Password of WLAN network (mandatory), e.g. \"PASSWORD\"\n\n";
    fputs(text.c_str(), configfilehandle);
    
    if (ssid.length())
        ssid = "ssid = \"" + ssid + "\"\n";
    else
        ssid = "ssid = \"\"\n";
    fputs(ssid.c_str(), configfilehandle);

    if (pwd.length())
        pwd = "password = \"" + pwd + "\"\n";
    else
        pwd = "password = \"\"\n";
    fputs(pwd.c_str(), configfilehandle);

    text  = "\n;++++++++++++++++++++++++++++++++++\n";
    text += "; Hostname: Name of device in network\n";
    text += "; This parameter can be configured via WebUI configuration\n";
    text += "; Default: \"watermeter\", if nothing is configured\n\n";
    fputs(text.c_str(), configfilehandle);

    if (hn.length())
        hn = "hostname = \"" + hn + "\"\n";
    else
        hn = ";hostname = \"watermeter\"\n";
    fputs(hn.c_str(), configfilehandle);

    text  = "\n;++++++++++++++++++++++++++++++++++\n";
    text += "; Fixed IP: If you like to use fixed IP instead of DHCP (default), the following\n";
    text += "; parameters needs to be configured: ip, gateway, netmask are mandatory, dns optional\n\n";
    fputs(text.c_str(), configfilehandle);

    if (ip.length())
        ip = "ip = \"" + ip + "\"\n";
    else
        ip = ";ip = \"xxx.xxx.xxx.xxx\"\n";
    fputs(ip.c_str(), configfilehandle);

    if (gw.length())
        gw = "gateway = \"" + gw + "\"\n";
    else
        gw = ";gateway = \"xxx.xxx.xxx.xxx\"\n";
    fputs(gw.c_str(), configfilehandle);

    if (nm.length())
        nm = "netmask = \"" + nm + "\"\n";
    else
        nm = ";netmask = \"xxx.xxx.xxx.xxx\"\n";
    fputs(nm.c_str(), configfilehandle);

    text  = "\n;++++++++++++++++++++++++++++++++++\n";
    text += "; DNS server (optional, if no DNS is configured, gateway address will be used)\n\n";
    fputs(text.c_str(), configfilehandle);

    if (dns.length())
        dns = "dns = \"" + dns + "\"\n";
    else
        dns = ";dns = \"xxx.xxx.xxx.xxx\"\n";
    fputs(dns.c_str(), configfilehandle);

    text  = "\n;++++++++++++++++++++++++++++++++++\n";
    text += "; WIFI Roaming:\n";
    text += "; Network assisted roaming protocol is activated by default\n";
    text += "; AP / mesh system needs to support roaming protocol 802.11k/v\n";
    text += ";\n";
    text += "; Optional feature (usually not neccessary):\n";
    text += "; RSSI Threshold for client requested roaming query (RSSI < RSSIThreshold)\n";
    text += "; Note: This parameter can be configured via WebUI configuration\n";
    text += "; Default: 0 = Disable client requested roaming query\n\n";
    fputs(text.c_str(), configfilehandle);

    if (rssithreshold.length())
        rssithreshold = "RSSIThreshold = " + rssithreshold + "\n";
    else
        rssithreshold = "RSSIThreshold = 0\n";
    fputs(rssithreshold.c_str(), configfilehandle);

    fflush(configfilehandle);
    fclose(configfilehandle);

    std::string zw = "ota without parameter - should not be the case!";
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, zw.c_str(), zw.length()); 

    ESP_LOGD(TAG, "end config.ini");

    return ESP_OK;
}


esp_err_t upload_post_handlerAP(httpd_req_t *req)
{
    printf("Start des Post Handlers\n");
    MakeDir("/sdcard/config");
    MakeDir("/sdcard/firmware");
    MakeDir("/sdcard/html");
    MakeDir("/sdcard/img_tmp");
    MakeDir("/sdcard/log");
    MakeDir("/sdcard/demo");
    printf("Nach Start des Post Handlers\n");

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "upload_post_handlerAP");
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;

    const char *filename = get_path_from_uri(filepath, "/sdcard",
                                             req->uri + sizeof("/upload") - 1, sizeof(filepath));
    if (!filename) {
        httpd_resp_send_err(req, HTTPD_414_URI_TOO_LONG, "Filename too long");
        return ESP_FAIL;
    }

    printf("filepath: %s, filename: %s\n", filepath, filename);

    DeleteFile(std::string(filepath));



    fd = fopen(filepath, "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to create file: %s", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Receiving file: %s...", filename);

    char buf[1024];
    int received;

    int remaining = req->content_len;

    printf("remaining: %d\n", remaining);



    while (remaining > 0) {

        ESP_LOGI(TAG, "Remaining size: %d", remaining);
        if ((received = httpd_req_recv(req, buf, MIN(remaining, 1024))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }

            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File reception failed!");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        if (received && (received != fwrite(buf, 1, received, fd))) {
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File write failed!");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
            return ESP_FAIL;
        }

        remaining -= received;
    }
    fclose(fd);
    isConfigINI = true;

    FILE* pfile = fopen("/sdcard/update.txt", "w");
    std::string _s_zw= "/sdcard" + std::string(filename);
    fwrite(_s_zw.c_str(), strlen(_s_zw.c_str()), 1, pfile);
    fclose(pfile);


    ESP_LOGI(TAG, "File reception complete");
    httpd_resp_set_hdr(req, "Location", "/test");
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/test");
    httpd_resp_send_chunk(req, NULL, 0);

    ESP_LOGI(TAG, "Update page send out");

    return ESP_OK;
}


httpd_handle_t start_webserverAP(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        // Do something
    }

    httpd_uri_t reboot_handle = {
        .uri       = "/reboot",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = reboot_handlerAP,
        .user_ctx  = NULL    // Pass server data as context
    };
    httpd_register_uri_handler(server, &reboot_handle);

    httpd_uri_t config_ini_handle = {
        .uri       = "/config",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = config_ini_handler,
        .user_ctx  = NULL    // Pass server data as context
    };
    httpd_register_uri_handler(server, &config_ini_handle);

    /* URI handler for uploading files to server */
    httpd_uri_t file_uploadAP = {
        .uri       = "/upload/*",   // Match all URIs of type /upload/path/to/file
        .method    = HTTP_POST,
        .handler   = upload_post_handlerAP,
        .user_ctx  = NULL    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_uploadAP);

    httpd_uri_t test_uri = {
        .uri      = "*",
        .method   = HTTP_GET,
        .handler  = test_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &test_uri);

    return NULL;
}


void CheckStartAPMode()
{
    isConfigINI = FileExists(CONFIG_FILE);
    isWlanINI = FileExists(WLAN_CONFIG_FILE);

    if (!isConfigINI)
        ESP_LOGW(TAG, "config.ini not found!");

    if (!isWlanINI)
        ESP_LOGW(TAG, "wlan.ini not found!");

    if (!isConfigINI || !isWlanINI)
    {
        ESP_LOGI(TAG, "Starting access point for remote configuration");
        StatusLED(AP_OR_OTA, 2, true);
        wifi_init_softAP();
        start_webserverAP();
        while(1) { // wait until reboot within task_do_Update_ZIP
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

#endif //#ifdef ENABLE_SOFTAP
