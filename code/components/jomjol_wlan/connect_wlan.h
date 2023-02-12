#pragma once

#ifndef CONNECT_WLAN_H
#define CONNECT_WLAN_H

#include <string>

int wifi_init_sta(void);
std::string* getIPAddress();
std::string* getSSID();
int get_WIFI_RSSI();
bool getWIFIisConnected();
void WIFIDestroy();

#endif //CONNECT_WLAN_H