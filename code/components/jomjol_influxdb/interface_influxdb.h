#ifdef ENABLE_INFLUXDB

#pragma once
#ifndef INTERFACE_INFLUXDB_H
#define INTERFACE_INFLUXDB_H

#include <string>
#include <map>
#include <functional>

void InfluxDBInit(std::string _influxDBURI, std::string _database, std::string _measurement, std::string _user, std::string _password);
void InfluxDBdestroy();

void InfluxDBPublish(std::string _key, std::string _content, std::string _timestamp);

#endif //INTERFACE_INFLUXDB_H
#endif //ENABLE_INFLUXDB