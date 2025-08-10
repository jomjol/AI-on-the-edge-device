/*
 * SPDX-FileCopyrightText: 2020-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "esp32_mock.h"

typedef struct esp_netif_s esp_netif_t;
typedef struct esp_netif_ip_info esp_netif_ip_info_t;
typedef struct esp_netif_dhcp_status esp_netif_dhcp_status_t;


const char *IP_EVENT = "IP_EVENT";


esp_err_t esp_netif_add_to_list(esp_netif_t *netif)
{
    return ESP_OK;
}

esp_err_t esp_netif_remove_from_list(esp_netif_t *netif)
{
    return ESP_ERR_NOT_FOUND;
}

esp_netif_t *esp_netif_next(esp_netif_t *netif)
{
    return NULL;
}

esp_netif_t *esp_netif_next_unsafe(esp_netif_t *netif)
{
    return NULL;
}

esp_netif_t *esp_netif_get_handle_from_ifkey(const char *if_key)
{
    return NULL;
}

esp_err_t esp_netif_get_ip_info(esp_netif_t *esp_netif, esp_netif_ip_info_t *ip_info)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t esp_netif_dhcpc_get_status(esp_netif_t *esp_netif, esp_netif_dhcp_status_t *status)
{
    return ESP_ERR_NOT_SUPPORTED;
}
