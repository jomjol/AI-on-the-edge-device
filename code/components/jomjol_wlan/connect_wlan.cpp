#include "connect_wlan.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "Helper.h"

#define __HIDE_PASSWORD



#define EXAMPLE_ESP_MAXIMUM_RETRY  1000

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

///////////////////////////////////////////////////////////
#define BLINK_GPIO GPIO_NUM_33

int BlinkDauer;
int BlinkAnzahl;
bool BlinkOff;
bool BlinkIsRunning = false;

std::string hostname = "";
std::string std_hostname = "watermeter";
std::string ipadress = "";
std::string ssid = "";
std::string ipv6en = "";
std::string std_ipv6en = "0";

std::string* getIPAddress()
{
    return &ipadress;
}

std::string* getSSID()
{
    return &ssid;
}


void task_doBlink(void *pvParameter)
{
    ESP_LOGI("BLINK", "Flash - start");
    while (BlinkIsRunning)
    {
//        ESP_LOGI("BLINK", "Blinken - wait");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    BlinkIsRunning = true;

	// Init the GPIO
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);  

    for (int i = 0; i < BlinkAnzahl; ++i)
    {
		if (BlinkAnzahl > 1)
		{
	        gpio_set_level(BLINK_GPIO, 1);
    	    vTaskDelay(BlinkDauer / portTICK_PERIOD_MS);
		}
        gpio_set_level(BLINK_GPIO, 0);      
        vTaskDelay(BlinkDauer / portTICK_PERIOD_MS);
    }

    if (BlinkOff)
        gpio_set_level(BLINK_GPIO, 1);

    ESP_LOGI("BLINK", "Flash - done");
    BlinkIsRunning = false;

    vTaskDelete(NULL); //Delete this task if it exits from the loop above
}

void LEDBlinkTask(int _dauer, int _anz, bool _off)
{
	BlinkDauer = _dauer;
	BlinkAnzahl = _anz;
	BlinkOff = _off;

    xTaskCreate(&task_doBlink, "task_doBlink", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY+1, NULL);
}
/////////////////////////////////////////////////////////

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        LEDBlinkTask(200, 1, true);
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
//        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retrying connection to the AP");
//        } else {
//            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
//        }
        ESP_LOGI(TAG,"connection to the AP failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ipadress = std::string(ip4addr_ntoa((const ip4_addr*) &event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        LEDBlinkTask(1000, 5, true);
#ifdef CONFIG_LWIP_IPV6
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6) {
	char buf[48];
	ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
	ESP_LOGI(TAG, "Got IPv6 address: " IPV6STR, IPV62STR(event->ip6_info.ip));

	ip6addr_ntoa_r((const ip6_addr*)&event->ip6_info.ip, buf, sizeof(buf));
	ipadress = std::string(buf);
	xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	LEDBlinkTask(1000, 5, true);
#endif
    }
}

#ifdef CONFIG_LWIP_IPV6
static void
on_wifi_connect(void *arg, esp_event_base_t event_base __unused,
    int32_t event_id __unused, void *event_data __unused)
{
	esp_netif_t *esp_netif = (esp_netif_t *)arg;
	ESP_LOGI(TAG, "Wifi connected. Enabling IPv6 link-local...");
	esp_netif_create_ip6_linklocal(esp_netif);
}
#endif

void strinttoip4(const char *ip, int &a, int &b, int &c, int &d) {
    std::string zw = std::string(ip);
    std::stringstream s(zw);
    char ch; //to temporarily store the '.'
    s >> a >> ch >> b >> ch >> c >> ch >> d;
}


void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname, const char *_ipadr, const char *_gw,  const char *_netmask, const char *_dns, const char *_ipv6en)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();

    if ((_ipadr != NULL) && (_gw != NULL) && (_netmask != NULL))
    {
    /*
       tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
        tcpip_adapter_ip_info_t ip_info;
        int a, b, c, d;
        strinttoip4(_ipadr, a, b, c, d);
        IP4_ADDR(&ip_info.ip, a, b, c, d);
        strinttoip4(_gw, a, b, c, d);
        IP4_ADDR(&ip_info.gw, a, b, c, d);
        strinttoip4(_netmask, a, b, c, d);
        IP4_ADDR(&ip_info.netmask, a, b, c, d);

        tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
    */


        ESP_LOGI(TAG, "set IP %s, GW %s, Netmask %s manual", _ipadr, _gw, _netmask);
        esp_netif_dhcpc_stop(my_sta);

        esp_netif_ip_info_t ip_info;
        int a, b, c, d;
        strinttoip4(_ipadr, a, b, c, d);
        IP4_ADDR(&ip_info.ip, a, b, c, d);
        strinttoip4(_gw, a, b, c, d);
        IP4_ADDR(&ip_info.gw, a, b, c, d);
        strinttoip4(_netmask, a, b, c, d);
        IP4_ADDR(&ip_info.netmask, a, b, c, d);

        esp_netif_set_ip_info(my_sta, &ip_info);
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if ((_ipadr != NULL) && (_gw != NULL) && (_netmask != NULL))
    {
        if (_dns == NULL)
            _dns = _gw;
            
        ESP_LOGI(TAG, "set DNS manual");
        esp_netif_dns_info_t dns_info;
        dns_info.ip.u_addr.ip4.addr = ipaddr_addr(_dns);
        dns_info.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(my_sta, ESP_NETIF_DNS_MAIN, &dns_info));
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    if (strval_is_true(_ipv6en)) {
#ifdef CONFIG_LWIP_IPV6
	esp_event_handler_instance_t instance_got_ip6;

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
						   WIFI_EVENT_STA_CONNECTED,
						   &on_wifi_connect,
						   my_sta));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
							    IP_EVENT_GOT_IP6,
							    &event_handler,
							    NULL,
							    &instance_got_ip6));
#else
	ESP_LOGI(TAG, "IPv6 support enabled but not compiled into image.");
#endif
    }

    wifi_config_t wifi_config = { };

    strcpy((char*)wifi_config.sta.ssid, (const char*)_ssid);
    strcpy((char*)wifi_config.sta.password, (const char*)_password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    if (_hostname != NULL)
    {
        esp_err_t ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA , _hostname);
        hostname = std::string(_hostname);
        if(ret != ESP_OK ){
            ESP_LOGE(TAG,"failed to set hostname:%d",ret);  
        }
        else {
            ESP_LOGE(TAG,"Set Hostname to:%s", _hostname); 
        }

    }

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
#ifdef __HIDE_PASSWORD
        ESP_LOGI(TAG, "connected to ap SSID: %s, password: XXXXXXX", _ssid);
#else
        ESP_LOGI(TAG, "connected to ap SSID: %s, password: %s", _ssid, _password);
#endif        
    } else if (bits & WIFI_FAIL_BIT) {
#ifdef __HIDE_PASSWORD
        ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: XXXXXXXX", _ssid);
#else
        ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: %s", _ssid, _password);
#endif        
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    ssid = std::string(_ssid);


    /* The event will not be processed after unregister */
//    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
//    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
//    vEventGroupDelete(s_wifi_event_group);
}

int get_WIFI_RSSI()
{
    wifi_ap_record_t ap;
    esp_wifi_sta_get_ap_info(&ap);
    return ap.rssi;
}

void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname, const char *_ipadr, const char *_gw,  const char *_netmask, const char *_dns)
{
    wifi_init_sta(_ssid, _password, _hostname, _ipadr, _gw, _netmask, _dns, NULL);
}

void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname)
{
    wifi_init_sta(_ssid, _password, _hostname, NULL, NULL, NULL, NULL, NULL);
}

void wifi_init_sta(const char *_ssid, const char *_password)
{
    wifi_init_sta(_ssid, _password, NULL, NULL, NULL, NULL, NULL, NULL);
}

