#ifdef ENABLE_INFLUXDB

#pragma once
#ifndef INTERFACE_INFLUXDB_H
#define INTERFACE_INFLUXDB_H

#include <string>
#include <map>
#include <functional>

// Interface to InfluxDB v1.x
void InfluxDBInit(std::string _influxDBURI, std::string _database, std::string _user, std::string _password);
void InfluxDBPublish(std::string _measurement, std::string _key, std::string _content, std::string _timestamp);

// Interface to InfluxDB v2.x
void InfluxDB_V2_Init(std::string _uri, std::string _database, std::string _org, std::string _token);
void InfluxDB_V2_Publish(std::string _measurement, std::string _key, std::string _content, std::string _timestamp);



void InfluxDBdestroy();

#endif //INTERFACE_INFLUXDB_H
#endif //ENABLE_INFLUXDB