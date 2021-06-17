#include "read_wlanini.h"

#include "Helper.h"

#include "connect_wlan.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>


std::vector<string> ZerlegeZeile(std::string input, std::string _delimiter = "")
{
	std::vector<string> Output;
	std::string delimiter = " =,";
    if (_delimiter.length() > 0){
        delimiter = _delimiter;
    }

	input = trim(input, delimiter);
	size_t pos = findDelimiterPos(input, delimiter);
	std::string token;
	while (pos != std::string::npos) {
		token = input.substr(0, pos);
		token = trim(token, delimiter);
		Output.push_back(token);
		input.erase(0, pos + 1);
		input = trim(input, delimiter);
		pos = findDelimiterPos(input, delimiter);
	}
	Output.push_back(input);

	return Output;
}



void LoadWlanFromFile(std::string fn, char *&_ssid, char *&_password, char *&_hostname, char *&_ipadr, char *&_gw,  char *&_netmask, char *&_dns)
{
    std::string ssid = "";
    std::string passphrase = "";
    std::string ipaddress = "";
    std::string gw = "";
    std::string netmask = "";
    std::string dns = "";

    std::string line = "";
    std::vector<string> zerlegt;
    hostname = std_hostname;

    FILE* pFile;
    fn = FormatFileName(fn);

    pFile = OpenFileAndWait(fn.c_str(), "r");
    printf("file loaded\n");

    if (pFile == NULL)
        return;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
//        printf("%s", line.c_str());
        zerlegt = ZerlegeZeile(line, "=");
        zerlegt[0] = trim(zerlegt[0], " ");
        for (int i = 2; i < zerlegt.size(); ++i)
            zerlegt[1] = zerlegt[1] + "=" + zerlegt[i];

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "HOSTNAME")){
            hostname = trim(zerlegt[1]);
            if ((hostname[0] == '"') && (hostname[hostname.length()-1] == '"')){
                hostname = hostname.substr(1, hostname.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "SSID")){
            ssid = trim(zerlegt[1]);
            if ((ssid[0] == '"') && (ssid[ssid.length()-1] == '"')){
                ssid = ssid.substr(1, ssid.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "PASSWORD")){
            passphrase = zerlegt[1];
            if ((passphrase[0] == '"') && (passphrase[passphrase.length()-1] == '"')){
                passphrase = passphrase.substr(1, passphrase.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "IP")){
            ipaddress = zerlegt[1];
            if ((ipaddress[0] == '"') && (ipaddress[ipaddress.length()-1] == '"')){
                ipaddress = ipaddress.substr(1, ipaddress.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "GATEWAY")){
            gw = zerlegt[1];
            if ((gw[0] == '"') && (gw[gw.length()-1] == '"')){
                gw = gw.substr(1, gw.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "NETMASK")){
            netmask = zerlegt[1];
            if ((netmask[0] == '"') && (netmask[netmask.length()-1] == '"')){
                netmask = netmask.substr(1, netmask.length()-2);
            }
        }

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "DNS")){
            dns = zerlegt[1];
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
}




bool ChangeHostName(std::string fn, std::string _newhostname)
{
    if (_newhostname == hostname)
        return false;

    string line = "";
    std::vector<string> zerlegt;

    bool found = false;

    std::vector<string> neuesfile;

    FILE* pFile;
    fn = FormatFileName(fn);
    pFile = OpenFileAndWait(fn.c_str(), "r");

    printf("file loaded\n");

    if (pFile == NULL)
        return false;

    char zw[1024];
    fgets(zw, 1024, pFile);
    line = std::string(zw);

    while ((line.size() > 0) || !(feof(pFile)))
    {
        printf("%s", line.c_str());
        zerlegt = ZerlegeZeile(line, "=");
        zerlegt[0] = trim(zerlegt[0], " ");

        if ((zerlegt.size() > 1) && (toUpper(zerlegt[0]) == "HOSTNAME")){
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

    pFile = OpenFileAndWait(fn.c_str(), "w+");

    for (int i = 0; i < neuesfile.size(); ++i)
    {
        printf(neuesfile[i].c_str());
        fputs(neuesfile[i].c_str(), pFile);
    }

    fclose(pFile);

    printf("*** Update hostname done ***\n");

    return true;
}

