#ifndef CONNECT_WLAN_H
#define CONNECT_WLAN_H

#include <string>
#include "driver/gpio.h"

const int CONNECTED_BIT = BIT0;

void initialise_wifi(std::string _ssid, std::string _passphrase, std::string _hostname);
void initialise_wifi_fixed_ip(std::string _ip, std::string _gw, std::string _netmask, std::string _ssid, std::string _passphrase, std::string _hostname, std::string _dns = "");


void LoadWlanFromFile(std::string fn, std::string &_ssid, std::string &_passphrase, std::string &_hostname);
void LoadNetConfigFromFile(std::string fn, std::string &_ip, std::string &_gw, std::string &_netmask, std::string &_dns);

bool ChangeHostName(std::string fn, std::string _newhostname);

std::string getHostname();
std::string getIPAddress();
std::string getSSID();
std::string getNetMask();
std::string getGW();

#endif