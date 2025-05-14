#if defined(BOARD_ESP32_S3_ALEKSEI)
#pragma once

#ifndef READ_LANINI_H
#define READ_LANINI_H

#include <string>
#include "read_wlanini.h"


extern struct wlan_config wlan_config;


int LoadLanFromFile(std::string fn);
bool ChangeLanHostName(std::string fn, std::string _newhostname);


#endif //READ_WLANINI_H
#endif