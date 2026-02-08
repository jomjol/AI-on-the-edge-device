#include "sensor_ds18b20.h"
#include "ClassLogFile.h"

#include <array>
#include <cstdio>

#ifdef ENABLE_MQTT
#include "interface_mqtt.h"
#include "server_mqtt.h"
#endif

#ifdef ENABLE_INFLUXDB
#include "interface_influxdb.h"
extern InfluxDB influxDB;
#endif

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"

static const char *TAG = "DS18B20";

// DS18B20 commands
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE
#define DS18B20_CMD_SEARCH_ROM      0xF0
#define DS18B20_CMD_MATCH_ROM       0x55

SensorDS18B20::SensorDS18B20(gpio_num_t gpio,
                             const std::string& mqttTopic,
                             const std::string& influxMeasurement,
                             int interval,
                             bool mqttEnabled,
                             bool influxEnabled,
                             int expectedSensors)
    : _gpio(gpio), _initialized(false), _readTaskHandle(nullptr), _readSuccess(false), _expectedSensors(expectedSensors)
{
    _mqttTopic = mqttTopic;
    _influxMeasurement = influxMeasurement;
    _readInterval = interval;
    _mqttEnabled = mqttEnabled;
    _influxEnabled = influxEnabled;
    _lastRead = 0;
}

SensorDS18B20::~SensorDS18B20()
{
    // Stop background task if running
    if (_readTaskHandle != nullptr) {
        vTaskDelete(_readTaskHandle);
        _readTaskHandle = nullptr;
    }
    
    if (_initialized) {
        gpio_reset_pin(_gpio);
    }
}

// Simple 1-Wire bit-banging functions
static inline void ow_set_high(gpio_num_t gpio)
{
    gpio_set_direction(gpio, GPIO_MODE_INPUT);  // High-Z (pulled up externally)
}

static inline void ow_set_low(gpio_num_t gpio)
{
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, 0);
}

static inline int ow_read(gpio_num_t gpio)
{
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
    return gpio_get_level(gpio);
}

static bool ow_reset(gpio_num_t gpio)
{
    // Pull bus low for 480μs
    ow_set_low(gpio);
    ets_delay_us(480);
    
    // Release bus and wait for presence pulse
    ow_set_high(gpio);
    ets_delay_us(70);
    
    // Read presence pulse (sensor should pull low)
    int presence = !ow_read(gpio);
    
    // Wait for presence pulse to complete
    ets_delay_us(410);
    
    return presence;
}

static void ow_write_bit(gpio_num_t gpio, int bit)
{
    if (bit) {
        // Write '1': Pull low for 6μs, then release
        ow_set_low(gpio);
        ets_delay_us(6);
        ow_set_high(gpio);
        ets_delay_us(64);
    } else {
        // Write '0': Pull low for 60μs, then release
        ow_set_low(gpio);
        ets_delay_us(60);
        ow_set_high(gpio);
        ets_delay_us(10);
    }
}

static int ow_read_bit(gpio_num_t gpio)
{
    // Pull low for 3μs to initiate read
    ow_set_low(gpio);
    ets_delay_us(3);
    
    // Release and wait 10μs
    ow_set_high(gpio);
    ets_delay_us(10);
    
    // Read bit
    int bit = ow_read(gpio);
    
    // Wait for rest of time slot
    ets_delay_us(53);
    
    return bit;
}

static void ow_write_byte(gpio_num_t gpio, uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        ow_write_bit(gpio, (byte >> i) & 0x01);
    }
}

static uint8_t ow_read_byte(gpio_num_t gpio)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte |= (ow_read_bit(gpio) << i);
    }
    return byte;
}

uint8_t SensorDS18B20::calculateCRC8(const uint8_t* data, int len)
{
    uint8_t crc = 0;
    for (int i = 0; i < len; i++) {
        uint8_t inByte = data[i];
        for (int j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ inByte) & 0x01;
            crc >>= 1;
            if (mix) {
                crc ^= 0x8C;
            }
            inByte >>= 1;
        }
    }
    return crc;
}

int SensorDS18B20::performRomSearch(std::vector<std::array<uint8_t, 8>>& romIds)
{
    romIds.clear();
    
    int lastDiscrepancy = 0;
    int lastFamilyDiscrepancy = 0;
    bool lastDeviceFlag = false;
    std::array<uint8_t, 8> romBuffer = {0};  // Initialize to zeros
    
    // Keep searching until all devices are found
    while (!lastDeviceFlag) {
        int idBitNumber = 1;
        int lastZero = 0;
        int romByteNumber = 0;
        uint8_t romByteMask = 1;
        bool searchResult = false;
        
        // Reset the bus
        if (!ow_reset(_gpio)) {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "No devices found on 1-Wire bus during ROM search");
            break;
        }
        
        // Issue search command
        ow_write_byte(_gpio, DS18B20_CMD_SEARCH_ROM);
        
        // Loop through all 64 bits of the ROM
        do {
            // Read a bit and its complement
            int idBit = ow_read_bit(_gpio);
            int cmpIdBit = ow_read_bit(_gpio);
            
            // Check for no devices or error
            if (idBit && cmpIdBit) {
                break;  // No devices responded
            }
            
            int searchDirection;
            
            // If both bits are 0, there's a discrepancy
            if (!idBit && !cmpIdBit) {
                // If this discrepancy is before the last discrepancy
                // on a previous Next, pick the same as last time
                if (idBitNumber < lastDiscrepancy) {
                    searchDirection = ((romBuffer[romByteNumber] & romByteMask) > 0);
                } else {
                    // If equal to last pick 1, if not pick 0
                    searchDirection = (idBitNumber == lastDiscrepancy);
                }
                
                // If 0 was picked, record its position
                if (searchDirection == 0) {
                    lastZero = idBitNumber;
                    
                    // Check for Last discrepancy in family
                    if (lastZero < 9) {
                        lastFamilyDiscrepancy = lastZero;
                    }
                }
            } else {
                // Bit write value for search
                searchDirection = idBit;
            }
            
            // Set or clear the bit in the ROM byte
            if (searchDirection) {
                romBuffer[romByteNumber] |= romByteMask;
            } else {
                romBuffer[romByteNumber] &= ~romByteMask;
            }
            
            // Serial number search direction write bit
            ow_write_bit(_gpio, searchDirection);
            
            // Increment bit counters
            idBitNumber++;
            romByteMask <<= 1;
            
            // If the mask is 0, go to new byte and reset mask
            if (romByteMask == 0) {
                romByteNumber++;
                romByteMask = 1;
            }
            
        } while (romByteNumber < 8);
        
        // If search was successful
        if (idBitNumber >= 65) {
            // Verify CRC
            if (calculateCRC8(romBuffer.data(), 7) == romBuffer[7]) {
                // Check if this is a DS18B20 (family code 0x28)
                if (romBuffer[0] == 0x28) {
                    romIds.push_back(romBuffer);
                    searchResult = true;
                } else {
                    // Format family code as proper hex
                    char familyCodeHex[8];
                    snprintf(familyCodeHex, sizeof(familyCodeHex), "0x%02X", romBuffer[0]);
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Found non-DS18B20 device with family code: " + 
                                        std::string(familyCodeHex));
                }
            } else {
                LogFile.WriteToFile(ESP_LOG_WARN, TAG, "CRC mismatch in ROM search");
            }
        }
        
        // Set search state for next iteration
        lastDiscrepancy = lastZero;
        
        // Check if we're done
        if (lastDiscrepancy == 0) {
            lastDeviceFlag = true;
        }
    }
    
    return romIds.size();
}

bool SensorDS18B20::startConversion(size_t sensorIndex)
{
    if (sensorIndex >= _romIds.size()) {
        return false;
    }
    
    // Reset and check presence
    if (!ow_reset(_gpio)) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "No presence pulse for sensor #" + std::to_string(sensorIndex + 1));
        return false;
    }
    
    // Match ROM - address specific sensor
    ow_write_byte(_gpio, DS18B20_CMD_MATCH_ROM);
    
    // Send the ROM ID
    for (int i = 0; i < 8; i++) {
        ow_write_byte(_gpio, _romIds[sensorIndex][i]);
    }
    
    // Start temperature conversion
    ow_write_byte(_gpio, DS18B20_CMD_CONVERT_T);
    
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Started conversion for sensor #" + std::to_string(sensorIndex + 1));
    
    return true;
}

bool SensorDS18B20::isConversionComplete(size_t sensorIndex)
{
    if (sensorIndex >= _romIds.size()) {
        return false;
    }
    
    // Check if conversion is complete by reading the bus
    // DS18B20 pulls line low during conversion, releases when done
    return ow_read(_gpio);
}

bool SensorDS18B20::readScratchpad(size_t sensorIndex)
{
    if (sensorIndex >= _romIds.size()) {
        return false;
    }
    
    // Reset and check presence
    if (!ow_reset(_gpio)) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "No presence pulse when reading sensor #" + std::to_string(sensorIndex + 1));
        return false;
    }
    
    // Small delay after reset for bus stabilization
    vTaskDelay(pdMS_TO_TICKS(1));
    
    // Match ROM again
    ow_write_byte(_gpio, DS18B20_CMD_MATCH_ROM);
    
    // Send the ROM ID again
    for (int i = 0; i < 8; i++) {
        ow_write_byte(_gpio, _romIds[sensorIndex][i]);
    }
    
    // Small delay before read command for better reliability
    vTaskDelay(pdMS_TO_TICKS(1));
    
    // Read scratchpad
    ow_write_byte(_gpio, DS18B20_CMD_READ_SCRATCHPAD);
    
    // Read 9 bytes
    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        data[i] = ow_read_byte(_gpio);
    }
    
    // Verify CRC
    if (calculateCRC8(data, 8) != data[8]) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CRC mismatch for sensor #" + std::to_string(sensorIndex + 1) + 
                            " (expected: 0x" + std::to_string(calculateCRC8(data, 8)) + 
                            ", got: 0x" + std::to_string(data[8]) + ")");
        return false;
    }
    
    // Convert temperature
    int16_t rawTemp = (data[1] << 8) | data[0];
    float temp = (float)rawTemp / 16.0f;
    
    // Update temperature directly
    _temperatures[sensorIndex] = temp;
    
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Sensor #" + std::to_string(sensorIndex + 1) + 
                        " (" + getRomId(sensorIndex) + "): " + std::to_string(temp) + "°C");
    
    return true;
}

bool SensorDS18B20::init()
{
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Initializing DS18B20 sensor on GPIO" + 
                        std::to_string(_gpio));
    
    // Log expected sensor count if configured
    if (_expectedSensors > 0) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Expected sensor count: " + std::to_string(_expectedSensors));
    }
    
    // Configure GPIO
    gpio_reset_pin(_gpio);
    gpio_set_pull_mode(_gpio, GPIO_PULLUP_ONLY);  // Enable pull-up
    ow_set_high(_gpio);
    
    // Test communication
    if (!ow_reset(_gpio)) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "No DS18B20 device found on GPIO" + 
                            std::to_string(_gpio));
        return false;
    }
    
    _initialized = true;
    
    // Perform ROM search to find all devices on the bus (ONE-TIME at startup)
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=== DS18B20 ROM Search (startup only) ===");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Scanning 1-Wire bus for DS18B20 devices...");
    
    // Retry ROM search with validation
    const int MAX_ROM_SEARCH_RETRIES = 5;
    int deviceCount = 0;
    int bestDeviceCount = 0;
    std::vector<std::array<uint8_t, 8>> bestRomIds;
    
    for (int retry = 0; retry < MAX_ROM_SEARCH_RETRIES; retry++) {
        if (retry > 0) {
            int delayMs = 100 + (retry * 50);  // 100ms, 150ms, 200ms, 250ms, 300ms
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "ROM search retry " + std::to_string(retry + 1) + 
                                " after " + std::to_string(delayMs) + "ms");
            vTaskDelay(pdMS_TO_TICKS(delayMs));
        }
        
        deviceCount = performRomSearch(_romIds);
        
        // Keep track of best result
        if (deviceCount > bestDeviceCount) {
            bestDeviceCount = deviceCount;
            bestRomIds = _romIds;
        }
        
        // If expected sensor count is set, validate against it
        if (_expectedSensors > 0) {
            if (deviceCount == _expectedSensors) {
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "ROM search found expected " + 
                                    std::to_string(deviceCount) + " sensor(s) on retry " + std::to_string(retry + 1));
                break;
            } else if (deviceCount > 0) {
                LogFile.WriteToFile(ESP_LOG_WARN, TAG, "ROM search found " + std::to_string(deviceCount) + 
                                    " sensor(s), expected " + std::to_string(_expectedSensors));
            }
        } else {
            // Auto-detect mode: accept any positive result
            if (deviceCount > 0) {
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "ROM search found " + std::to_string(deviceCount) + 
                                    " sensor(s) on retry " + std::to_string(retry + 1));
                break;
            }
        }
    }
    
    // Use best result if we didn't get the expected count (only when ExpectedSensors is configured)
    if (_expectedSensors > 0 && deviceCount != _expectedSensors && bestDeviceCount > 0) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Using best ROM search result: " + 
                            std::to_string(bestDeviceCount) + " sensor(s)");
        _romIds = bestRomIds;
        deviceCount = bestDeviceCount;
    }
    
    if (deviceCount == 0) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ROM search found no DS18B20 devices after " + 
                            std::to_string(MAX_ROM_SEARCH_RETRIES) + " retries");
        return false;
    }
    
    // Log warning if we found fewer sensors than expected
    if (_expectedSensors > 0 && deviceCount < _expectedSensors) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Found " + std::to_string(deviceCount) + 
                            " sensor(s) but expected " + std::to_string(_expectedSensors) + 
                            " - continuing with detected sensors");
    }
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "ROM search complete: Found " + std::to_string(deviceCount) + " DS18B20 sensor(s)");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Discovered ROM IDs (will be used for all future reads):");
    
    // Log ROM IDs
    for (size_t i = 0; i < _romIds.size(); i++) {
        std::string romIdStr = getRomId(i);
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "  Sensor #" + std::to_string(i + 1) + ": " + romIdStr);
    }
    
    // Read initial temperatures from all sensors
    _temperatures.clear();
    _temperatures.resize(_romIds.size(), 0.0f);
    
    // Set timestamp for initial read
    _lastRead = time(nullptr);
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=== DS18B20 initialization complete ===");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Future reads will use these " + std::to_string(deviceCount) + 
                        " cached sensor(s) without re-scanning");
    
    return true;
}

void SensorDS18B20::readTaskWrapper(void* pvParameters)
{
    SensorDS18B20* sensor = static_cast<SensorDS18B20*>(pvParameters);
    sensor->readTask();
}

void SensorDS18B20::readTask()
{
    // Use tick-based timing instead of time() which returns epoch (1970) on cold boot before NTP sync
    TickType_t taskStartTicks = xTaskGetTickCount();
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "DS18B20 background read task started for " + 
                        std::to_string(_romIds.size()) + " sensor(s)");
    
    // Read all sensors with retry logic
    const int maxRetries = 5;  // Increased from 3 to 5 for better transient error handling
    bool anySuccess = false;
    
    for (size_t sensorIndex = 0; sensorIndex < _romIds.size(); sensorIndex++) {
        bool success = false;
        
        for (int retry = 0; retry < maxRetries; retry++) {
            // Start conversion
            if (!startConversion(sensorIndex)) {
                if (retry < maxRetries - 1) {
                    // Exponential backoff: 50ms, 100ms, 150ms, 200ms, 250ms
                    int delayMs = 50 + (retry * 50);
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Failed to start conversion for sensor #" + 
                                        std::to_string(sensorIndex + 1) + ", retry " + std::to_string(retry + 1) + 
                                        " after " + std::to_string(delayMs) + "ms");
                    vTaskDelay(pdMS_TO_TICKS(delayMs));
                    continue;
                }
                break;
            }
            
            // Poll for completion - yields to other tasks
            bool conversionComplete = false;
            const int maxWaitMs = 1000;
            const int pollIntervalMs = 10;
            
            for (int elapsed = 0; elapsed < maxWaitMs; elapsed += pollIntervalMs) {
                vTaskDelay(pdMS_TO_TICKS(pollIntervalMs));
                
                if (isConversionComplete(sensorIndex)) {
                    conversionComplete = true;
                    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Conversion completed for sensor #" + 
                                        std::to_string(sensorIndex + 1) + " after ~" + std::to_string(elapsed) + "ms");
                    break;
                }
            }
            
            if (!conversionComplete) {
                if (retry < maxRetries - 1) {
                    int delayMs = 50 + (retry * 50);
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Conversion timeout for sensor #" + 
                                        std::to_string(sensorIndex + 1) + ", retry " + std::to_string(retry + 1) + 
                                        " after " + std::to_string(delayMs) + "ms");
                    vTaskDelay(pdMS_TO_TICKS(delayMs));
                    continue;
                }
                break;
            }
            
            // Add settling delay after conversion completes to reduce CRC errors
            // This gives the sensor time to stabilize the data before we read it
            vTaskDelay(pdMS_TO_TICKS(3));
            
            // Read the data
            if (readScratchpad(sensorIndex)) {
                success = true;
                anySuccess = true;
                break;
            } else if (retry < maxRetries - 1) {
                // Exponential backoff on read failures
                int delayMs = 50 + (retry * 50);
                LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Read failed for sensor #" + 
                                    std::to_string(sensorIndex + 1) + " (likely CRC error), retry " + 
                                    std::to_string(retry + 1) + " after " + std::to_string(delayMs) + "ms");
                vTaskDelay(pdMS_TO_TICKS(delayMs));
            }
        }
        
        if (!success) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to read sensor #" + 
                                std::to_string(sensorIndex + 1) + " after " + std::to_string(maxRetries) + " attempts");
        }
    }
    
    _readSuccess = anySuccess;
    
    // Calculate task duration using ticks (works even when time() returns epoch on cold boot)
    TickType_t taskEndTicks = xTaskGetTickCount();
    TickType_t taskDurationTicks = taskEndTicks - taskStartTicks;
    int taskDurationSeconds = taskDurationTicks / configTICK_RATE_HZ;
    
    if (anySuccess) {
        // Update last read time using time(nullptr) for consistency with shouldRead()
        // Note: On cold boot before NTP, this is seconds since boot, which is fine for interval checking
        _lastRead = time(nullptr);
        
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "DS18B20 read completed successfully in " + 
                            std::to_string(taskDurationSeconds) + "s, publishing data");
        
        // Publish data from background task after successful read
        publishMQTT();
        publishInfluxDB();
        
        // Calculate total time including publishing
        TickType_t totalTicks = xTaskGetTickCount() - taskStartTicks;
        int totalSeconds = totalTicks / configTICK_RATE_HZ;
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "DS18B20 async task finished - clearing handle and exiting (total time: " + 
                            std::to_string(totalSeconds) + "s)");
    } else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Background read task failed to read any sensors after " + 
                            std::to_string(taskDurationSeconds) + "s");
    }
    
    // Clear handle before deleting task to prevent race condition
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "DS18B20 async task setting handle to nullptr (was=" + 
                        std::to_string((unsigned long)_readTaskHandle) + ")");
    _readTaskHandle = nullptr;
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "DS18B20 async task calling vTaskDelete(NULL)");
    vTaskDelete(NULL);
}

bool SensorDS18B20::readData()
{
    if (!_initialized) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Cannot read DS18B20: sensor not initialized");
        return false;
    }
    
    // Check if a read is already in progress
    // Note: This check is safe because:
    // 1. Task sets _readTaskHandle to nullptr immediately before vTaskDelete(NULL)
    // 2. vTaskDelete(NULL) for current task is synchronous - task ends immediately
    // 3. No window where task is active but handle is nullptr
    if (_readTaskHandle != nullptr) {
        // Read still in progress, return false (not complete yet)
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "DS18B20 read skipped: previous read task still in progress (handle=" + 
                            std::to_string((unsigned long)_readTaskHandle) + ")");
        return false;
    }
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "DS18B20 readData() starting - creating async read task");
    
    // Note: shouldRead() check is done by SensorManager::update() before calling this
    // Start a background task to read sensors asynchronously
    // This task will poll sensors and delete itself when done
    
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        &SensorDS18B20::readTaskWrapper,
        "ds18b20_read",
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
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "DS18B20 readData() spawned async read task successfully (handle=" + 
                        std::to_string((unsigned long)_readTaskHandle) + ")");
    
    // Return true to indicate read was initiated
    // The task will complete in background
    return true;
}

int SensorDS18B20::getSensorCount() const
{
    return _temperatures.size();
}

float SensorDS18B20::getTemperature(int index) const
{
    if (index >= 0 && index < (int)_temperatures.size()) {
        return _temperatures[index];
    }
    return 0.0f;
}

std::string SensorDS18B20::getRomId(int index) const
{
    if (index >= 0 && index < (int)_romIds.size()) {
        char buffer[32];
        // Format as "28-XXXXXXXXXXXXXX" where 28 is the family code for DS18B20
        // ROM format: [0]=family, [1-6]=serial, [7]=CRC
        snprintf(buffer, sizeof(buffer), "%02X-%02X%02X%02X%02X%02X%02X%02X",
                 _romIds[index][0],  // Family code (should be 0x28)
                 _romIds[index][6], _romIds[index][5], _romIds[index][4],
                 _romIds[index][3], _romIds[index][2], _romIds[index][1],
                 _romIds[index][7]); // CRC byte
        return std::string(buffer);
    }
    // Return placeholder format if ROM IDs not yet read
    return "28-00000000000000";
}

int SensorDS18B20::scanDevices()
{
    // Return the number of sensors found during ROM search
    return _romIds.size();
}

void SensorDS18B20::publishMQTT()
{
#ifdef ENABLE_MQTT
    if (!_mqttEnabled) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "MQTT publishing disabled for DS18B20");
        return;
    }
    
    if (!getMQTTisConnected()) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Cannot publish DS18B20 to MQTT: not connected");
        return;
    }
    
    if (_temperatures.empty()) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Cannot publish DS18B20 to MQTT: no sensor data");
        return;
    }
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Publishing " + std::to_string(_temperatures.size()) + 
                        " DS18B20 sensor(s) to MQTT");
    
    for (size_t i = 0; i < _temperatures.size(); i++) {
        // Determine base topic: use main topic if config topic is empty, otherwise use config topic
        // Note: This creates a dependency on the MQTT server module, but follows the existing pattern
        // used by other components (e.g., ClassFlowControll, server_GPIO) that call mqttServer_getMainTopic()
        std::string baseTopic;
        if (_mqttTopic.empty()) {
            // Use main topic with "ds18b20" subfolder for consistency with SHT3x pattern
            baseTopic = mqttServer_getMainTopic() + "/ds18b20";
        } else {
            baseTopic = _mqttTopic;
        }
        
        // Include ROM ID in topic for sensor identification, with /temperature suffix
        std::string romIdStr = getRomId(i);
        std::string topic = baseTopic + "/" + romIdStr + "/temperature";
        
        std::string value = std::to_string(_temperatures[i]);
        MQTTPublish(topic, value, 1, true);
        
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Published to MQTT: " + topic + " = " + value);
    }
#else
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "MQTT not compiled in (ENABLE_MQTT not defined)");
#endif
}

void SensorDS18B20::publishInfluxDB()
{
#ifdef ENABLE_INFLUXDB
    if (!_influxEnabled) {
        return;
    }
    
    time_t now = time(nullptr);
    
    for (size_t i = 0; i < _temperatures.size(); i++) {
        // Include sensor type and ROM ID in field name for identification
        std::string romIdStr = getRomId(i);
        std::string field = "ds18b20_" + romIdStr + "_temperature";
        
        influxDB.InfluxDBPublish(_influxMeasurement, 
                                 field, 
                                 std::to_string(_temperatures[i]), 
                                 now);
        
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Published to InfluxDB: " + field + " = " + 
                            std::to_string(_temperatures[i]));
    }
#endif
}
