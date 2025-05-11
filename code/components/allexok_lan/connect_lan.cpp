#if defined(BOARD_ESP32_S3_ALEKSEI)
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

#endif