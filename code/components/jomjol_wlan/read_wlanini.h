#ifndef READ_WLANINI_H
#define READ_WLANINI_H

#include <string>

void LoadWlanFromFile(std::string fn, char *&_ssid, char *&_password, char *&_hostname, char *&_ipadr, char *&_gw,  char *&_netmask, char *&_dns);

bool ChangeHostName(std::string fn, std::string _newhostname);


#endif