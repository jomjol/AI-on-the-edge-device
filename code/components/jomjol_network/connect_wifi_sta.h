#pragma once

#ifndef CONNECT_WIFI_STA_H
#define CONNECT_WIFI_STA_H

#include <string>
#include <esp_err.h>
#include <esp_log.h>

esp_err_t wifi_init_sta(void);
std::string *getIPAddress();
std::string *getSSID();
int get_WIFI_RSSI();
std::string *getHostname();

bool getWIFIisConnected();
void WIFIDestroy();

#if (defined WLAN_USE_MESH_ROAMING && defined WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES)
void wifiRoamingQuery(void);
#endif

#ifdef WLAN_USE_ROAMING_BY_SCANNING
void wifiRoamByScanning(void);
#endif

#endif // CONNECT_WIFI_STA_H
