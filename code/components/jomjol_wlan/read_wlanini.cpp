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
#include "../../include/defines.h"

static const char *TAG = "WLAN.INI";

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



bool LoadWlanFromFile(std::string fn, char *&_ssid, char *&_password, char *&_hostname, char *&_ipadr, char *&_gw,  char *&_netmask, char *&_dns, int &_rssithreashold)
{
    std::string ssid = "";
    std::string passphrase = "";
    std::string ipaddress = "";
    std::string gw = "";
    std::string netmask = "";
    std::string dns = "";
    int rssithreshold = 0;

    std::string line = "";
    std::vector<string> splitted;
    hostname = std_hostname;

    FILE* pFile;
    fn = FormatFileName(fn);

    pFile = fopen(fn.c_str(), "r");
    if (!pFile)
        return false;

    ESP_LOGD(TAG, "file loaded");

    if (pFile == NULL)
        return false;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
//        ESP_LOGD(TAG, "%s", line.c_str());
        splitted = ZerlegeZeileWLAN(line, "=");
        splitted[0] = trim(splitted[0], " ");

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "HOSTNAME")){
            hostname = trim(splitted[1]);
            if ((hostname[0] == '"') && (hostname[hostname.length()-1] == '"')){
                hostname = hostname.substr(1, hostname.length()-2);
            }
        }

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "SSID")){
            ssid = trim(splitted[1]);
            if ((ssid[0] == '"') && (ssid[ssid.length()-1] == '"')){
                ssid = ssid.substr(1, ssid.length()-2);
            }
        }

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "RSSITHREASHOLD")){
            string _s = trim(splitted[1]);
            if ((_s[0] == '"') && (_s[_s.length()-1] == '"')){
                _s = _s.substr(1, ssid.length()-2);
            }
            rssithreshold = atoi(_s.c_str());
        }


        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "PASSWORD")){
            passphrase = splitted[1];
            if ((passphrase[0] == '"') && (passphrase[passphrase.length()-1] == '"')){
                passphrase = passphrase.substr(1, passphrase.length()-2);
            }
        }

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "IP")){
            ipaddress = splitted[1];
            if ((ipaddress[0] == '"') && (ipaddress[ipaddress.length()-1] == '"')){
                ipaddress = ipaddress.substr(1, ipaddress.length()-2);
            }
        }

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "GATEWAY")){
            gw = splitted[1];
            if ((gw[0] == '"') && (gw[gw.length()-1] == '"')){
                gw = gw.substr(1, gw.length()-2);
            }
        }

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "NETMASK")){
            netmask = splitted[1];
            if ((netmask[0] == '"') && (netmask[netmask.length()-1] == '"')){
                netmask = netmask.substr(1, netmask.length()-2);
            }
        }

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "DNS")){
            dns = splitted[1];
            if ((dns[0] == '"') && (dns[dns.length()-1] == '"')){
                dns = dns.substr(1, dns.length()-2);
            }
        }


        if (fgets(zw, 1024, pFile) == NULL)
        {
            line = "";
        }
        else
        {
            line = std::string(zw);
        }
    }

    fclose(pFile);

    // Check if Hostname was empty in .ini if yes set to std_hostname
    if(hostname.length() == 0){
        hostname = std_hostname;
    }

    _hostname = new char[hostname.length() + 1];
    strcpy(_hostname, hostname.c_str());

    _ssid = new char[ssid.length() + 1];
    strcpy(_ssid, ssid.c_str());

    _password = new char[passphrase.length() + 1];
    strcpy(_password, passphrase.c_str());

    if (ipaddress.length() > 0)
    {
        _ipadr = new char[ipaddress.length() + 1];
        strcpy(_ipadr, ipaddress.c_str());
    }
    else
        _ipadr = NULL;

    if (gw.length() > 0)
    {
        _gw = new char[gw.length() + 1];
        strcpy(_gw, gw.c_str());
    }
    else
        _gw = NULL;

    if (netmask.length() > 0)
    {
        _netmask = new char[netmask.length() + 1];
        strcpy(_netmask, netmask.c_str());
    }
    else
        _netmask = NULL;

    if (dns.length() > 0)
    {
        _dns = new char[dns.length() + 1];
        strcpy(_dns, dns.c_str());
    }
    else
        _dns = NULL;

    _rssithreashold = rssithreshold;
    RSSIThreashold = rssithreshold;
    return true;
}




bool ChangeHostName(std::string fn, std::string _newhostname)
{
    if (_newhostname == hostname)
        return false;

    string line = "";
    std::vector<string> splitted;

    bool found = false;

    std::vector<string> neuesfile;

    FILE* pFile;
    fn = FormatFileName(fn);
    pFile = fopen(fn.c_str(), "r");

    ESP_LOGD(TAG, "file loaded\n");

    if (pFile == NULL)
        return false;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
        ESP_LOGD(TAG, "%s", line.c_str());
        splitted = ZerlegeZeileWLAN(line, "=");
        splitted[0] = trim(splitted[0], " ");

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "HOSTNAME")){
            line = "hostname = \"" + _newhostname + "\"\n";
            found = true;
        }

        neuesfile.push_back(line);

        if (fgets(zw, 1024, pFile) == NULL)
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
        line = "\nhostname = \"" + _newhostname + "\"\n";
        neuesfile.push_back(line);        
    }

    fclose(pFile);

    pFile = fopen(fn.c_str(), "w+");

    for (int i = 0; i < neuesfile.size(); ++i)
    {
        ESP_LOGD(TAG, "%s", neuesfile[i].c_str());
        fputs(neuesfile[i].c_str(), pFile);
    }

    fclose(pFile);

    ESP_LOGD(TAG, "*** Hostname update done ***");

    return true;
}


bool ChangeRSSIThreashold(std::string fn, int _newrssithreashold)
{
    if (RSSIThreashold == _newrssithreashold)
        return false;

    string line = "";
    std::vector<string> splitted;

    bool found = false;

    std::vector<string> neuesfile;

    FILE* pFile;
    fn = FormatFileName(fn);
    pFile = fopen(fn.c_str(), "r");

    ESP_LOGD(TAG, "file loaded\n");

    if (pFile == NULL)
        return false;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
        ESP_LOGD(TAG, "%s", line.c_str());
        splitted = ZerlegeZeileWLAN(line, "=");
        splitted[0] = trim(splitted[0], " ");

        if ((splitted.size() > 1) && (toUpper(splitted[0]) == "RSSITHREASHOLD")){
            line = "RSSIThreashold = " + to_string(_newrssithreashold) + "\n";
            found = true;
        }

        neuesfile.push_back(line);

        if (fgets(zw, 1024, pFile) == NULL)
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
        line = "RSSIThreashold = " + to_string(_newrssithreashold) + "\n";
        neuesfile.push_back(line);        
    }

    fclose(pFile);

    pFile = fopen(fn.c_str(), "w+");

    for (int i = 0; i < neuesfile.size(); ++i)
    {
        ESP_LOGD(TAG, "%s", neuesfile[i].c_str());
        fputs(neuesfile[i].c_str(), pFile);
    }

    fclose(pFile);

    ESP_LOGD(TAG, "*** RSSIThreashold update done ***");

    return true;
}

