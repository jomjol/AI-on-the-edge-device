#pragma once

#ifndef READ_WLANINI_H
#define READ_WLANINI_H

#include <string>

struct wlan_config {
    std::string ssid = "";
    std::string password = "";
    std::string hostname = "watermeter";    // Default: watermeter
    std::string ipaddress = "";
    std::string gateway = "";
    std::string netmask = "";
    std::string dns = "";
    int rssi_threshold = 0;                 // Default: 0 -> ROAMING disabled
};
extern struct wlan_config wlan_config;


int LoadWlanFromFile(std::string fn);
bool ChangeHostName(std::string fn, std::string _newhostname);
bool ChangeRSSIThreshold(std::string fn, int _newrssithreshold);


#endif //READ_WLANINI_H