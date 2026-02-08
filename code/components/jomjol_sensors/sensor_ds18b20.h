#pragma once

#ifndef SENSOR_DS18B20_H
#define SENSOR_DS18B20_H

#include "sensor_manager.h"
#include "driver/gpio.h"

/**
 * @brief DS18B20 Temperature Sensor (1-Wire)
 * 
 * Multi-sensor support with ROM search:
 * - ROM search is performed ONCE during init() at startup
 * - All discovered sensor ROM IDs are cached
 * - Each readData() call reads from the cached list of sensors
 * - Hot-plugging sensors after startup is NOT supported
 * - To detect new sensors, device must be restarted
 */
class SensorDS18B20 : public SensorBase {
public:
    /**
     * @brief Construct a new SensorDS18B20 object
     * @param gpio GPIO pin for 1-Wire bus
     * @param mqttTopic MQTT topic for publishing
     * @param influxMeasurement InfluxDB measurement name
     * @param interval Read interval in seconds
     * @param mqttEnabled Enable MQTT publishing
     * @param influxEnabled Enable InfluxDB publishing
     * @param expectedSensors Expected number of sensors (-1 = auto-detect, >0 = expected count for validation)
     */
    SensorDS18B20(gpio_num_t gpio,
                  const std::string& mqttTopic,
                  const std::string& influxMeasurement,
                  int interval,
                  bool mqttEnabled,
                  bool influxEnabled,
                  int expectedSensors = -1);
    
    virtual ~SensorDS18B20();
    
    bool init() override;
    bool readData() override;
    void publishMQTT() override;
    void publishInfluxDB() override;
    std::string getName() override { return "DS18B20"; }
    
    /**
     * @brief Get number of detected sensors on the bus
     */
    int getSensorCount() const;
    
    /**
     * @brief Get temperature reading for specific sensor
     * @param index Sensor index
     * @return Temperature in Celsius
     */
    float getTemperature(int index = 0) const;
    
    /**
     * @brief Get ROM ID for specific sensor
     * @param index Sensor index
     * @return ROM ID as hex string (e.g., "28-0123456789AB")
     */
    std::string getRomId(int index = 0) const;
    
    /**
     * @brief Check if a read operation is currently in progress
     * @return true if reading
     */
    bool isReadInProgress() const override { return _readTaskHandle != nullptr; }
    
private:
    std::vector<float> _temperatures;
    std::vector<std::array<uint8_t, 8>> _romIds; // Store ROM IDs for each sensor
    gpio_num_t _gpio;
    bool _initialized;
    TaskHandle_t _readTaskHandle;  // Handle for background read task
    bool _readSuccess;  // Result of background read
    int _expectedSensors;  // Expected number of sensors (-1 = auto-detect)
    
    /**
     * @brief Background task that polls sensors until conversion complete
     */
    static void readTaskWrapper(void* pvParameters);
    void readTask();
    
    /**
     * @brief Scan the 1-Wire bus for DS18B20 devices using ROM search
     * @return Number of devices found
     */
    int scanDevices();
    
    /**
     * @brief Start temperature conversion for a sensor (non-blocking)
     * @param sensorIndex Index of sensor to start conversion for
     * @return true if conversion started successfully
     */
    bool startConversion(size_t sensorIndex);
    
    /**
     * @brief Check if conversion is complete (non-blocking check only)
     * @param sensorIndex Index of sensor to check
     * @return true if conversion is complete
     */
    bool isConversionComplete(size_t sensorIndex);
    
    /**
     * @brief Read scratchpad from sensor
     * @param sensorIndex Index of sensor to read from
     * @return true if read was successful
     */
    bool readScratchpad(size_t sensorIndex);
    
    /**
     * @brief Perform 1-Wire ROM search to find all devices on the bus
     * @param romIds Vector to store found ROM IDs
     * @return Number of devices found
     * @note This is called ONCE during initialization, not on every read
     * @note The discovered ROM IDs are cached and reused for all subsequent reads
     */
    int performRomSearch(std::vector<std::array<uint8_t, 8>>& romIds);
    
    /**
     * @brief Calculate CRC8 for ROM or scratchpad data
     * @param data Data buffer
     * @param len Length of data
     * @return CRC8 value
     */
    uint8_t calculateCRC8(const uint8_t* data, int len);
};

#endif // SENSOR_DS18B20_H
