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


enum InfluxDBVersion {
    INFLUXDB_V1,
    INFLUXDB_V2
};

/**
 * @class InfluxDB
 * @brief A class to handle connections and data publishing to InfluxDB servers.
 * 
 * This class supports both InfluxDB v1.x and v2.x versions. It provides methods to initialize
 * the connection parameters, publish data, and destroy the connection.
 * 
 * @private
 * @var std::string influxDBURI
 * URI for the InfluxDB server.
 * 
 * @var std::string database
 * Database name for InfluxDB v1.x.
 * 
 * @var std::string user
 * Username for InfluxDB v1.x.
 * 
 * @var std::string password
 * Password for InfluxDB v1.x.
 * 
 * @var std::string bucket
 * Bucket name for InfluxDB v2.x.
 * 
 * @var std::string org
 * Organization name for InfluxDB v2.x.
 * 
 * @var std::string token
 * Token for InfluxDB v2.x.
 * 
 * @var InfluxDBVersion version
 * Version of the InfluxDB server (v1.x or v2.x).
 * 
 * @var esp_http_client_handle_t httpClient
 * HTTP client handle for making requests to the InfluxDB server.
 * 
 * @var void connectHTTP()
 * Establishes an HTTP connection to the InfluxDB server.
 * 
 * @public
 * @fn void InfluxDBInitV1(std::string _influxDBURI, std::string _database, std::string _user, std::string _password)
 * Initializes the connection parameters for InfluxDB v1.x.
 * 
 * @fn void InfluxDBInitV2(std::string _influxDBURI, std::string _bucket, std::string _org, std::string _token)
 * Initializes the connection parameters for InfluxDB v2.x.
 * 
 * @fn void InfluxDBdestroy()
 * Destroys the InfluxDB connection.
 * 
 * @fn void InfluxDBPublish(std::string _measurement, std::string _key, std::string _content, long int _timeUTC)
 * Publishes data to the InfluxDB server.
 * 
 * @param _measurement The measurement name.
 * @param _key The key for the data point.
 * @param _content The content or value of the data point.
 * @param _timeUTC The timestamp in UTC for the data point.
 */

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



#endif //INTERFACE_INFLUXDB_H
#endif //ENABLE_INFLUXDB