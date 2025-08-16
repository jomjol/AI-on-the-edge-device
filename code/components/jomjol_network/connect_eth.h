/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 *
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_eth.html#basic-ethernet-concepts
 */
#include "sdkconfig.h"

#if (CONFIG_ETH_ENABLED && CONFIG_ETH_USE_SPI_ETHERNET && CONFIG_ETH_SPI_ETHERNET_W5500)

#pragma once

#include <string>
#include <esp_err.h>
#include <esp_http_server.h>

#ifndef CONNECT_ETH_H
#define CONNECT_ETH_H

// Initialize Lan or WLAN based on config file
esp_err_t eth_init_W5500(void);
void eth_deinit_W5500(void);

bool getETHisConnected(void);

#endif // CONNECT_ETH_H

#endif // (CONFIG_ETH_ENABLED && CONFIG_ETH_USE_SPI_ETHERNET && CONFIG_ETH_SPI_ETHERNET_W5500)
