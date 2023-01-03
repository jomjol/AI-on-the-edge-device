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
#ifdef ENABLE_MQTT
    #include "interface_mqtt.h"
#endif //ENABLE_MQTT

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

//////////////////////
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wnm.h"
#include "esp_rrm.h"
#include "esp_mbo.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_netif.h"


/////////////////////
#include "../../include/defines.h"


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "WIFI";

static int s_retry_num = 0;
bool WIFIConnected = false;

///////////////////////////////////////////////////////////

int BlinkDauer;
int BlinkAnzahl;
bool BlinkOff;
bool BlinkIsRunning = false;

std::string hostname = "";
std::string std_hostname = "watermeter";
std::string ipadress = "";
std::string ssid = "";
int RSSIThreashold;

/////////////////////////////////
/////////////////////////////////

#ifdef WLAN_USE_MESH_ROAMING

int RSSI_Threshold = WLAN_WIFI_RSSI_THRESHOLD;

/* rrm ctx */
int rrm_ctx = 0;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* esp netif object representing the WIFI station */
static esp_netif_t *sta_netif = NULL;

//static const char *TAG = "roaming_example";

static inline uint32_t WPA_GET_LE32(const uint8_t *a)
{
	return ((uint32_t) a[3] << 24) | (a[2] << 16) | (a[1] << 8) | a[0];
}


#ifndef WLAN_EID_MEASURE_REPORT
#define WLAN_EID_MEASURE_REPORT 39
#endif
#ifndef MEASURE_TYPE_LCI
#define MEASURE_TYPE_LCI 9
#endif
#ifndef MEASURE_TYPE_LOCATION_CIVIC
#define MEASURE_TYPE_LOCATION_CIVIC 11
#endif
#ifndef WLAN_EID_NEIGHBOR_REPORT
#define WLAN_EID_NEIGHBOR_REPORT 52
#endif
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define MAX_NEIGHBOR_LEN 512
static char * get_btm_neighbor_list(uint8_t *report, size_t report_len)
{
	size_t len = 0;
	const uint8_t *data;
	int ret = 0;

	/*
	 * Neighbor Report element (IEEE P802.11-REVmc/D5.0)
	 * BSSID[6]
	 * BSSID Information[4]
	 * Operating Class[1]
	 * Channel Number[1]
	 * PHY Type[1]
	 * Optional Subelements[variable]
	 */
#define NR_IE_MIN_LEN (ETH_ALEN + 4 + 1 + 1 + 1)

	if (!report || report_len == 0) {
		ESP_LOGI(TAG, "RRM neighbor report is not valid");
		return NULL;
	}

	char *buf = (char*) calloc(1, MAX_NEIGHBOR_LEN);
	data = report;

	while (report_len >= 2 + NR_IE_MIN_LEN) {
		const uint8_t *nr;
		char lci[256 * 2 + 1];
		char civic[256 * 2 + 1];
		uint8_t nr_len = data[1];
		const uint8_t *pos = data, *end;

		if (pos[0] != WLAN_EID_NEIGHBOR_REPORT ||
		    nr_len < NR_IE_MIN_LEN) {
			ESP_LOGI(TAG, "CTRL: Invalid Neighbor Report element: id=%u len=%u",
					data[0], nr_len);
			ret = -1;
			goto cleanup;
		}

		if (2U + nr_len > report_len) {
			ESP_LOGI(TAG, "CTRL: Invalid Neighbor Report element: id=%u len=%zu nr_len=%u",
					data[0], report_len, nr_len);
			ret = -1;
			goto cleanup;
		}
		pos += 2;
		end = pos + nr_len;

		nr = pos;
		pos += NR_IE_MIN_LEN;

		lci[0] = '\0';
		civic[0] = '\0';
		while (end - pos > 2) {
			uint8_t s_id, s_len;

			s_id = *pos++;
			s_len = *pos++;
			if (s_len > end - pos) {
				ret = -1;
				goto cleanup;
			}
			if (s_id == WLAN_EID_MEASURE_REPORT && s_len > 3) {
				/* Measurement Token[1] */
				/* Measurement Report Mode[1] */
				/* Measurement Type[1] */
				/* Measurement Report[variable] */
				switch (pos[2]) {
					case MEASURE_TYPE_LCI:
						if (lci[0])
							break;
						memcpy(lci, pos, s_len);
						break;
					case MEASURE_TYPE_LOCATION_CIVIC:
						if (civic[0])
							break;
						memcpy(civic, pos, s_len);
						break;
				}
			}

			pos += s_len;
		}

		ESP_LOGI(TAG, "RMM neigbor report bssid=" MACSTR
				" info=0x%x op_class=%u chan=%u phy_type=%u%s%s%s%s",
				MAC2STR(nr), WPA_GET_LE32(nr + ETH_ALEN),
				nr[ETH_ALEN + 4], nr[ETH_ALEN + 5],
				nr[ETH_ALEN + 6],
				lci[0] ? " lci=" : "", lci,
				civic[0] ? " civic=" : "", civic);

		/* neighbor start */
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, " neighbor=");
		/* bssid */
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, MACSTR, MAC2STR(nr));
		/* , */
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
		/* bssid info */
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "0x%04x", WPA_GET_LE32(nr + ETH_ALEN));
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
		/* operating class */
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "%u", nr[ETH_ALEN + 4]);
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
		/* channel number */
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "%u", nr[ETH_ALEN + 5]);
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
		/* phy type */
		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "%u", nr[ETH_ALEN + 6]);
		/* optional elements, skip */

		data = end;
		report_len -= 2 + nr_len;
	}

cleanup:
	if (ret < 0) {
		free(buf);
		buf = NULL;
	}
	return buf;
}


void neighbor_report_recv_cb(void *ctx, const uint8_t *report, size_t report_len)
{
	int *val = (int*) ctx;
	uint8_t *pos = (uint8_t *)report;
	int cand_list = 0;

	if (!report) {
		ESP_LOGE(TAG, "report is null");
		return;
	}
	if (*val != rrm_ctx) {
		ESP_LOGE(TAG, "rrm_ctx value didn't match, not initiated by us");
		return;
	}
	/* dump report info */
	ESP_LOGI(TAG, "rrm: neighbor report len=%d", report_len);
	ESP_LOG_BUFFER_HEXDUMP(TAG, pos, report_len, ESP_LOG_INFO);

	/* create neighbor list */
	char *neighbor_list = get_btm_neighbor_list(pos + 1, report_len - 1);

	/* In case neighbor list is not present issue a scan and get the list from that */
	if (!neighbor_list) {
		/* issue scan */
		wifi_scan_config_t params;
		memset(&params, 0, sizeof(wifi_scan_config_t));
		if (esp_wifi_scan_start(&params, true) < 0) {
			goto cleanup;
		}
		/* cleanup from net802.11 */
		uint16_t number = 1;
		wifi_ap_record_t ap_records;
		esp_wifi_scan_get_ap_records(&number, &ap_records);
		cand_list = 1;
	}
	/* send AP btm query, this will cause STA to roam as well */
	esp_wnm_send_bss_transition_mgmt_query(REASON_FRAME_LOSS, neighbor_list, cand_list);

cleanup:
	if (neighbor_list)
		free(neighbor_list);
}


static void esp_bss_rssi_low_handler(void* arg, esp_event_base_t event_base,
		int32_t event_id, void* event_data)
{
	wifi_event_bss_rssi_low_t *event = (wifi_event_bss_rssi_low_t*) event_data;

	ESP_LOGI(TAG, "%s:bss rssi is=%d", __func__, event->rssi);
	/* Lets check channel conditions */
	rrm_ctx++;
	if (esp_rrm_send_neighbor_rep_request(neighbor_report_recv_cb, &rrm_ctx) < 0) {
		/* failed to send neighbor report request */
		ESP_LOGI(TAG, "failed to send neighbor report request");
		if (esp_wnm_send_bss_transition_mgmt_query(REASON_FRAME_LOSS, NULL, 0) < 0) {
			ESP_LOGI(TAG, "failed to send btm query");
		}
	}
}


#endif

//////////////////////////////////
//////////////////////////////////


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

    xTaskCreate(&task_doBlink, "task_doBlink", 4 * 1024, NULL, tskIDLE_PRIORITY+1, NULL);
}
/////////////////////////////////////////////////////////


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        WIFIConnected = false;
        LEDBlinkTask(200, 1, true);
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        WIFIConnected = false;
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

        WIFIConnected = true;
        #ifdef ENABLE_MQTT
            if (getMQTTisEnabled()) {
                vTaskDelay(5000 / portTICK_PERIOD_MS); 
                MQTT_Init();    // Init when WIFI is getting connected    
            }
        #endif //ENABLE_MQTT
    }
}


void strinttoip4(const char *ip, int &a, int &b, int &c, int &d) {
    std::string zw = std::string(ip);
    std::stringstream s(zw);
    char ch; //to temporarily store the '.'
    s >> a >> ch >> b >> ch >> c >> ch >> d;
}


void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname, const char *_ipadr, const char *_gw,  const char *_netmask, const char *_dns, int _rssithreashold)
{
	RSSI_Threshold = _rssithreashold;
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();

    if ((_ipadr != NULL) && (_gw != NULL) && (_netmask != NULL))
    {

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
        ip4_addr_t ip;
        ip.addr = esp_ip4addr_aton(_dns);
        ip_addr_set_ip4_u32(&dns_info.ip, ip.addr);
        ESP_ERROR_CHECK(esp_netif_set_dns_info(my_sta, ESP_NETIF_DNS_MAIN, &dns_info));
    }

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_t instance_bss_rssi_low;
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

	#ifdef WLAN_USE_MESH_ROAMING
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, 
                                                        WIFI_EVENT_STA_BSS_RSSI_LOW,
                                                        &esp_bss_rssi_low_handler, 
                                                        NULL,
                                                        &instance_bss_rssi_low));
	#endif

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
            ESP_LOGE(TAG,"Failed to set hostname: %d",ret);  
        }
        else {
            ESP_LOGI(TAG,"Set hostname to: %s", _hostname); 
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
            ESP_LOGI(TAG, "Connected with AP: %s, password: XXXXXXX", _ssid);
        #else
            ESP_LOGI(TAG, "Connected with AP: %s, password: %s", _ssid, _password);
        #endif        
    } else if (bits & WIFI_FAIL_BIT) {
        #ifdef __HIDE_PASSWORD
            ESP_LOGI(TAG, "Failed to connect with AP: %s, password: XXXXXXXX", _ssid);
        #else
            ESP_LOGI(TAG, "Failed to connect with AP: %s, password: %s", _ssid, _password);
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


void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname)
{
    wifi_init_sta(_ssid, _password, _hostname, NULL, NULL, NULL, NULL, 0);
}


void wifi_init_sta(const char *_ssid, const char *_password)
{
    wifi_init_sta(_ssid, _password, NULL, NULL, NULL, NULL, NULL, 0);
}


bool getWIFIisConnected() 
{
    return WIFIConnected;
}


void WIFIDestroy() 
{	
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler);
	#ifdef WLAN_USE_MESH_ROAMING
	esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_BSS_RSSI_LOW, esp_bss_rssi_low_handler);
	#endif
	
	esp_wifi_stop();
	esp_wifi_deinit();
}

