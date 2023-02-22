#include "connect_wlan.h"

#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wnm.h"
#include "esp_rrm.h"
#include "esp_mbo.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#ifdef ENABLE_MQTT
    #include "interface_mqtt.h"
#endif //ENABLE_MQTT

#include "ClassLogFile.h"
#include "read_wlanini.h"
#include "Helper.h"
#include "statusled.h"

#include "../../include/defines.h"


static const char *TAG = "WIFI";


bool WIFIConnected = false;


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
    return &wlan_config.ipaddress;
}


std::string* getSSID()
{
    return &wlan_config.ssid;
}


static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
	{
        WIFIConnected = false;
        esp_wifi_connect();
    }
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{
		/* Disconnect reason: https://github.com/espressif/esp-idf/blob/d825753387c1a64463779bbd2369e177e5d59a79/components/esp_wifi/include/esp_wifi_types.h */
		wifi_event_sta_disconnected_t *disconn = (wifi_event_sta_disconnected_t *)event_data;
		if (disconn->reason == WIFI_REASON_ROAMING) {
			LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Disconnected (" + std::to_string(disconn->reason) + ", Roaming)");
			// --> no reconnect neccessary, it should automatically reconnect to new AP
		}
		else {
			WIFIConnected = false;
			if (disconn->reason == WIFI_REASON_NO_AP_FOUND) {
				LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Disconnected (" + std::to_string(disconn->reason) + ", No AP)");
				StatusLED(WLAN_CONN, 1, false);
			}
			else if (disconn->reason == WIFI_REASON_AUTH_EXPIRE ||
					 disconn->reason == WIFI_REASON_AUTH_FAIL || 
					 disconn->reason == WIFI_REASON_NOT_AUTHED ||
					 disconn->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT || 
					 disconn->reason == WIFI_REASON_HANDSHAKE_TIMEOUT) {
				LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Disconnected (" + std::to_string(disconn->reason) + ", Auth fail)");
				StatusLED(WLAN_CONN, 2, false);
			}
			else if (disconn->reason == WIFI_REASON_BEACON_TIMEOUT) {
				LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Disconnected (" + std::to_string(disconn->reason) + ", Timeout)");
				StatusLED(WLAN_CONN, 3, false);
			}
			else {
				LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Disconnected (" + std::to_string(disconn->reason) + ")");
				StatusLED(WLAN_CONN, 4, false);
			}
			esp_wifi_connect(); // Try to connect again
		}
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) 
	{
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Connected to: " + wlan_config.ssid + ", RSSI: " + 
												std::to_string(get_WIFI_RSSI()));
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
	{
        WIFIConnected = true;

		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        wlan_config.ipaddress = std::string(ip4addr_ntoa((const ip4_addr*) &event->ip_info.ip));
		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Got IP: " + wlan_config.ipaddress);

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


esp_err_t wifi_init_sta(void)
{
	esp_err_t retval = esp_netif_init();
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_netif_init: Error: "  + std::to_string(retval));
		return retval;
	}

    retval = esp_event_loop_create_default();
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_loop_create_default: Error: "  + std::to_string(retval));
		return retval;
	}
	
    esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();

    if (!wlan_config.ipaddress.empty() && !wlan_config.gateway.empty() && !wlan_config.netmask.empty())
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Manual interface config -> IP: " + wlan_config.ipaddress + ", Gateway: " + 
												std::string(wlan_config.gateway) + ", Netmask: " + std::string(wlan_config.netmask));
		esp_netif_dhcpc_stop(my_sta);	// Stop DHCP service

        esp_netif_ip_info_t ip_info;
        int a, b, c, d;
        strinttoip4(wlan_config.ipaddress.c_str(), a, b, c, d);
        IP4_ADDR(&ip_info.ip, a, b, c, d);	// Set static IP address

        strinttoip4(wlan_config.gateway.c_str(), a, b, c, d);
        IP4_ADDR(&ip_info.gw, a, b, c, d);	// Set gateway

        strinttoip4(wlan_config.netmask.c_str(), a, b, c, d);
        IP4_ADDR(&ip_info.netmask, a, b, c, d);	// Set netmask

        esp_netif_set_ip_info(my_sta, &ip_info);	// Set static IP configuration
    }
	else {
		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Automatic interface config --> Use DHCP service");
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    retval = esp_wifi_init(&cfg);
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_init: Error: "  + std::to_string(retval));
		return retval;
	}

    if (!wlan_config.ipaddress.empty() && !wlan_config.gateway.empty() && !wlan_config.netmask.empty())
    {
        if (wlan_config.dns.empty()) {
			LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No DNS server, use gateway");
			 wlan_config.dns = wlan_config.gateway;
		} 
		else {
			LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Manual interface config -> DNS: " + wlan_config.dns);
		}
     
        esp_netif_dns_info_t dns_info;
        ip4_addr_t ip;
        ip.addr = esp_ip4addr_aton(wlan_config.dns.c_str());
        ip_addr_set_ip4_u32(&dns_info.ip, ip.addr);

        retval = esp_netif_set_dns_info(my_sta, ESP_NETIF_DNS_MAIN, &dns_info);
		if (retval != ESP_OK) {
			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_netif_set_dns_info: Error: "  + std::to_string(retval));
			return retval;
		}
	}

    retval = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &event_handler, NULL, NULL);
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_handler_instance_register - WIFI_ANY: Error: "  + std::to_string(retval));
		return retval;
	}

    retval = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &event_handler, NULL, NULL);
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_handler_instance_register - GOT_IP: Error: "  + std::to_string(retval));
		return retval;
	}

	#ifdef WLAN_USE_MESH_ROAMING
	retval = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_BSS_RSSI_LOW,
                                                        &esp_bss_rssi_low_handler, NULL, NULL);
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_handler_instance_register - BSS_RSSI_LOW: Error: "  + std::to_string(retval));
		return retval;
	}
	#endif

    wifi_config_t wifi_config = { };

    strcpy((char*)wifi_config.sta.ssid, (const char*)wlan_config.ssid.c_str());
    strcpy((char*)wifi_config.sta.password, (const char*)wlan_config.password.c_str());

    retval = esp_wifi_set_mode(WIFI_MODE_STA);
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_set_mode: Error: "  + std::to_string(retval));
		return retval;
	}

    retval = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
	if (retval != ESP_OK) {
		if (retval == ESP_ERR_WIFI_PASSWORD) {
			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_set_config: SSID password invalid! Error: " + std::to_string(retval));
		}
		else {
			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_set_config: Error: "  + std::to_string(retval));
		}
		return retval;
	}

	retval = esp_wifi_start();
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_start: Error: "  + std::to_string(retval));
		return retval;
	}

    if (!wlan_config.hostname.empty())
    {
        retval = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA , wlan_config.hostname.c_str());
        if(retval != ESP_OK ) {
			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to set hostname! Error: " + std::to_string(retval));
        }
        else {
			LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Set hostname to: " + wlan_config.hostname);
        }
    }

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Init successful");
	return ESP_OK;
}


int get_WIFI_RSSI()
{
	wifi_ap_record_t ap;
	if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) 
		return ap.rssi;
	else
		return -127;	// Return -127 if no info available e.g. not connected
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

	esp_wifi_disconnect();
	esp_wifi_stop();
	esp_wifi_deinit();
}

