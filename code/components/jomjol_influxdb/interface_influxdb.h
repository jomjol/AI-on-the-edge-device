#ifdef ENABLE_INFLUXDB

#pragma once
#ifndef INTERFACE_INFLUXDB_H
#define INTERFACE_INFLUXDB_H

#include <string>
#include <map>
#include <functional>


#include <string>
#include "esp_http_client.h"
#include "esp_log.h"


// Interface to InfluxDB v1.x
void InfluxDBInit(std::string _influxDBURI, std::string _database, std::string _user, std::string _password);
void InfluxDBPublish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC);

// Interface to InfluxDB v2.x
void InfluxDB_V2_Init(std::string _uri, std::string _bucket, std::string _org, std::string _token);
void InfluxDB_V2_Publish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC);



void InfluxDBdestroy();


class InfluxDB {
private:
    std::string influxDBURI;
    std::string database;
//    std::string measurement;
    std::string user;
    std::string password;
    esp_http_client_handle_t httpClient;

public:
    // Initialize the InfluxDB connection parameters
    void InfluxDBInit(std::string _influxDBURI, std::string _database, std::string _user, std::string _password);
    // Destroy the InfluxDB connection
    void InfluxDBdestroy();
    // Publish data to the InfluxDB server
    void InfluxDBPublish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC);
};

// Usage example:
// InfluxDB influxDB;
// influxDB.InfluxDBInit("http://your-influxdb-url", "your-database", "your-measurement", "user", "password");
// influxDB.InfluxDBPublish("key", "content", "timestamp");
// influxDB.InfluxDBdestroy();


#endif //INTERFACE_INFLUXDB_H
#endif //ENABLE_INFLUXDB