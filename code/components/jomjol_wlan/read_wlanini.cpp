#include "read_wlanini.h"

#include "Helper.h"

#include "connect_wlan.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "esp_log.h"
#include "ClassLogFile.h"
#include "../../include/defines.h"

static const char *TAG = "WLANINI";


struct wlan_config wlan_config = {};


std::vector<string> ZerlegeZeileWLAN(std::string input, std::string _delimiter = "")
{
	std::vector<string> Output;
	std::string delimiter = " =,";
    if (_delimiter.length() > 0){
        delimiter = _delimiter;
    }

	input = trim(input, delimiter);
	size_t pos = findDelimiterPos(input, delimiter);
	std::string token;
    if (pos != std::string::npos)           // splitted only up to first equal sign !!! Special case for WLAN.ini
    {
		token = input.substr(0, pos);
		token = trim(token, delimiter);
		Output.push_back(token);
		input.erase(0, pos + 1);
		input = trim(input, delimiter);
	}
	Output.push_back(input);

	return Output;
}


int LoadWlanFromFile(std::string fn)
{
    std::string line = "";
    std::string tmp = "";
    std::vector<string> splitted;

    fn = FormatFileName(fn);
    FILE* pFile = fopen(fn.c_str(), "r");
    if (pFile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Unable to open file (read). Device init aborted!"); 
        return -1;
    }

    ESP_LOGD(TAG, "LoadWlanFromFile: wlan.ini opened");

    char zw[256];
    if (fgets(zw, sizeof(zw), pFile) == NULL) {
        line = "";
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "file opened, but empty or content not readable. Device init aborted!");
        fclose(pFile);
        return -1;
    }
    else {
        line = std::string(zw);
    }

    while ((line.size() > 0) || !(feof(pFile)))
    {
        //ESP_LOGD(TAG, "line: %s", line.c_str());
        if (line[0] != ';') {   // Skip lines which starts with ';'

            splitted = ZerlegeZeileWLAN(line, "=");
            splitted[0] = trim(splitted[0], " ");
            
            if ((splitted.size() > 1) && (toUpper(splitted[0]) == "SSID")){
                tmp = trim(splitted[1]);
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.ssid = tmp;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "SSID: " + wlan_config.ssid);
            }

            else if ((splitted.size() > 1) && (toUpper(splitted[0]) == "PASSWORD")){
                tmp = splitted[1];
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.password = tmp;
                #ifndef __HIDE_PASSWORD
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Password: " + wlan_config.password);
                #else
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Password: XXXXXXXX");
                #endif
            }   

            else if ((splitted.size() > 1) && (toUpper(splitted[0]) == "HOSTNAME")){
                tmp = trim(splitted[1]);
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.hostname = tmp;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Hostname: " + wlan_config.hostname);
            }

            else if ((splitted.size() > 1) && (toUpper(splitted[0]) == "IP")){
                tmp = splitted[1];
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.ipaddress = tmp;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "IP-Address: " + wlan_config.ipaddress);
            }

            else if ((splitted.size() > 1) && (toUpper(splitted[0]) == "GATEWAY")){
                tmp = splitted[1];
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.gateway = tmp;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Gateway: " + wlan_config.gateway);
            }

            else if ((splitted.size() > 1) && (toUpper(splitted[0]) == "NETMASK")){
                tmp = splitted[1];
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.netmask = tmp;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Netmask: " + wlan_config.netmask);
            }

            else if ((splitted.size() > 1) && (toUpper(splitted[0]) == "DNS")){
                tmp = splitted[1];
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.dns = tmp;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "DNS: " + wlan_config.dns);
            }
            #if (defined WLAN_USE_ROAMING_BY_SCANNING || (defined WLAN_USE_MESH_ROAMING && defined WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES))
            else if ((splitted.size() > 1) && (toUpper(splitted[0]) == "RSSITHRESHOLD")){
                tmp = trim(splitted[1]);
                if ((tmp[0] == '"') && (tmp[tmp.length()-1] == '"')){
                    tmp = tmp.substr(1, tmp.length()-2);
                }
                wlan_config.rssi_threshold = atoi(tmp.c_str());
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "RSSIThreshold: " + std::to_string(wlan_config.rssi_threshold));
            }
            #endif
        }

        /* read next line */
        if (fgets(zw, sizeof(zw), pFile) == NULL) {
            line = "";
        }
        else {
            line = std::string(zw);
        }
    }
    fclose(pFile);

    /* Check if SSID is empty (mandatory parameter) */
    if (wlan_config.ssid.empty()) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SSID empty. Device init aborted!");
        return -2;
    }

    /* Check if password is empty (mandatory parameter) */
    /* Disabled see issue #2393
    if (wlan_config.password.empty()) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Password empty. Device init aborted!");
        return -2;
    }
    */

    return 0;
}


bool ChangeHostName(std::string fn, std::string _newhostname)
{
    if (_newhostname == wlan_config.hostname)
        return false;

    std::string line = "";
    std::vector<string> splitted;
    std::vector<string> neuesfile;
    bool found = false;

    FILE* pFile = NULL;

    fn = FormatFileName(fn);
    pFile = fopen(fn.c_str(), "r");
    if (pFile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ChangeHostName: Unable to open file wlan.ini (read)"); 
        return false;
    }

    ESP_LOGD(TAG, "ChangeHostName: wlan.ini opened");

    char zw[256];
    if (fgets(zw, sizeof(zw), pFile) == NULL) {
        line = "";
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ChangeHostName: File opened, but empty or content not readable");
        return false;
    }
    else {
        line = std::string(zw);
    }

    while ((line.size() > 0) || !(feof(pFile)))
    {
        //ESP_LOGD(TAG, "ChangeHostName: line: %s", line.c_str());
        splitted = ZerlegeZeileWLAN(line, "=");
        splitted[0] = trim(splitted[0], " ");

        if ((splitted.size() > 1) && ((toUpper(splitted[0]) == "HOSTNAME") || (toUpper(splitted[0]) == ";HOSTNAME"))){
            line = "hostname = \"" + _newhostname + "\"\n";
            found = true;
        }

        neuesfile.push_back(line);

        if (fgets(zw, sizeof(zw), pFile) == NULL)
        {
            line = "";
        }
        else
        {
            line = std::string(zw);
        }
    }

    if (!found)
    {
        line  = "\n;++++++++++++++++++++++++++++++++++\n";
        line += "; Hostname: Name of device in network\n";
        line += "; This parameter can be configured via WebUI configuration\n";
        line += "; Default: \"watermeter\", if nothing is configured\n\n";
        line = "hostname = \"" + _newhostname + "\"\n";
        neuesfile.push_back(line);        
    }
    fclose(pFile);

    pFile = fopen(fn.c_str(), "w+");
    if (pFile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ChangeHostName: Unable to open file wlan.ini (write)"); 
        return false;
    }

    for (int i = 0; i < neuesfile.size(); ++i)
    {
        //ESP_LOGD(TAG, "%s", neuesfile[i].c_str());
        fputs(neuesfile[i].c_str(), pFile);
    }
    fclose(pFile);

    ESP_LOGD(TAG, "ChangeHostName done");

    return true;
}

#if (defined WLAN_USE_ROAMING_BY_SCANNING || (defined WLAN_USE_MESH_ROAMING && defined WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES))
bool ChangeRSSIThreshold(std::string fn, int _newrssithreshold)
{
    if (wlan_config.rssi_threshold == _newrssithreshold)
        return false;

    std::string line = "";
    std::vector<string> splitted;
    std::vector<string> neuesfile;
    bool found = false;

    FILE* pFile = NULL;

    fn = FormatFileName(fn);
    pFile = fopen(fn.c_str(), "r");
    if (pFile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ChangeRSSIThreshold: Unable to open file wlan.ini (read)"); 
        return false;
    }

    ESP_LOGD(TAG, "ChangeRSSIThreshold: wlan.ini opened");

    char zw[256];
    if (fgets(zw, sizeof(zw), pFile) == NULL) {
        line = "";
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ChangeRSSIThreshold: File opened, but empty or content not readable");
        return false;
    }
    else {
        line = std::string(zw);
    }

    while ((line.size() > 0) || !(feof(pFile)))
    {
        ESP_LOGD(TAG, "%s", line.c_str());
        splitted = ZerlegeZeileWLAN(line, "=");
        splitted[0] = trim(splitted[0], " ");

        /* Workaround to eliminate line with typo "RSSIThreashold" or "rssi" if existing */
        if (((splitted.size() > 1) && (toUpper(splitted[0]) == "RSSITHREASHOLD")) ||
            ((splitted.size() > 1) && (toUpper(splitted[0]) == ";RSSITHREASHOLD")) ||
            ((splitted.size() > 1) && (toUpper(splitted[0]) == "RSSI")) ||
            ((splitted.size() > 1) && (toUpper(splitted[0]) == ";RSSI"))) {
            if (fgets(zw, sizeof(zw), pFile) == NULL) {
                line = "";
            }
            else {
                line = std::string(zw);
            }
            continue;
        }

        if ((splitted.size() > 1) && ((toUpper(splitted[0]) == "RSSITHRESHOLD") || (toUpper(splitted[0]) == ";RSSITHRESHOLD"))) {
            line = "RSSIThreshold = " + to_string(_newrssithreshold) + "\n";
            found = true;
        }
    
        neuesfile.push_back(line);
        
        if (fgets(zw, sizeof(zw), pFile) == NULL) {
            line = "";
        }
        else {
            line = std::string(zw);
        }
    }

    if (!found)
    {
        line  = "\n;++++++++++++++++++++++++++++++++++\n";
        line += "; WIFI Roaming:\n";
        line += "; Network assisted roaming protocol is activated by default\n";
        line += "; AP / mesh system needs to support roaming protocol 802.11k/v\n";
        line += ";\n";
        line += "; Optional feature (usually not neccessary):\n";
        line += "; RSSI Threshold for client requested roaming query (RSSI < RSSIThreshold)\n";
        line += "; Note: This parameter can be configured via WebUI configuration\n";
        line += "; Default: 0 = Disable client requested roaming query\n\n";
        line += "RSSIThreshold = " + to_string(_newrssithreshold) + "\n";
        neuesfile.push_back(line);        
    }

    fclose(pFile);

    pFile = fopen(fn.c_str(), "w+");
    if (pFile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ChangeRSSIThreshold: Unable to open file wlan.ini (write)"); 
        return false;
    }

    for (int i = 0; i < neuesfile.size(); ++i)
    {
        //ESP_LOGD(TAG, "%s", neuesfile[i].c_str());
        fputs(neuesfile[i].c_str(), pFile);
    }

    fclose(pFile);

    ESP_LOGD(TAG, "ChangeRSSIThreshold done");

    return true;
}
#endif
