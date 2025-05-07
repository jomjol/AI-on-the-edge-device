#pragma once

#ifndef CONNECT_LAN_H
#define CONNECT_LAN_H

#include <string>
// #include "connect_wlan.h"

// int wifi_init_sta(void);
std::string* getLanIPAddress();
// int get_WIFI_RSSI();
std::string* getLanHostname();

bool getLanIsConnected();
void LanDestroy();
int lan_init();

#endif //CONNECT_WLAN_H