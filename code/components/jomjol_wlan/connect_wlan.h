#pragma once

#ifndef CONNECT_WLAN_H
#define CONNECT_WLAN_H

#include <string>

void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname, const char *_ipadr, const char *_gw,  const char *_netmask, const char *_dns, int _rssithreashold);
void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname);
void wifi_init_sta(const char *_ssid, const char *_password);

std::string* getIPAddress();
std::string* getSSID();
int get_WIFI_RSSI();
bool getWIFIisConnected();
void WIFIDestroy();

extern std::string hostname;
extern std::string std_hostname;
extern int RSSIThreashold;

#endif //CONNECT_WLAN_H