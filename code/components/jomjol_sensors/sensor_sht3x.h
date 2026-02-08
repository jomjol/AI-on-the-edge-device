#pragma once

#ifndef SENSOR_SHT3X_H
#define SENSOR_SHT3X_H

#include "sensor_manager.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

/**
 * @brief SHT3x Temperature and Humidity Sensor (I2C)
 */
class SensorSHT3x : public SensorBase {
public:
    /**
     * @brief Construct a new SensorSHT3x object
     * @param address I2C address (0x44 or 0x45)
     * @param mqttTopic MQTT topic for publishing
     * @param influxMeasurement InfluxDB measurement name
     * @param interval Read interval in seconds
     * @param mqttEnabled Enable MQTT publishing
     * @param influxEnabled Enable InfluxDB publishing
     */
    SensorSHT3x(uint8_t address, 
                const std::string& mqttTopic,
                const std::string& influxMeasurement,
                int interval,
                bool mqttEnabled,
                bool influxEnabled);
    
    virtual ~SensorSHT3x();
    
    bool init() override;
    bool readData() override;
    void publishMQTT() override;
    void publishInfluxDB() override;
    std::string getName() override { return "SHT3x"; }
    
    /**
     * @brief Get last temperature reading
     */
    float getTemperature() const { return _temperature; }
    
    /**
     * @brief Get last humidity reading
     */
    float getHumidity() const { return _humidity; }
    
    /**
     * @brief Check if a read operation is currently in progress
     * @return true if reading
     */
    bool isReadInProgress() const override { return _readTaskHandle != nullptr; }
    
private:
    float _temperature;
    float _humidity;
    uint8_t _i2cAddress;
    i2c_port_t _i2cPort;
    bool _initialized;
    TaskHandle_t _readTaskHandle;  // Handle for background read task
    bool _readSuccess;  // Result of background read
    
    /**
     * @brief Background task that polls sensor until measurement complete
     */
    static void readTaskWrapper(void* pvParameters);
    void readTask();
    
    /**
     * @brief Calculate CRC8 checksum for SHT3x
     * @param data Data buffer
     * @param len Data length
     * @return CRC8 checksum
     */
    uint8_t calculateCRC(const uint8_t* data, size_t len);
};

#endif // SENSOR_SHT3X_H
