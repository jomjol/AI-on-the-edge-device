#include "connect_lan.h"

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
#include <netdb.h>
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#ifdef ENABLE_MQTT
    #include "interface_mqtt.h"
#endif //ENABLE_MQTT

#include "ClassLogFile.h"
#include "read_lanini.h"
#include "Helper.h"
#include "statusled.h"

#include "../../include/defines.h"

#if (ESP_IDF_VERSION_MAJOR >= 5)
#include "soc/periph_defs.h"
#include "esp_private/periph_ctrl.h"
#include "soc/gpio_sig_map.h"
#include "soc/gpio_periph.h"
#include "soc/io_mux_reg.h"
#include "esp_rom_gpio.h"
#define gpio_pad_select_gpio esp_rom_gpio_pad_select_gpio
#define gpio_matrix_in(a,b,c) esp_rom_gpio_connect_in_signal(a,b,c)
#define gpio_matrix_out(a,b,c,d) esp_rom_gpio_connect_out_signal(a,b,c,d)
#define ets_delay_us(a) esp_rom_delay_us(a)
#endif

#include "../esp-protocols/components/mdns/include/mdns.h" 

#include <driver/spi_master.h>
#include <esp_eth.h>
#include <esp_netif.h>

static const char *TAG = "LAN";

extern bool WIFIConnected;
static int LanReconnectCnt = 0;


// #ifdef WLAN_USE_MESH_ROAMING
// /* rrm ctx */
// int rrm_ctx = 0;

// static inline uint32_t WPA_GET_LE32(const uint8_t *a)
// {
// 	return ((uint32_t) a[3] << 24) | (a[2] << 16) | (a[1] << 8) | a[0];
// }


// #ifndef WLAN_EID_MEASURE_REPORT
// #define WLAN_EID_MEASURE_REPORT 39
// #endif
// #ifndef MEASURE_TYPE_LCI
// #define MEASURE_TYPE_LCI 9
// #endif
// #ifndef MEASURE_TYPE_LOCATION_CIVIC
// #define MEASURE_TYPE_LOCATION_CIVIC 11
// #endif
// #ifndef WLAN_EID_NEIGHBOR_REPORT
// #define WLAN_EID_NEIGHBOR_REPORT 52
// #endif
// #ifndef ETH_ALEN
// #define ETH_ALEN 6
// #endif


// #define MAX_NEIGHBOR_LEN 512
// static char * get_btm_neighbor_list(uint8_t *report, size_t report_len)
// {
// 	size_t len = 0;
// 	const uint8_t *data;
// 	int ret = 0;

// 	/*
// 	 * Neighbor Report element (IEEE P802.11-REVmc/D5.0)
// 	 * BSSID[6]
// 	 * BSSID Information[4]
// 	 * Operating Class[1]
// 	 * Channel Number[1]
// 	 * PHY Type[1]
// 	 * Optional Subelements[variable]
// 	 */
// 	#define NR_IE_MIN_LEN (ETH_ALEN + 4 + 1 + 1 + 1)

// 	if (!report || report_len == 0) {
// 		ESP_LOGD(TAG, "Roaming: RRM neighbor report is not valid");
// 		return NULL;
// 	}

// 	char *buf = (char*) calloc(1, MAX_NEIGHBOR_LEN);
// 	data = report;

// 	while (report_len >= 2 + NR_IE_MIN_LEN) {
// 		const uint8_t *nr;
// 		char lci[256 * 2 + 1];
// 		char civic[256 * 2 + 1];
// 		uint8_t nr_len = data[1];
// 		const uint8_t *pos = data, *end;

// 		if (pos[0] != WLAN_EID_NEIGHBOR_REPORT ||
// 		    nr_len < NR_IE_MIN_LEN) {
// 			ESP_LOGD(TAG, "Roaming CTRL: Invalid Neighbor Report element: id=%u len=%u",
// 					data[0], nr_len);
// 			ret = -1;
// 			goto cleanup;
// 		}

// 		if (2U + nr_len > report_len) {
// 			ESP_LOGD(TAG, "Roaming CTRL: Invalid Neighbor Report element: id=%u len=%zu nr_len=%u",
// 					data[0], report_len, nr_len);
// 			ret = -1;
// 			goto cleanup;
// 		}
// 		pos += 2;
// 		end = pos + nr_len;

// 		nr = pos;
// 		pos += NR_IE_MIN_LEN;

// 		lci[0] = '\0';
// 		civic[0] = '\0';
// 		while (end - pos > 2) {
// 			uint8_t s_id, s_len;

// 			s_id = *pos++;
// 			s_len = *pos++;
// 			if (s_len > end - pos) {
// 				ret = -1;
// 				goto cleanup;
// 			}
// 			if (s_id == WLAN_EID_MEASURE_REPORT && s_len > 3) {
// 				/* Measurement Token[1] */
// 				/* Measurement Report Mode[1] */
// 				/* Measurement Type[1] */
// 				/* Measurement Report[variable] */
// 				switch (pos[2]) {
// 					case MEASURE_TYPE_LCI:
// 						if (lci[0])
// 							break;
// 						memcpy(lci, pos, s_len);
// 						break;
// 					case MEASURE_TYPE_LOCATION_CIVIC:
// 						if (civic[0])
// 							break;
// 						memcpy(civic, pos, s_len);
// 						break;
// 				}
// 			}

// 			pos += s_len;
// 		}
				
// 		ESP_LOGI(TAG, "Roaming: RMM neighbor report bssid=" MACSTR
// 				" info=0x%x op_class=%u chan=%u phy_type=%u%s%s%s%s",
// 				MAC2STR(nr), WPA_GET_LE32(nr + ETH_ALEN),
// 				nr[ETH_ALEN + 4], nr[ETH_ALEN + 5],
// 				nr[ETH_ALEN + 6],
// 				lci[0] ? " lci=" : "", lci,
// 				civic[0] ? " civic=" : "", civic);

		
// 		LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: RMM neighbor report BSSID: " + BssidToString((char*)nr) + 
// 		                                        ", Channel: " + std::to_string(nr[ETH_ALEN + 5]));

// 		/* neighbor start */
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, " neighbor=");
// 		/* bssid */
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, MACSTR, MAC2STR(nr));
// 		/* , */
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
// 		/* bssid info */
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "0x%04x", WPA_GET_LE32(nr + ETH_ALEN));
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
// 		/* operating class */
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "%u", nr[ETH_ALEN + 4]);
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
// 		/* channel number */
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "%u", nr[ETH_ALEN + 5]);
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, ",");
// 		/* phy type */
// 		len += snprintf(buf + len, MAX_NEIGHBOR_LEN - len, "%u", nr[ETH_ALEN + 6]);
// 		/* optional elements, skip */

// 		data = end;
// 		report_len -= 2 + nr_len;
// 	}

// cleanup:
// 	if (ret < 0) {
// 		free(buf);
// 		buf = NULL;
// 	}
// 	return buf;
// }


// void neighbor_report_recv_cb(void *ctx, const uint8_t *report, size_t report_len)
// {
// 	int *val = (int*) ctx;
// 	uint8_t *pos = (uint8_t *)report;
// 	int cand_list = 0;
// 	int ret;

// 	if (!report) {
// 		ESP_LOGD(TAG, "Roaming: Neighbor report is null");
// 		return;
// 	}
// 	if (*val != rrm_ctx) {
// 		ESP_LOGE(TAG, "Roaming: rrm_ctx value didn't match, not initiated by us");
// 		return;
// 	}
// 	/* dump report info */
// 	ESP_LOGD(TAG, "Roaming: RRM neighbor report len=%d", report_len);
// 	ESP_LOG_BUFFER_HEXDUMP(TAG, pos, report_len, ESP_LOG_DEBUG);

// 	/* create neighbor list */
// 	char *neighbor_list = get_btm_neighbor_list(pos + 1, report_len - 1);

// 	/* In case neighbor list is not present issue a scan and get the list from that */
// 	if (!neighbor_list) {
// 		/* issue scan */
// 		wifi_scan_config_t params;
// 		memset(&params, 0, sizeof(wifi_scan_config_t));
// 		if (esp_wifi_scan_start(&params, true) < 0) {
// 			goto cleanup;
// 		}
// 		/* cleanup from net802.11 */
// 		uint16_t number = 1;
// 		wifi_ap_record_t ap_records;
// 		esp_wifi_scan_get_ap_records(&number, &ap_records);
// 		cand_list = 1;
// 	}
// 	/* send AP btm query requesting to roam depending on candidate list of AP */
// 	// btm_query_reasons: https://github.com/espressif/esp-idf/blob/release/v4.4/components/wpa_supplicant/esp_supplicant/include/esp_wnm.h
// 	ret = esp_wnm_send_bss_transition_mgmt_query(REASON_FRAME_LOSS, neighbor_list, cand_list);	// query reason 16 -> LOW RSSI --> (btm_query_reason)16
// 	ESP_LOGD(TAG, "neighbor_report_recv_cb retval - bss_transisition_query: %d", ret);

// cleanup:
// 	if (neighbor_list)
// 		free(neighbor_list);
// }


// static void esp_bss_rssi_low_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// {
// 	int retval = -1;
// 	wifi_event_bss_rssi_low_t *event = (wifi_event_bss_rssi_low_t*) event_data;

// 	LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming Event: RSSI " + std::to_string(event->rssi) + 
// 								" < RSSI_Threshold " + std::to_string(wlan_config.rssi_threshold));

// 	/* If RRM is supported, call RRM and then send BTM query to AP */
// 	if (esp_rrm_is_rrm_supported_connection() && esp_wnm_is_btm_supported_connection()) 
// 	{
// 		/* Lets check channel conditions */
// 		rrm_ctx++;

// 		retval = esp_rrm_send_neighbor_rep_request(neighbor_report_recv_cb, &rrm_ctx);
// 		ESP_LOGD(TAG, "esp_rrm_send_neighbor_rep_request retval: %d", retval);
// 		if (retval == 0)
// 			LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: RRM + BTM query sent");
// 		else
// 			ESP_LOGD(TAG, "esp_rrm_send_neighbor_rep_request retval: %d", retval);
// 	}

// 	/* If RRM is not supported or RRM request failed, send directly BTM query to AP */
// 	if (retval < 0 && esp_wnm_is_btm_supported_connection()) 
// 	{
// 		// btm_query_reasons: https://github.com/espressif/esp-idf/blob/release/v4.4/components/wpa_supplicant/esp_supplicant/include/esp_wnm.h
// 		retval = esp_wnm_send_bss_transition_mgmt_query(REASON_FRAME_LOSS, NULL, 0);	// query reason 16 -> LOW RSSI --> (btm_query_reason)16
// 		ESP_LOGD(TAG, "esp_wnm_send_bss_transition_mgmt_query retval: %d", retval);
// 		if (retval == 0)
// 			LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: BTM query sent");
// 		else
// 			ESP_LOGD(TAG, "esp_wnm_send_bss_transition_mgmt_query retval: %d", retval);
// 	}
// }


// void printRoamingFeatureSupport(void) 
// {
// 	if (esp_rrm_is_rrm_supported_connection())
// 		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Roaming: RRM (802.11k) supported by AP");
// 	else
// 		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Roaming: RRM (802.11k) NOT supported by AP");

// 	if (esp_wnm_is_btm_supported_connection())
// 		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Roaming: BTM (802.11v) supported by AP");
// 	else
// 		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Roaming: BTM (802.11v) NOT supported by AP");
// }


// #ifdef WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES
// void wifiRoamingQuery(void) 
// {
// 	/* Query only if WIFI is connected and feature is supported by AP */
// 	if (LanConnected && (esp_rrm_is_rrm_supported_connection() || esp_wnm_is_btm_supported_connection())) {
// 		/* Client is allowed to send query to AP for roaming request if RSSI is lower than threshold */
// 		/* Note 1: Set RSSI threshold funtion needs to be called to trigger WIFI_EVENT_STA_BSS_RSSI_LOW */
// 		/* Note 2: Additional querys will be sent after flow round is finshed --> server_tflite.cpp - function "task_autodoFlow" */
// 		/* Note 3: RSSI_Threshold = 0 --> Disable client query by application (WebUI parameter) */
		
// 		if (wlan_config.rssi_threshold != 0 && get_WIFI_RSSI() != -127 && (get_WIFI_RSSI() < wlan_config.rssi_threshold))
// 			esp_wifi_set_rssi_threshold(wlan_config.rssi_threshold);
// 	}
// }
// #endif // WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES
// #endif // WLAN_USE_MESH_ROAMING


// #ifdef WLAN_USE_ROAMING_BY_SCANNING
// std::string getAuthModeName(const wifi_auth_mode_t auth_mode)
// {
// 	std::string AuthModeNames[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "WPA2 ENTERPRISE",
//                                    "WPA3 PSK", "WPA2 WPA3 PSK", "WAPI_PSK", "MAX"};
//     return AuthModeNames[auth_mode];
// }


// void wifi_scan(void)
// {
//     wifi_scan_config_t wifi_scan_config;
//     memset(&wifi_scan_config, 0, sizeof(wifi_scan_config));

//     wifi_scan_config.ssid = (uint8_t*)wlan_config.ssid.c_str(); // only scan for configured SSID
//     wifi_scan_config.show_hidden = true;            // scan also hidden SSIDs
// 	wifi_scan_config.channel = 0;                   // scan all channels

//     esp_wifi_scan_start(&wifi_scan_config, true);   // not using event handler SCAN_DONE by purpose to keep SYS_EVENT heap smaller 
//                                                     // and the calling task task_autodoFlow is after scan is finish in wait state anyway
//                                                     // Scan duration: ca. (120ms + 30ms) * Number of channels -> ca. 1,5 - 2s

//     uint16_t max_number_of_ap_found = 10;           // max. number of APs, value will be updated by function "esp_wifi_scan_get_ap_num"
// 	esp_wifi_scan_get_ap_num(&max_number_of_ap_found); // get actual found APs
//     wifi_ap_record_t* wifi_ap_records = new wifi_ap_record_t[max_number_of_ap_found]; // Allocate necessary record datasets
// 	if (wifi_ap_records == NULL) {
// 		esp_wifi_scan_get_ap_records(0, NULL); // free internal heap
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "wifi_scan: Failed to allocate heap for wifi_ap_records");
// 		return;
// 	}
// 	else {
//     	if (esp_wifi_scan_get_ap_records(&max_number_of_ap_found, wifi_ap_records) != ESP_OK) { // Retrieve results (and free internal heap)
// 			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "wifi_scan: esp_wifi_scan_get_ap_records: Error retrieving datasets");
// 			delete[] wifi_ap_records;
// 			return;
// 		}
// 	}

// 	wifi_ap_record_t currentAP;
// 	esp_wifi_sta_get_ap_info(&currentAP);

// 	LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: Current AP BSSID=" + BssidToString((char*)currentAP.bssid));
//     LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: Scan completed, APs found with configured SSID: " + std::to_string(max_number_of_ap_found));
//     for (int i = 0; i < max_number_of_ap_found; i++) {
//         LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: " + std::to_string(i+1) +
//                                                 ": SSID=" + std::string((char*)wifi_ap_records[i].ssid) +
//                                                 ", BSSID=" + BssidToString((char*)wifi_ap_records[i].bssid) + 
//                                                 ", RSSI=" + std::to_string(wifi_ap_records[i].rssi) + 
//                                                 ", CH=" + std::to_string(wifi_ap_records[i].primary) + 
//                                                 ", AUTH=" + getAuthModeName(wifi_ap_records[i].authmode));
// 		if (wifi_ap_records[i].rssi > (currentAP.rssi + 5) && // RSSI is better than actual RSSI + 5 --> Avoid switching to AP with roughly same RSSI
//            (strcmp(BssidToString((char*)wifi_ap_records[i].bssid).c_str(), BssidToString((char*)currentAP.bssid).c_str()) != 0))
//         {
// 			APWithBetterRSSI = true;
//         }
// 	}
// 	delete[] wifi_ap_records;
// }


// void wifiRoamByScanning(void)
// {
// 	if (wlan_config.rssi_threshold != 0 && get_WIFI_RSSI() != -127 && (get_WIFI_RSSI() < wlan_config.rssi_threshold)) {
// 		LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: Start scan of all channels for SSID " + wlan_config.ssid);
// 		wifi_scan();

// 		if (APWithBetterRSSI) {
// 			APWithBetterRSSI = false;
// 			LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Roaming: AP with better RSSI in range, disconnecting to switch AP...");
// 			esp_wifi_disconnect();
// 		} 
// 		else {
// 			LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Roaming: Scan completed, stay on current AP");
// 		}
// 	}
// }
// #endif // WLAN_USE_ROAMING_BY_SCANNING


std::string* getLanIPAddress()
{
    return &wlan_config.ipaddress;
}


static void eth_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
	{
        WIFIConnected = false;
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Ethernet Started");
    }
	else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_DISCONNECTED) 
	{
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Ethernet Link Down");
        // Optionally, try to reconnect or handle fallback LED:
		StatusLED(WLAN_CONN, 1, false);
        LanReconnectCnt++;
		WIFIConnected = false;
		}	
	else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_CONNECTED) 
	{
		uint8_t mac_addr[6] = {0};
		esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;
		
		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Ethernet Link Up");
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	}	
    else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_STOP) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Ethernet Stopped");
        WIFIConnected = false;
		}
}

static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data)
{
    WIFIConnected = true;
	LanReconnectCnt = 0;
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    wlan_config.ipaddress = std::string(ip4addr_ntoa((const ip4_addr*) &event->ip_info.ip));
	LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Assigned IP: " + wlan_config.ipaddress);
	#ifdef ENABLE_MQTT
        if (getMQTTisEnabled()) {
            vTaskDelay(5000 / portTICK_PERIOD_MS); 
            MQTT_Init();    // Init when WIFI is getting connected    
        }
    #endif //ENABLE_MQTT   
	// LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Assigned IP: " + WIFIConnected);
	// LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Assigned IP: " + getWIFIisConnected());
}


esp_eth_handle_t eth_handle = NULL;
esp_netif_t *eth_netif = NULL;

int lan_init(void)
{
	esp_err_t retval = esp_netif_init();
	if (retval != ESP_OK) {
		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_netif_init: Error: "  + std::to_string(retval));
		return retval;
	}
    int retVal = esp_event_loop_create_default();
    if (retVal != ESP_OK && retVal != ESP_ERR_INVALID_STATE) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_loop_create_default, Error");
        return retVal;
    }

    gpio_set_direction(ETH_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_level(ETH_ENABLE, 1);

    gpio_set_direction(ETH_INT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ETH_INT, GPIO_PULLUP_ONLY);

    esp_log_level_set("netif", ESP_LOG_DEBUG);
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "SPI init");
    // 1) SPI bus init
    spi_bus_config_t buscfg = { 0 };
    buscfg.mosi_io_num = ETH_MOSI;
    buscfg.miso_io_num = ETH_MISO;
    buscfg.sclk_io_num = ETH_CLK;

    ESP_ERROR_CHECK(spi_bus_initialize(W5500_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // 2) Prepare a `spi_device_interface_config_t` but DO NOT call spi_bus_add_device manually
    spi_device_interface_config_t devcfg = {
        .mode         = 0,  // SPI mode 0
        .clock_speed_hz = 40 * 1000 * 1000, // 20MHz
        .spics_io_num = ETH_CS,
        .queue_size   = 30,
        // the rest zero-initialized
    };

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(W5500_SPI_HOST, &devcfg);
    w5500_config.int_gpio_num = ETH_INT;
    // 4) Standard MAC/PHY config
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1; // typical W5500
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Driver install");

    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Driver installed");

    uint8_t base_mac_addr[6];
    esp_err_t ret = esp_efuse_mac_get_default(base_mac_addr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get efuse base MAC, error=0x%x", ret);
    }
    uint8_t local_mac[6];
    esp_derive_local_mac(local_mac, base_mac_addr);
    ret = esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, local_mac);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set W5500 MAC, error=0x%x", ret);
    }

    // 5) Attach netif + start
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_cfg);

        // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(
                        ETH_EVENT, ESP_EVENT_ANY_ID,
                        &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(
                        IP_EVENT, IP_EVENT_ETH_GOT_IP,
                        &got_ip_event_handler, NULL));

    esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle));
    esp_eth_start(eth_handle);
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "W5500 Ethernet init done");

    return ESP_OK;
}

// esp_err_t wifi_init_sta(void)
// {
// 	esp_err_t retval = esp_netif_init();
// 	if (retval != ESP_OK) {
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_netif_init: Error: "  + std::to_string(retval));
// 		return retval;
// 	}

//     retval = esp_event_loop_create_default();
// 	if (retval != ESP_OK) {
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_loop_create_default: Error: "  + std::to_string(retval));
// 		return retval;
// 	}
	
//     my_sta = esp_netif_create_default_wifi_sta();

//     if (!wlan_config.ipaddress.empty() && !wlan_config.gateway.empty() && !wlan_config.netmask.empty())
//     {
//         LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Manual interface config -> IP: " + wlan_config.ipaddress + ", Gateway: " + 
// 												std::string(wlan_config.gateway) + ", Netmask: " + std::string(wlan_config.netmask));
// 		esp_netif_dhcpc_stop(my_sta);	// Stop DHCP service

//         esp_netif_ip_info_t ip_info;
//         int a, b, c, d;
//         strinttoip4(wlan_config.ipaddress.c_str(), a, b, c, d);
//         IP4_ADDR(&ip_info.ip, a, b, c, d);	// Set static IP address

//         strinttoip4(wlan_config.gateway.c_str(), a, b, c, d);
//         IP4_ADDR(&ip_info.gw, a, b, c, d);	// Set gateway

//         strinttoip4(wlan_config.netmask.c_str(), a, b, c, d);
//         IP4_ADDR(&ip_info.netmask, a, b, c, d);	// Set netmask

//         esp_netif_set_ip_info(my_sta, &ip_info);	// Set static IP configuration
//     }
// 	else {
// 		LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Automatic interface config --> Use DHCP service");
// 	}

// 	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     retval = esp_wifi_init(&cfg);
// 	if (retval != ESP_OK) {
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_init: Error: "  + std::to_string(retval));
// 		return retval;
// 	}

//     if (!wlan_config.ipaddress.empty() && !wlan_config.gateway.empty() && !wlan_config.netmask.empty())
//     {
//         if (wlan_config.dns.empty()) {
// 			LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No DNS server, use gateway");
// 			 wlan_config.dns = wlan_config.gateway;
// 		} 
// 		else {
// 			LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Manual interface config -> DNS: " + wlan_config.dns);
// 		}
     
//         esp_netif_dns_info_t dns_info;
//         ip4_addr_t ip;
//         ip.addr = esp_ip4addr_aton(wlan_config.dns.c_str());
//         ip_addr_set_ip4_u32(&dns_info.ip, ip.addr);

//         retval = esp_netif_set_dns_info(my_sta, ESP_NETIF_DNS_MAIN, &dns_info);
// 		if (retval != ESP_OK) {
// 			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_netif_set_dns_info: Error: "  + std::to_string(retval));
// 			return retval;
// 		}
// 	}

//     retval = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
//                                                         &event_handler, NULL, NULL);
// 	if (retval != ESP_OK) {
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_handler_instance_register - WIFI_ANY: Error: "  + std::to_string(retval));
// 		return retval;
// 	}

//     retval = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
//                                                         &event_handler, NULL, NULL);
// 	if (retval != ESP_OK) {
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_handler_instance_register - GOT_IP: Error: "  + std::to_string(retval));
// 		return retval;
// 	}

//     wifi_config_t wifi_config = { };

// 	wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;		// Scan all channels instead of stopping after first match
// 	wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;	// Sort by signal strength and keep up to 4 best APs
// 	//wifi_config.sta.failure_retry_cnt = 3;					// IDF version 5.0 will support this

// 	#ifdef WLAN_USE_MESH_ROAMING
// 	wifi_config.sta.rm_enabled = 1;		 // 802.11k (Radio Resource Management)
// 	wifi_config.sta.btm_enabled = 1;	 // 802.11v (BSS Transition Management)
// 	//wifi_config.sta.mbo_enabled = 1;	 // Multiband Operation (better use of Wi-Fi network resources in roaming decisions) -> not activated to save heap
// 	wifi_config.sta.pmf_cfg.capable = 1; // 802.11w (Protected Management Frame, activated by default if other device also advertizes PMF capability)
// 	//wifi_config.sta.ft_enabled = 1;	 // 802.11r (BSS Fast Transition) -> Upcoming IDF version 5.0 will support 11r
// 	#endif

//     // strcpy((char*)wifi_config.sta.ssid, (const char*)wlan_config.ssid.c_str());
//     strcpy((char*)wifi_config.sta.password, (const char*)wlan_config.password.c_str());

//     retval = esp_wifi_set_mode(WIFI_MODE_STA);
// 	if (retval != ESP_OK) {
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_set_mode: Error: "  + std::to_string(retval));
// 		return retval;
// 	}

//     retval = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
// 	if (retval != ESP_OK) {
// 		if (retval == ESP_ERR_WIFI_PASSWORD) {
// 			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_set_config: SSID password invalid! Error: " + std::to_string(retval));
// 		}
// 		else {
// 			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_set_config: Error: "  + std::to_string(retval));
// 		}
// 		return retval;
// 	}

// 	retval = esp_wifi_start();
// 	if (retval != ESP_OK) {
// 		LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_wifi_start: Error: "  + std::to_string(retval));
// 		return retval;
// 	}

//     if (!wlan_config.hostname.empty())
//     {
//         retval = esp_netif_set_hostname(my_sta, wlan_config.hostname.c_str());
//         if(retval != ESP_OK ) {
// 			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to set hostname! Error: " + std::to_string(retval));
//         }
//         else {
// 			LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Set hostname to: " + wlan_config.hostname);
//         }
// 		//initialize mDNS service
//         retval = mdns_init();
//         if (retval != ESP_OK) {
//             LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "mdns_init failed! Error: "  + std::to_string(retval));
//         } else {
// 			//set mdns hostname
// 			mdns_hostname_set(wlan_config.hostname.c_str());
// 	    }
//     }

//     LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Init successful");
// 	return ESP_OK;
// }


// int get_WIFI_RSSI()
// {
// 	wifi_ap_record_t ap;
// 	if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) 
// 		return ap.rssi;
// 	else
// 		return -127;	// Return -127 if no info available e.g. not connected
// }


/*std::string getIp() {
	esp_netif_ip_info_t ip_info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(my_sta, ip_info));
	char ipFormated[4*3+3+1];
    sprintf(ipFormated, IPSTR, IP2STR(&ip_info.ip));
	return std::string(ipFormated);
}*/

void LanDestroy() 
{	
	// esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler);
    // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler);

	// esp_wifi_disconnect();
	// esp_wifi_stop();
	// esp_wifi_deinit();
}

