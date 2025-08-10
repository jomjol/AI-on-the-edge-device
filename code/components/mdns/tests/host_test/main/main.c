/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_console.h"
#include "mdns.h"
#include "mdns_console.h"

static const char *TAG = "mdns-test";

static void mdns_test_app(esp_netif_t *interface);

#ifdef CONFIG_TEST_CONSOLE
static EventGroupHandle_t s_exit_signal = NULL;

static int exit_console(int argc, char **argv)
{
    xEventGroupSetBits(s_exit_signal, 1);
    return 0;
}

#else
static void query_mdns_host(const char *host_name)
{
    ESP_LOGI(TAG, "Query A: %s.local", host_name);

    struct esp_ip4_addr addr;
    addr.addr = 0;

    esp_err_t err = mdns_query_a(host_name, 2000,  &addr);
    if (err) {
        if (err == ESP_ERR_NOT_FOUND) {
            ESP_LOGW(TAG, "%x: Host was not found!", (err));
            return;
        }
        ESP_LOGE(TAG, "Query Failed: %x", (err));
        return;
    }

    ESP_LOGI(TAG, "Query A: %s.local resolved to: " IPSTR, host_name, IP2STR(&addr));
}
#endif // TEST_CONSOLE

#ifndef CONFIG_IDF_TARGET_LINUX
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "nvs_flash.h"

/**
 * @brief This is an entry point for the real target device,
 * need to init few components and connect to a network interface
 */
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    mdns_test_app(EXAMPLE_INTERFACE);

    ESP_ERROR_CHECK(example_disconnect());
}
#else

/**
 * @brief This is an entry point for the linux target (simulator on host)
 * need to create a dummy WiFi station and use it as mdns network interface
 */
int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    const esp_netif_inherent_config_t base_cg = { .if_key = "WIFI_STA_DEF", .if_desc = CONFIG_TEST_NETIF_NAME };
    esp_netif_config_t cfg = { .base = &base_cg  };
    esp_netif_t *sta = esp_netif_new(&cfg);

    mdns_test_app(sta);

    esp_netif_destroy(sta);
    return 0;
}
#endif

static void mdns_test_app(esp_netif_t *interface)
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(CONFIG_TEST_HOSTNAME));
    ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_TEST_HOSTNAME);
    ESP_ERROR_CHECK(mdns_register_netif(interface));
    ESP_ERROR_CHECK(mdns_netif_action(interface, MDNS_EVENT_ENABLE_IP4 /*| MDNS_EVENT_ENABLE_IP6 */ | MDNS_EVENT_IP4_REVERSE_LOOKUP | MDNS_EVENT_IP6_REVERSE_LOOKUP));

#ifdef CONFIG_TEST_CONSOLE
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    s_exit_signal = xEventGroupCreate();

    repl_config.prompt = "mdns>";
    // init console REPL environment
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

    const esp_console_cmd_t cmd_exit = {
        .command = "exit",
        .help = "exit mDNS console application",
        .hint = NULL,
        .func = exit_console,
        .argtable = NULL
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_exit) );
    mdns_console_register();
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
    xEventGroupWaitBits(s_exit_signal, 1, pdTRUE, pdFALSE, portMAX_DELAY);
    repl->del(repl);
#else
    vTaskDelay(pdMS_TO_TICKS(10000));
    query_mdns_host("david-work");
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
    mdns_free();
    ESP_LOGI(TAG, "Exit");
}
