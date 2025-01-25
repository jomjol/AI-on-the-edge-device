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
// void InfluxDBInit(std::string _influxDBURI, std::string _database, std::string _user, std::string _password);
// void InfluxDBPublish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC);

// Interface to InfluxDB v2.x
// void InfluxDB_V2_Init(std::string _uri, std::string _bucket, std::string _org, std::string _token);
// void InfluxDB_V2_Publish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC);



void InfluxDBdestroy();

enum InfluxDBVersion {
    INFLUXDB_V1,
    INFLUXDB_V2
};

class InfluxDB {
private:
    // Information for InfluxDB v1.x
    std::string influxDBURI = "";
    // Information for InfluxDB v1.x
    std::string database = "";
    std::string user = "";
    std::string password = "";

    // Information for InfluxDB v2.x
    std::string bucket = "";
    std::string org = "";
    std::string token = "";

    InfluxDBVersion version;

    esp_http_client_handle_t httpClient = NULL;

    void connectHTTP();

public:
    // Initialize the InfluxDB connection parameters
    void InfluxDBInitV1(std::string _influxDBURI, std::string _database, std::string _user, std::string _password);
    void InfluxDBInitV2(std::string _influxDBURI, std::string _bucket, std::string _org, std::string _token);

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