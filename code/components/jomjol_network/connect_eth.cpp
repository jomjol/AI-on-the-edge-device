/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 *
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_eth.html#basic-ethernet-concepts
 */
#include "sdkconfig.h"

#if (CONFIG_ETH_ENABLED && CONFIG_ETH_USE_SPI_ETHERNET && CONFIG_ETH_SPI_ETHERNET_W5500)

#include "Helper.h"
#include "defines.h"

#include "connect_eth.h"

#include <string>
#include <esp_event.h>
#include <netdb.h>
#include <esp_system.h>
#include <esp_wnm.h>
#include <esp_rrm.h>
#include <esp_mbo.h>
#include <esp_mac.h>
#include <esp_log.h>
#include <esp_eth.h>
#include <esp_netif.h>
#include <esp_netif_sntp.h>

#include "driver/gpio.h"
#include <driver/spi_master.h>

// define `gpio_pad_select_gpip` for newer versions of IDF
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0))
#include "esp_rom_gpio.h"
#define gpio_pad_select_gpio esp_rom_gpio_pad_select_gpio
#endif

#include "time_sntp.h"
#include "ClassLogFile.h"

#include "statusled.h"
#include "read_network_config.h"

#ifdef ENABLE_MQTT
#include "interface_mqtt.h"
#endif // ENABLE_MQTT

static const char *TAG = "ETH";

static bool gpio_isr_svc_init_by_eth = false; // indicates that we initialized the GPIO ISR service

esp_eth_handle_t my_w5500_handle = NULL;
esp_netif_t *my_w5500_netif = NULL;
esp_eth_netif_glue_handle_t my_w5500_netif_glue = NULL;

static bool ETHInitialized = false;
static bool ETHConnected = false;
static bool ETHConnectionSuccessful = false;
static int ETHReconnectCnt = 0;

/**
 * @brief SPI bus initialization (to be used by Ethernet SPI modules)
 *
 * @return
 *          - ESP_OK on success
 */
static esp_err_t spi_bus_init(void)
{
    esp_err_t retVal = ESP_OK;

    // Configure IO Pad as General Purpose IO,
    // so that it can be connected to internal Matrix,
    // then combined with one or more peripheral signals.
    gpio_pad_select_gpio(ETH_SPI_EN);

    ESP_ERROR_CHECK(gpio_set_direction(ETH_SPI_EN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(ETH_SPI_EN, 1));

    vTaskDelay(pdMS_TO_TICKS(500));

    if (ETH_SPI_INT0_GPIO != GPIO_NUM_NC)
    {
        // Install GPIO ISR handler to be able to service SPI Eth modules interrupts
        ESP_LOGI(TAG, "spi_bus_init(): Install GPIO ISR handler...");
        retVal = gpio_install_isr_service(0);

        if (retVal == ESP_OK)
        {
            gpio_isr_svc_init_by_eth = true;
            ESP_LOGI(TAG, "spi_bus_init(): GPIO ISR handler install successful");
        }
        else if (retVal == ESP_ERR_INVALID_STATE)
        {
            ESP_LOGW(TAG, "spi_bus_init(): GPIO ISR handler has been already installed");
            retVal = ESP_OK; // ISR handler has been already installed so no issues
        }
        else
        {
            ESP_LOGE(TAG, "spi_bus_init(): GPIO ISR handler install failed");
            return retVal;
        }
    }

    // Init SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = ETH_SPI_MOSI_GPIO,
        .miso_io_num = ETH_SPI_MISO_GPIO,
        .sclk_io_num = ETH_SPI_SCLK_GPIO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
    };

    retVal = spi_bus_initialize(ETH_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_init(): SPI host #%d init failed", ETH_SPI_HOST);
    }

    return retVal;
}

static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_START)
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Ethernet Started");
        // Typically nothing special here
    }
    else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_STOP)
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Ethernet Stopped");
        ETHConnected = false;
    }
    else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_CONNECTED)
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Ethernet Link Up");

        uint8_t mac_addr[6] = {0};
        /* we can get the ethernet driver handle from event data */
        esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

        ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr));
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        ETHConnected = true; // Not IP-ready until IP_EVENT_ETH_GOT_IP
    }
    else if (event_base == ETH_EVENT && event_id == ETHERNET_EVENT_DISCONNECTED)
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Ethernet Link Down");
        ETHConnected = false;
        // Optionally, try to reconnect or handle fallback LED:
        StatusLED(WLAN_CONN, 1, false);
        ETHReconnectCnt++;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_ETH_GOT_IP)
    {
        // We have a valid IP
        ETHConnectionSuccessful = true;
        ETHConnected = true;
        ETHReconnectCnt = 0;

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        network_config.ipaddress = std::string(ip4addr_ntoa((const ip4_addr *)&event->ip_info.ip));
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Assigned IP: " + network_config.ipaddress);

        network_config.netmask = std::string(ip4addr_ntoa((const ip4_addr *)&event->ip_info.netmask));
        network_config.gateway = std::string(ip4addr_ntoa((const ip4_addr *)&event->ip_info.gw));

        esp_netif_dns_info_t dnsInfo;
        ESP_ERROR_CHECK(esp_netif_get_dns_info(event->esp_netif, ESP_NETIF_DNS_MAIN, &dnsInfo));
        network_config.dns = std::string(ip4addr_ntoa((const ip4_addr *)&dnsInfo.ip));

        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Assigned IP: " + network_config.ipaddress + ", Subnet: " + network_config.netmask + ", Gateway: " + network_config.gateway + ", DNS: " + network_config.dns);

#ifdef ENABLE_MQTT
        if (getMQTTisEnabled())
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            MQTT_Init(); // Init when Ethernet is getting connected
        }
#endif // ENABLE_MQTT

        // Optionally set a status LED
        StatusLED(WLAN_CONN, 0, true); // e.g., 0 means "ok"
    }
}

esp_err_t eth_init_W5500(void)
{
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "W5500 Ethernet init...");

    // Set log level for netif component to WARN level (default: INFO; only relevant for serial console)
    // ********************************************
    esp_log_level_set("netif", ESP_LOG_WARN);

    esp_err_t retVal = esp_netif_init();
    if (retVal != ESP_OK)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_netif_init: Error: " + std::to_string(retVal));
        return retVal;
    }

    retVal = esp_event_loop_create_default();
    if (retVal != ESP_OK)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_event_loop_create_default: Error: " + std::to_string(retVal));
        return retVal;
    }

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "SPI init");
    retVal = spi_bus_init();
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to init spi bus, error=0x%x", retVal);
        return retVal;
    }

    spi_device_interface_config_t devcfg = {
        .mode = 0, // SPI mode 0
        .clock_speed_hz = ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = ETH_SPI_CS0_GPIO,
        .queue_size = 30,
    };

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(ETH_SPI_HOST, &devcfg);
    // Interrupt GPIO number, set -1 to not use interrupt and to poll rx status periodically
    w5500_config.int_gpio_num = ETH_SPI_INT0_GPIO;
    // Period in ms to poll rx status when interrupt mode is not used
    if (gpio_isr_svc_init_by_eth == false)
    {
        w5500_config.poll_period_ms = ETH_SPI_POLLING0_MS;
    }

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);

    // Update PHY config based on board specific configuration
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG(); // apply default PHY configuration
    phy_config.phy_addr = ETH_SPI_PHY_ADDR0;                // alter the PHY address
    phy_config.reset_gpio_num = ETH_SPI_PHY_RST0_GPIO;      // alter the GPIO used for PHY reset
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Driver install...");
    retVal = esp_eth_driver_install(&eth_config, &my_w5500_handle);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install Ethernet driver, error=0x%x", retVal);
        return retVal;
    }
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Driver installed");

    uint8_t base_mac_addr[6];
    retVal = esp_efuse_mac_get_default(base_mac_addr);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get efuse base MAC, error=0x%x", retVal);
        return retVal;
    }

    uint8_t local_mac[6];
    retVal = esp_derive_local_mac(local_mac, base_mac_addr);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to derive local MAC address from universal MAC address, error=0x%x", retVal);
        return retVal;
    }

    retVal = esp_eth_ioctl(my_w5500_handle, ETH_CMD_S_MAC_ADDR, local_mac);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set W5500 MAC, error=0x%x", retVal);
        return retVal;
    }

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    my_w5500_netif = esp_netif_new(&netif_cfg);
    my_w5500_netif_glue = esp_eth_new_netif_glue(my_w5500_handle);

    if (!network_config.ipaddress.empty() && !network_config.gateway.empty() && !network_config.netmask.empty())
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Manual interface config -> IP: " + network_config.ipaddress + ", Gateway: " + std::string(network_config.gateway) + ", Netmask: " + std::string(network_config.netmask));
        esp_netif_dhcpc_stop(my_w5500_netif); // Stop DHCP service

        esp_netif_ip_info_t ip_info;
        int a, b, c, d;
        strinttoip4(network_config.ipaddress.c_str(), a, b, c, d);
        IP4_ADDR(&ip_info.ip, a, b, c, d); // Set static IP address

        strinttoip4(network_config.gateway.c_str(), a, b, c, d);
        IP4_ADDR(&ip_info.gw, a, b, c, d); // Set gateway

        strinttoip4(network_config.netmask.c_str(), a, b, c, d);
        IP4_ADDR(&ip_info.netmask, a, b, c, d); // Set netmask

        esp_netif_set_ip_info(my_w5500_netif, &ip_info); // Set static IP configuration

        if (network_config.dns.empty())
        {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No DNS server, use gateway");
            network_config.dns = network_config.gateway;
        }
        else
        {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Manual interface config -> DNS: " + network_config.dns);
        }

        esp_netif_dns_info_t dns_info;
        ip4_addr_t ip;
        ip.addr = esp_ip4addr_aton(network_config.dns.c_str());
        ip_addr_set_ip4_u32(&dns_info.ip, ip.addr);

        retVal = esp_netif_set_dns_info(my_w5500_netif, ESP_NETIF_DNS_MAIN, &dns_info);
        if (retVal != ESP_OK)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "esp_netif_set_dns_info: Error: " + std::to_string(retVal));
            return retVal;
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Automatic interface config --> Use DHCP service");
    }

    // Register an event handler to the system event loop (legacy).
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &eth_event_handler, NULL));

    retVal = esp_netif_attach(my_w5500_netif, my_w5500_netif_glue);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to attaches esp_netif instance to the io driver handle, error=0x%x", retVal);
        return retVal;
    }

    retVal = esp_eth_start(my_w5500_handle);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start Ethernet driver, error=0x%x", retVal);
        return retVal;
    }

    ETHInitialized = true;

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "W5500 Ethernet init successful done");
    return ESP_OK;
}

void eth_deinit_W5500(void)
{
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "W5500 Ethernet deinit...");

    esp_err_t retVal = ESP_OK;
    ETHInitialized = false;

    ESP_LOGD(TAG, "esp_eth_stop(my_w5500_handle)");
    ESP_ERROR_CHECK(esp_eth_stop(my_w5500_handle));

    ESP_LOGD(TAG, "esp_eth_del_netif_glue(my_w5500_netif_glue)");
    ESP_ERROR_CHECK(esp_eth_del_netif_glue(my_w5500_netif_glue));

    ESP_LOGD(TAG, "esp_netif_destroy(my_w5500_netif)");
    esp_netif_destroy(my_w5500_netif);

    ESP_LOGD(TAG, "esp_netif_deinit()");
    ESP_ERROR_CHECK(esp_netif_deinit());

    esp_eth_mac_t *mac = NULL;
    esp_eth_phy_t *phy = NULL;

    ESP_LOGD(TAG, "esp_eth_get_mac_instance(my_w5500_handle, &mac)");
    ESP_ERROR_CHECK(esp_eth_get_mac_instance(my_w5500_handle, &mac));

    ESP_LOGD(TAG, "esp_eth_get_phy_instance(my_w5500_handle, &phy)");
    ESP_ERROR_CHECK(esp_eth_get_phy_instance(my_w5500_handle, &phy));

    ESP_LOGD(TAG, "esp_eth_driver_uninstall(my_w5500_handle)");
    retVal = esp_eth_driver_uninstall(my_w5500_handle);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "Ethernet driver %p uninstall failed", my_w5500_handle);
    }

    ESP_LOGD(TAG, "spi_bus_free(ETH_SPI_HOST)");
    retVal = spi_bus_free(ETH_SPI_HOST);
    if (retVal != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_free failed");
    }

    // We installed the GPIO ISR service so let's uninstall it too.
    // BE CAREFUL HERE though since the service might be used by other functionality!
    if (gpio_isr_svc_init_by_eth == true)
    {
        ESP_LOGW(TAG, "uninstalling GPIO ISR service!");
        gpio_uninstall_isr_service();
    }

    ESP_LOGD(TAG, "free(my_w5500_handle)");
    free(my_w5500_handle);

    ESP_LOGD(TAG, "esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, eth_event_handler)");
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, eth_event_handler));

    ESP_LOGD(TAG, "esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler)");
    ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler));

    ESP_LOGD(TAG, "esp_event_loop_delete_default()");
    ESP_ERROR_CHECK(esp_event_loop_delete_default());

    ESP_LOGD(TAG, "gpio_set_level(ETH_SPI_EN, 0)");
    ESP_ERROR_CHECK(gpio_set_level(ETH_SPI_EN, 0));

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "W5500 Ethernet deinit done");
}

bool getETHisConnected(void)
{
    return ETHConnected;
}

#endif // (CONFIG_ETH_ENABLED && CONFIG_ETH_USE_SPI_ETHERNET && CONFIG_ETH_SPI_ETHERNET_W5500)
