#include "connect_wlan.h"



#include <string.h>
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include <arpa/inet.h>

#include "Helper.h"




static const char *MAIN_TAG = "connect_wlan";

std::string ssid;
std::string passphrase;
std::string hostname;
std::string ipaddress;
std::string gw;
std::string netmask;
std::string dns;

std::string std_hostname = "watermeter";

static EventGroupHandle_t wifi_event_group;


#define BLINK_GPIO GPIO_NUM_33


std::vector<string> ZerlegeZeile(std::string input, std::string _delimiter = "")
{
	std::vector<string> Output;
	std::string delimiter = " =,";
    if (_delimiter.length() > 0){
        delimiter = _delimiter;
    }

	input = trim(input, delimiter);
	size_t pos = findDelimiterPos(input, delimiter);
	std::string token;
	while (pos != std::string::npos) {
		token = input.substr(0, pos);
		token = trim(token, delimiter);
		Output.push_back(token);
		input.erase(0, pos + 1);
		input = trim(input, delimiter);
		pos = findDelimiterPos(input, delimiter);
	}
	Output.push_back(input);

	return Output;
}




void wifi_connect(){
    wifi_config_t cfg = { };
    strcpy((char*)cfg.sta.ssid, (const char*)ssid.c_str());
    strcpy((char*)cfg.sta.password, (const char*)passphrase.c_str());
    
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

void blinkstatus(int dauer, int _anzahl)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    for (int i = 0; i < _anzahl; ++i)
    {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(dauer / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(dauer / portTICK_PERIOD_MS);          
    }
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        blinkstatus(200, 1);
        wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        blinkstatus(1000, 3);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        blinkstatus(200, 5);
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void initialise_wifi(std::string _ssid, std::string _passphrase, std::string _hostname)
{
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    wifi_event_group = xEventGroupCreate();  
    ssid = _ssid;
    passphrase = _passphrase;
    hostname = _hostname;
    esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    esp_err_t ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA , hostname.c_str());
    if(ret != ESP_OK ){
      ESP_LOGE(MAIN_TAG,"failed to set hostname:%d",ret);  
    }
    xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,true,true,portMAX_DELAY);
    tcpip_adapter_ip_info_t ip_info;
    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
    ipaddress = std::string(ip4addr_ntoa(&ip_info.ip));
    netmask = std::string(ip4addr_ntoa(&ip_info.netmask));
    gw = std::string(ip4addr_ntoa(&ip_info.gw));
    printf("IPv4 :  %s\n", ip4addr_ntoa(&ip_info.ip));
    printf("HostName :  %s\n", hostname.c_str());
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void strinttoip4(std::string ip, int &a, int &b, int &c, int &d) {
    std::stringstream s(ip);
    char ch; //to temporarily store the '.'
    s >> a >> ch >> b >> ch >> c >> ch >> d;
}

void initialise_wifi_fixed_ip(std::string _ip, std::string _gw, std::string _netmask, std::string _ssid, std::string _passphrase, std::string _hostname, std::string _dns)
{

    ssid = _ssid;
    passphrase = _passphrase;
    hostname = _hostname;
    dns = _dns;

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();

    esp_netif_dhcpc_stop(my_sta);

    esp_netif_ip_info_t ip_info;

    int a, b, c, d;

    strinttoip4(_ip, a, b, c, d);
    IP4_ADDR(&ip_info.ip, a, b, c, d);

    strinttoip4(_gw, a, b, c, d);
   	IP4_ADDR(&ip_info.gw, a, b, c, d);

    strinttoip4(_netmask, a, b, c, d);
   	IP4_ADDR(&ip_info.netmask, a, b, c, d);

    esp_netif_set_ip_info(my_sta, &ip_info);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if (dns.length() > 0) {
        esp_netif_dns_info_t dns_info;
        ip4_addr_t ip;
        ip.addr = esp_ip4addr_aton(dns.c_str());
        ip_addr_set_ip4_u32(&dns_info.ip, ip.addr);
        ESP_ERROR_CHECK(esp_netif_set_dns_info(my_sta, ESP_NETIF_DNS_MAIN, &dns_info));
    }

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_config_t wifi_config = { };
    strcpy((char*)wifi_config.sta.ssid, (const char*)ssid.c_str());
    strcpy((char*)wifi_config.sta.password, (const char*)passphrase.c_str());

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(MAIN_TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,true,true,portMAX_DELAY);

    if (bits & CONNECTED_BIT) {
        ESP_LOGI(MAIN_TAG, "connected to ap SSID:%s password:%s",
                 ssid.c_str(), passphrase.c_str());
    } else  {
        ESP_LOGI(MAIN_TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid.c_str(), passphrase.c_str());
    }
    tcpip_adapter_ip_info_t ip_info2;
    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info2));
    ipaddress = std::string(ip4addr_ntoa(&ip_info2.ip));
    netmask = std::string(ip4addr_ntoa(&ip_info2.netmask));
    gw = std::string(ip4addr_ntoa(&ip_info2.gw));

    vEventGroupDelete(wifi_event_group);
}


bool ChangeHostName(std::string fn, std::string _newhostname)
{
    if (_newhostname == hostname)
        return false;

    string line = "";
    std::vector<string> zerlegt;

    bool found = false;

    std::vector<string> neuesfile;

    FILE* pFile;
    fn = FormatFileName(fn);
    pFile = OpenFileAndWait(fn.c_str(), "r");

    printf("file loaded\n");

    if (pFile == NULL)
        return false;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
        printf("%s", line.c_str());
        zerlegt = ZerlegeZeile(line, "=");
        zerlegt[0] = trim(zerlegt[0], " ");

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "HOSTNAME")){
            line = "hostname = \"" + _newhostname + "\"\n";
            found = true;
        }

        neuesfile.push_back(line);

        if (fgets(zw, 1024, pFile) == NULL)
        {
            line = "";
        }
        else
        {
            line = std::string(zw);
        }
    }

    if (!found)
    {
        line = "hostname = \"" + _newhostname + "\"\n";
        neuesfile.push_back(line);        
    }

    fclose(pFile);

    pFile = OpenFileAndWait(fn.c_str(), "w+");

    for (int i = 0; i < neuesfile.size(); ++i)
    {
        fputs(neuesfile[i].c_str(), pFile);
    }

    fclose(pFile);

    return true;
}


void LoadWlanFromFile(std::string fn, std::string &_ssid, std::string &_passphrase, std::string &_hostname)
{
    string line = "";
    std::vector<string> zerlegt;
    _hostname = std_hostname;

    FILE* pFile;
    fn = FormatFileName(fn);
    pFile = OpenFileAndWait(fn.c_str(), "r");

    printf("file loaded\n");

    if (pFile == NULL)
        return;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
        printf("%s", line.c_str());
        zerlegt = ZerlegeZeile(line, "=");
        zerlegt[0] = trim(zerlegt[0], " ");

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "HOSTNAME")){
            _hostname = trim(zerlegt[1]);
            if ((_hostname[0] == '"') && (_hostname[_hostname.length()-1] == '"')){
                _hostname = _hostname.substr(1, _hostname.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "SSID")){
            _ssid = trim(zerlegt[1]);
            if ((_ssid[0] == '"') && (_ssid[_ssid.length()-1] == '"')){
                _ssid = _ssid.substr(1, _ssid.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "PASSWORD")){
            _passphrase = zerlegt[1];
            if ((_passphrase[0] == '"') && (_passphrase[_passphrase.length()-1] == '"')){
                _passphrase = _passphrase.substr(1, _passphrase.length()-2);
            }
        }

        if (fgets(zw, 1024, pFile) == NULL)
        {
            line = "";
        }
        else
        {
            line = std::string(zw);
        }
    }

    fclose(pFile);

    // Check if Hostname was empty in .ini if yes set to std_hostname
    if(_hostname.length() <= 0){
        _hostname = std_hostname;
    }
}

void LoadNetConfigFromFile(std::string fn, std::string &_ip, std::string &_gw, std::string &_netmask, std::string &_dns)
{
    string line = "";
    std::vector<string> zerlegt;

    FILE* pFile;
    fn = FormatFileName(fn);
    pFile = OpenFileAndWait(fn.c_str(), "r");

    printf("file loaded\n");

    if (pFile == NULL)
        return;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
        printf("%s", line.c_str());
        zerlegt = ZerlegeZeile(line, "=");
        zerlegt[0] = trim(zerlegt[0], " ");

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "IP")){
            _ip = zerlegt[1];
            if ((_ip[0] == '"') && (_ip[_ip.length()-1] == '"')){
                _ip = _ip.substr(1, _ip.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "GATEWAY")){
            _gw = zerlegt[1];
            if ((_gw[0] == '"') && (_gw[_gw.length()-1] == '"')){
                _gw = _gw.substr(1, _gw.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "NETMASK")){
            _netmask = zerlegt[1];
            if ((_netmask[0] == '"') && (_netmask[_netmask.length()-1] == '"')){
                _netmask = _netmask.substr(1, _netmask.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "DNS")){
            _dns = zerlegt[1];
            if ((_dns[0] == '"') && (_dns[_dns.length()-1] == '"')){
                _dns = _dns.substr(1, _dns.length()-2);
            }
        }

        if (fgets(zw, 1024, pFile) == NULL)
        {
            line = "";
        }
        else
        {
            line = std::string(zw);
        }
    }

    fclose(pFile);
}


std::string getHostname(){
    return hostname;
}

std::string getIPAddress(){
    return ipaddress;
}

std::string getSSID(){
    return ssid;
}

std::string getNetMask(){
    return netmask;
}

std::string getGW(){
    return gw;
}

