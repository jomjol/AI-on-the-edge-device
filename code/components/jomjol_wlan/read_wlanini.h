#pragma once

#ifndef READ_WLANINI_H
#define READ_WLANINI_H

#include <string>

bool LoadWlanFromFile(std::string fn, char *&_ssid, char *&_password, char *&_hostname, char *&_ipadr, char *&_gw,  char *&_netmask, char *&_dns, int &_rssithreashold);

bool ChangeHostName(std::string fn, std::string _newhostname);
bool ChangeRSSIThreashold(std::string fn, int _newrssithreashold);


#endif //READ_WLANINI_H