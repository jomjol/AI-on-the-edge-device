#include "sensor_sht3x.h"
#include "ClassLogFile.h"

#ifdef ENABLE_MQTT
#include "interface_mqtt.h"
#include "server_mqtt.h"
#endif

#ifdef ENABLE_INFLUXDB
#include "interface_influxdb.h"
extern InfluxDB influxDB;
#endif

#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SHT3x";

// SHT3x I2C commands
#define SHT3X_CMD_MEASURE_HIGH_REP 0x2400  // High repeatability measurement
#define SHT3X_CMD_SOFT_RESET       0x30A2  // Soft reset command

SensorSHT3x::SensorSHT3x(uint8_t address,
                         const std::string& mqttTopic,
                         const std::string& influxMeasurement,
                         int interval,
                         bool mqttEnabled,
                         bool influxEnabled)
    : _temperature(0.0f), _humidity(0.0f), _i2cAddress(address),
      _i2cPort(I2C_NUM_0), _initialized(false), _readTaskHandle(nullptr), _readSuccess(false)
{
    _mqttTopic = mqttTopic;
    _influxMeasurement = influxMeasurement;
    _readInterval = interval;
    _mqttEnabled = mqttEnabled;
    _influxEnabled = influxEnabled;
    _lastRead = 0;
}

SensorSHT3x::~SensorSHT3x()
{
    // Stop background task if running
    if (_readTaskHandle != nullptr) {
        vTaskDelete(_readTaskHandle);
        _readTaskHandle = nullptr;
    }
}

uint8_t SensorSHT3x::calculateCRC(const uint8_t* data, size_t len)
{
    // CRC-8 polynomial: 0x31 (x^8 + x^5 + x^4 + 1)
    uint8_t crc = 0xFF;
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = crc << 1;
            }
        }
    }
    
    return crc;
}

void SensorSHT3x::readTaskWrapper(void* pvParameters)
{
    SensorSHT3x* sensor = static_cast<SensorSHT3x*>(pvParameters);
    sensor->readTask();
}

void SensorSHT3x::readTask()
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Background read task started");
    
    // Read with retry logic
    const int maxRetries = 5;  // Increased from 3 to 5 for better transient error handling
    bool success = false;
    
    for (int retry = 0; retry < maxRetries; retry++) {
        // Send measurement command
        uint8_t cmd[2];
        cmd[0] = (SHT3X_CMD_MEASURE_HIGH_REP >> 8) & 0xFF;
        cmd[1] = SHT3X_CMD_MEASURE_HIGH_REP & 0xFF;
        
        i2c_cmd_handle_t cmdHandle = i2c_cmd_link_create();
        i2c_master_start(cmdHandle);
        i2c_master_write_byte(cmdHandle, (_i2cAddress << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmdHandle, cmd, 2, true);
        i2c_master_stop(cmdHandle);
        
        esp_err_t ret = i2c_master_cmd_begin(_i2cPort, cmdHandle, pdMS_TO_TICKS(1000));
        i2c_cmd_link_delete(cmdHandle);
        
        if (ret != ESP_OK) {
            if (retry < maxRetries - 1) {
                // Exponential backoff: 50ms, 100ms, 150ms, 200ms, 250ms
                int delayMs = 50 + (retry * 50);
                LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Failed to send measurement command, retry " + 
                                    std::to_string(retry + 1) + " after " + std::to_string(delayMs) + "ms");
                vTaskDelay(pdMS_TO_TICKS(delayMs));
                continue;
            }
            break;
        }
        
        // Wait for measurement to complete before polling
        // SHT3x high repeatability measurement takes ~15ms according to datasheet
        vTaskDelay(pdMS_TO_TICKS(15));
        
        // Poll for completion - yields to other tasks
        // Timeout protection: max 100ms (should be done already, but just in case)
        bool measurementComplete = false;
        const int maxWaitMs = 100;
        const int pollIntervalMs = 5;
        
        for (int elapsed = 0; elapsed < maxWaitMs; elapsed += pollIntervalMs) {
            // Try to read data - sensor will NACK if not ready
            uint8_t data[6];
            cmdHandle = i2c_cmd_link_create();
            i2c_master_start(cmdHandle);
            i2c_master_write_byte(cmdHandle, (_i2cAddress << 1) | I2C_MASTER_READ, true);
            i2c_master_read(cmdHandle, data, 5, I2C_MASTER_ACK);
            i2c_master_read_byte(cmdHandle, &data[5], I2C_MASTER_NACK);
            i2c_master_stop(cmdHandle);
            
            ret = i2c_master_cmd_begin(_i2cPort, cmdHandle, pdMS_TO_TICKS(100));
            i2c_cmd_link_delete(cmdHandle);
            
            if (ret == ESP_OK) {
                // Data received - log completion time
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Measurement completed in ~" + 
                                    std::to_string(15 + elapsed) + "ms");
                
                // Add settling delay after data ready to reduce CRC errors
                vTaskDelay(pdMS_TO_TICKS(2));
                
                // Verify CRC
                uint8_t tempCRC = calculateCRC(&data[0], 2);
                uint8_t humCRC = calculateCRC(&data[3], 2);
                
                if (tempCRC != data[2]) {
                    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Temperature CRC mismatch (expected: 0x" + 
                                        std::to_string(tempCRC) + ", got: 0x" + std::to_string(data[2]) + ")");
                    break;  // Try retry
                }
                
                if (humCRC != data[5]) {
                    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Humidity CRC mismatch (expected: 0x" + 
                                        std::to_string(humCRC) + ", got: 0x" + std::to_string(data[5]) + ")");
                    break;  // Try retry
                }
                
                // Convert raw values to temperature and humidity
                uint16_t rawTemp = (data[0] << 8) | data[1];
                uint16_t rawHum = (data[3] << 8) | data[4];
                
                _temperature = -45.0f + 175.0f * (float)rawTemp / 65535.0f;
                _humidity = 100.0f * (float)rawHum / 65535.0f;
                
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Read: Temp=" + std::to_string(_temperature) + 
                                    "°C, Humidity=" + std::to_string(_humidity) + "%");
                
                measurementComplete = true;
                success = true;
                break;
            } else if (ret != ESP_ERR_TIMEOUT && ret != ESP_FAIL) {
                // Real I2C error (not just sensor busy)
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "I2C read error: " + std::to_string(ret));
                break;
            }
            // Otherwise sensor is still busy, continue polling after delay
            vTaskDelay(pdMS_TO_TICKS(pollIntervalMs));
        }
        
        if (success) {
            break;  // Success, exit retry loop
        }
        
        if (!measurementComplete && retry < maxRetries - 1) {
            // Exponential backoff on measurement timeout
            int delayMs = 50 + (retry * 50);
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Measurement timeout, retry " + 
                                std::to_string(retry + 1) + " after " + std::to_string(delayMs) + "ms");
            vTaskDelay(pdMS_TO_TICKS(delayMs));
        }
    }
    
    _readSuccess = success;
    
    if (success) {
        _lastRead = time(nullptr);
        
        // Publish data from background task after successful read
        publishMQTT();
        publishInfluxDB();
        
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Background read task completed successfully");
    } else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Background read task failed after " + 
                            std::to_string(maxRetries) + " attempts");
    }
    
    // Clear handle before deleting task to prevent race condition
    _readTaskHandle = nullptr;
    vTaskDelete(NULL);
}

bool SensorSHT3x::readData()
{
    if (!_initialized) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Cannot read SHT3x: sensor not initialized");
        return false;
    }
    
    // Check if a read is already in progress  
    // Note: This check is safe because:
    // 1. Task sets _readTaskHandle to nullptr immediately before vTaskDelete(NULL)
    // 2. vTaskDelete(NULL) for current task is synchronous - task ends immediately
    // 3. No window where task is active but handle is nullptr
    if (_readTaskHandle != nullptr) {
        // Read still in progress, return false (not complete yet)
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "SHT3x read skipped: previous read task still in progress");
        return false;
    }
    
    // Note: shouldRead() check is done by SensorManager::update() before calling this
    // Start a background task to read sensor asynchronously
    // This task will poll sensor and delete itself when done
    
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        &SensorSHT3x::readTaskWrapper,
        "sht3x_read",
        4096,  // Stack size
        this,  // Parameter
        tskIDLE_PRIORITY,  // Priority - low for reading (not critical)
        &_readTaskHandle,
        0  // Core 0
    );
    
    if (xReturned != pdPASS) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to create background read task");
        _readTaskHandle = nullptr;
        return false;
    }
    
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Started background read task (true async)");
    
    // Return true to indicate read was initiated
    // The task will complete in background
    return true;
}

bool SensorSHT3x::init()
{
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Initializing SHT3x sensor at address 0x" + 
                        std::to_string(_i2cAddress));
    
    // Small delay to ensure I2C bus is ready
    vTaskDelay(pdMS_TO_TICKS(5));
    
    // Try to communicate with the sensor - send soft reset
    uint8_t cmd[2];
    cmd[0] = (SHT3X_CMD_SOFT_RESET >> 8) & 0xFF;
    cmd[1] = SHT3X_CMD_SOFT_RESET & 0xFF;
    
    i2c_cmd_handle_t cmdHandle = i2c_cmd_link_create();
    i2c_master_start(cmdHandle);
    i2c_master_write_byte(cmdHandle, (_i2cAddress << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmdHandle, cmd, 2, true);
    i2c_master_stop(cmdHandle);
    
    esp_err_t ret = i2c_master_cmd_begin(_i2cPort, cmdHandle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmdHandle);
    
    if (ret != ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to communicate with SHT3x: " + std::to_string(ret));
        return false;
    }
    
    // Wait longer for reset to complete and sensor to be ready
    vTaskDelay(pdMS_TO_TICKS(20));
    
    _initialized = true;
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "SHT3x sensor initialized successfully");
    return true;
}

void SensorSHT3x::publishMQTT()
{
#ifdef ENABLE_MQTT
    if (!_mqttEnabled || !getMQTTisConnected()) {
        return;
    }
    
    // Determine base topic: use main topic if config topic is empty, otherwise use config topic
    // Note: This creates a dependency on the MQTT server module, but follows the existing pattern
    // used by other components (e.g., ClassFlowControll, server_GPIO) that call mqttServer_getMainTopic()
    std::string baseTopic;
    if (_mqttTopic.empty()) {
        baseTopic = mqttServer_getMainTopic() + "/sht3x";
    } else {
        baseTopic = _mqttTopic;
    }
    
    // Publish temperature
    std::string tempTopic = baseTopic + "/temperature";
    std::string tempValue = std::to_string(_temperature);
    MQTTPublish(tempTopic, tempValue, 1, true);
    
    // Publish humidity
    std::string humTopic = baseTopic + "/humidity";
    std::string humValue = std::to_string(_humidity);
    MQTTPublish(humTopic, humValue, 1, true);
    
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Published to MQTT: " + tempTopic + ", " + humTopic);
#endif
}

void SensorSHT3x::publishInfluxDB()
{
#ifdef ENABLE_INFLUXDB
    if (!_influxEnabled) {
        return;
    }
    
    time_t now = time(nullptr);
    
    // Publish temperature with sensor type prefix
    influxDB.InfluxDBPublish(_influxMeasurement, 
                             "sht3x_temperature", 
                             std::to_string(_temperature), 
                             now);
    
    // Publish humidity with sensor type prefix
    influxDB.InfluxDBPublish(_influxMeasurement, 
                             "sht3x_humidity", 
                             std::to_string(_humidity), 
                             now);
    
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Published to InfluxDB");
#endif
}
