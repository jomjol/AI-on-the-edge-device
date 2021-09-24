#ifndef CONNECT_WLAN_H
#define CONNECT_WLAN_H

#include <string>

void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname, const char *_ipadr, const char *_gw,  const char *_netmask, const char *_dns);
void wifi_init_sta(const char *_ssid, const char *_password, const char *_hostname);
void wifi_init_sta(const char *_ssid, const char *_password);

std::string* getIPAddress();
std::string* getSSID();

extern std::string hostname;
extern std::string std_hostname;

#endif