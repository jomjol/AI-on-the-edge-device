#pragma once

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "sensor_config.h"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Base class for all sensors
 */
class SensorBase {
public:
    virtual ~SensorBase() {
        // CRITICAL: Stop periodic task before object is destroyed to prevent race condition
        // The task runs in a separate thread and accesses member variables
        // If we don't stop it, it will crash when trying to access freed memory
        stopPeriodicTask();
    }
    
    /**
     * @brief Initialize the sensor hardware
     * @return true if successful, false otherwise
     */
    virtual bool init() = 0;
    
    /**
     * @brief Read data from the sensor
     * @return true if successful, false otherwise
     */
    virtual bool readData() = 0;
    
    /**
     * @brief Publish sensor data to MQTT
     */
    virtual void publishMQTT() = 0;
    
    /**
     * @brief Publish sensor data to InfluxDB
     */
    virtual void publishInfluxDB() = 0;
    
    /**
     * @brief Get sensor name/identifier
     */
    virtual std::string getName() = 0;
    
    /**
     * @brief Check if it's time to read this sensor
     * @param flowInterval Current flow interval in seconds (for "follow flow" mode)
     * @return true if should read now
     */
    bool shouldRead(int flowInterval = 0);
    
    /**
     * @brief Get timestamp of last successful read
     * @return Unix timestamp
     */
    time_t getLastReadTime() const { return _lastRead; }
    
    /**
     * @brief Start periodic task for this sensor (for custom intervals)
     * @return true if task started successfully
     */
    bool startPeriodicTask();
    
    /**
     * @brief Stop periodic task for this sensor
     */
    void stopPeriodicTask();
    
    /**
     * @brief Get the read interval for this sensor
     * @return interval in seconds (-1 = follow flow, >0 = custom interval)
     */
    int getReadInterval() const { return _readInterval; }
    
    /**
     * @brief Check if sensor is currently performing an async read operation
     * @return true if read is in progress
     */
    virtual bool isReadInProgress() const { return false; }
    
    /**
     * @brief Get the MQTT topic for this sensor
     * @return MQTT topic string (empty if using default)
     */
    std::string getMqttTopic() const { return _mqttTopic; }

protected:
    std::string _mqttTopic;
    std::string _influxMeasurement;
    int _readInterval;           // -1 = follow flow, >0 = custom interval in seconds
    bool _mqttEnabled;
    bool _influxEnabled;
    time_t _lastRead = 0;
    
private:
    TaskHandle_t _taskHandle = nullptr;
    
    /**
     * @brief Static task function wrapper for FreeRTOS
     */
    static void sensorTaskWrapper(void* pvParameters);
    
    /**
     * @brief Periodic task function for reading sensor
     */
    void sensorTask();
};

/**
 * @brief Manager class for all sensors
 */
/**
 * @brief Sensor initialization status
 */
enum class SensorInitStatus {
    NOT_INITIALIZED,
    INITIALIZED,
    FAILED_BUS_INIT,
    FAILED_NO_DEVICE,
    FAILED_OTHER
};

/**
 * @brief Sensor error information
 */
struct SensorError {
    std::string sensorName;
    SensorInitStatus status;
    std::string errorMessage;
    int retryCount;
};

class SensorManager {
public:
    SensorManager();
    ~SensorManager();
    
    /**
     * @brief Initialize sensor manager from parsed configuration
     * @param configFile Path to config.ini file (for GPIO scanning)
     * @param sensorConfigs Map of sensor type to configuration
     * @return true if successful, false otherwise
     */
    bool initFromConfig(const std::string& configFile, const std::map<std::string, SensorConfig>& sensorConfigs);
    
    /**
     * @brief Initialize sensor manager and all configured sensors
     * @return true if successful, false otherwise
     * @deprecated Use initFromConfig instead
     */
    bool init();
    
    /**
     * @brief Update all sensors (read if interval elapsed, publish if needed)
     * @param flowInterval Current flow interval for "follow flow" mode (in seconds)
     */
    void update(int flowInterval = 0);
    
    /**
     * @brief Clean up and deinitialize all sensors
     */
    void deinit();
    
    /**
     * @brief Check if sensor manager is enabled
     */
    bool isEnabled() { return _enabled; }
    
    /**
     * @brief Get sensor data as JSON string
     * @return JSON string with sensor readings and timestamps
     */
    std::string getJSON();
    
    /**
     * @brief Get list of detected/enabled sensors
     * @return Vector of sensor pointers
     */
    const std::vector<std::unique_ptr<SensorBase>>& getSensors() const { return _sensors; }
    
    /**
     * @brief Get list of sensor errors
     * @return Vector of sensor errors
     */
    const std::vector<SensorError>& getSensorErrors() const { return _sensorErrors; }
    
    /**
     * @brief Check if there are any sensor errors
     * @return true if errors exist
     */
    bool hasSensorErrors() const { return !_sensorErrors.empty(); }
    
private:
    std::vector<std::unique_ptr<SensorBase>> _sensors;
    std::vector<SensorError> _sensorErrors;
    bool _enabled;
    bool _i2cInitialized;
    
    // TODO: Make retry count configurable via config file
    static constexpr int SENSOR_INIT_RETRY_COUNT = 3;
    
    /**
     * @brief Initialize I2C bus
     * @param sda SDA GPIO pin
     * @param scl SCL GPIO pin
     * @param freq I2C frequency in Hz
     * @return true if successful, false otherwise
     */
    bool initI2C(int sda, int scl, uint32_t freq);
    
    /**
     * @brief Scan GPIO configuration to find sensor pins
     * @param configFile Path to config.ini file
     * @param sdaPin Output: SDA pin number (-1 if not found)
     * @param sclPin Output: SCL pin number (-1 if not found)
     * @param onewirePin Output: 1-Wire pin number (-1 if not found)
     */
    void scanGPIOConfig(const std::string& configFile, int& sdaPin, int& sclPin, int& onewirePin);
    
    /**
     * @brief Add a sensor error to the error list
     * @param sensorName Name of the sensor
     * @param status Initialization status
     * @param errorMessage Detailed error message
     * @param retryCount Number of retries attempted
     */
    void addSensorError(const std::string& sensorName, SensorInitStatus status, 
                       const std::string& errorMessage, int retryCount);
};

#endif // SENSOR_MANAGER_H
