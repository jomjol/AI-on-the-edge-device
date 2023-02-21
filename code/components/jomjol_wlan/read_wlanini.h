#pragma once

#ifndef READ_WLANINI_H
#define READ_WLANINI_H

#include <string>

bool LoadWlanFromFile(std::string fn, char *&_ssid, char *&_password, char *&_hostname, char *&_ipadr, char *&_gw,  char *&_netmask, char *&_dns, int &_rssithreshold);

bool ChangeHostName(std::string fn, std::string _newhostname);
bool ChangeRSSIThreshold(std::string fn, int _newrssithreshold);


#endif //READ_WLANINI_H