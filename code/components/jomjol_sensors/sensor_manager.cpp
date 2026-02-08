#include "sensor_manager.h"
#include "sensor_sht3x.h"
#include "sensor_ds18b20.h"

#include "ClassLogFile.h"
#include "Helper.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cstdint>

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSOR_MANAGER";

// Helper function to safely parse integer without exceptions
static bool safeParseInt(const std::string& str, int& result) {
    if (str.empty()) {
        return false;
    }
    
    char* endptr = nullptr;
    errno = 0;
    long val = strtol(str.c_str(), &endptr, 10);
    
    // Check for errors
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        return false;
    }
    
    // Check if no conversion was performed
    if (endptr == str.c_str() || *endptr != '\0') {
        return false;
    }
    
    result = static_cast<int>(val);
    return true;
}

// Helper function to safely parse unsigned long without exceptions
static bool safeParseULong(const std::string& str, unsigned long& result, int base = 10) {
    if (str.empty()) {
        return false;
    }
    
    char* endptr = nullptr;
    errno = 0;
    unsigned long val = strtoul(str.c_str(), &endptr, base);
    
    // Check for errors
    if (errno == ERANGE) {
        return false;
    }
    
    // Check if no conversion was performed
    if (endptr == str.c_str() || *endptr != '\0') {
        return false;
    }
    
    result = val;
    return true;
}

bool SensorBase::shouldRead(int flowInterval)
{
    time_t now = time(nullptr);
    int interval = _readInterval;
    
    // If readInterval is -1, use flow interval (follow flow mode)
    if (_readInterval < 0) {
        interval = flowInterval;
    }
    
    // If interval is still 0 or negative, don't read
    if (interval <= 0) {
        return false;
    }
    
    return (now - _lastRead) >= interval;
}

void SensorBase::sensorTaskWrapper(void* pvParameters)
{
    SensorBase* sensor = static_cast<SensorBase*>(pvParameters);
    sensor->sensorTask();
}

void SensorBase::sensorTask()
{
    // ============================================================================
    // PERIODIC TASK FOR SENSOR READING (DS18B20, SHT3x, etc.)
    // ============================================================================
    // 
    // This task runs for sensors with custom intervals (not "follow flow" mode).
    // 
    // SCHEDULING LOGIC:
    //   1. Spawn async read task (returns immediately)
    //   2. Wait for async task to complete (polls isReadInProgress())
    //   3. Wait configured interval
    //   4. Repeat
    // 
    // This ensures interval is time BETWEEN reads, not overlapping reads.
    // Works correctly even if read takes longer than interval.
    // 
    // FAILPROOF FEATURES:
    //   - If readData() fails: skips and retries after interval
    //   - If async task hangs: timeout after 5 minutes, continue anyway
    //   - vTaskDelay() always executes: next iteration always scheduled
    //   - No try-catch: uses return values and timeouts
    // 
    // POWER EFFICIENCY:
    //   - Task at tskIDLE_PRIORITY (lowest)
    //   - vTaskDelay() yields CPU completely
    //   - Polling uses 100ms delays (not busy wait)
    // 
    // USED BY: DS18B20, SHT3x (both inherit from SensorBase)
    // ============================================================================
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Periodic task started (interval: " + 
                        std::to_string(_readInterval) + "s)");
    
    // Prevent integer overflow: _readInterval (seconds) * 1000 can overflow for large intervals
    uint64_t intervalMs = (uint64_t)_readInterval * 1000ULL;
    const uint64_t maxSafeMs = (uint64_t)UINT32_MAX * 1000ULL / configTICK_RATE_HZ;
    if (intervalMs > maxSafeMs) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Interval exceeds maximum, capping to " + 
                            std::to_string(maxSafeMs / 1000) + "s");
        intervalMs = maxSafeMs;
    }
    const TickType_t xDelay = pdMS_TO_TICKS(intervalMs);
    
    // Initial delay before first read
    TickType_t initialDelay = xDelay;
    if (_readInterval > 300) {  // If interval > 5 minutes, use shorter initial delay
        initialDelay = pdMS_TO_TICKS(30000);
    }
    
    float delaySeconds = (float)initialDelay / (float)configTICK_RATE_HZ;
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Waiting " + std::to_string((int)delaySeconds) + 
                        "s before first read");
    
    // Power-efficient sleep - completely yields CPU
    vTaskDelay(initialDelay);
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Starting main loop (interval: " + 
                        std::to_string(_readInterval) + "s between reads)");
    
    int iteration = 0;
    while (true) {
        iteration++;
        
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Iteration " + std::to_string(iteration));
        
        // Trigger async read (spawns separate task, returns immediately)
        bool readStarted = readData();
        if (!readStarted) {
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Read busy or failed, will retry after interval");
            // Even if read failed to start, still wait the interval before retrying
            vTaskDelay(xDelay);
            continue;
        }
        
        // Wait for async read task to complete before scheduling next iteration
        // This ensures interval is BETWEEN reads, not overlapping reads
        // Use polling with delays (power efficient - yields CPU between checks)
        // Add timeout to prevent infinite wait if async task crashes
        bool wasWaiting = false;
        int waitIterations = 0;
        const int maxWaitIterations = 3000;  // 3000 * 100ms = 5 minutes max wait
        
        while (isReadInProgress()) {
            if (!wasWaiting) {
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Waiting for async read to complete");
                wasWaiting = true;
            }
            
            // Power-efficient: yield CPU while waiting
            // Check every 100ms to be responsive but not wasteful
            vTaskDelay(pdMS_TO_TICKS(100));
            
            waitIterations++;
            if (waitIterations >= maxWaitIterations) {
                // Timeout: async task took too long or crashed
                // Log error and continue to next iteration to prevent permanent hang
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, 
                    "Timeout waiting for async read (waited " + 
                    std::to_string(waitIterations / 10) + "s), continuing anyway");
                break;  // Exit wait loop, proceed to interval delay
            }
        }
        
        if (wasWaiting) {
            if (waitIterations < maxWaitIterations) {
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, 
                    "Async read completed after " + std::to_string(waitIterations / 10) + "s");
            }
        }
        
        // Now that previous read is complete, wait for the configured interval
        // This ensures reads happen at interval AFTER completion, not overlapping
        vTaskDelay(xDelay);
    }
}

bool SensorBase::startPeriodicTask()
{
    // Only create task for custom intervals
    // _readInterval = -1 means "follow flow" mode - handled by SensorManager::update()
    // _readInterval > 0 means custom interval - needs dedicated task
    if (_readInterval <= 0) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Using follow-flow mode, no periodic task");
        return true;
    }
    
    if (_taskHandle != nullptr) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Task already running");
        return true;
    }
    
    // Create task at IDLE priority to ensure it never blocks main digitalization flow
    // This is critical for power and CPU efficiency
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        &SensorBase::sensorTaskWrapper,
        "sensor",              // Short name to save memory
        4096,                  // Stack size
        this,                  // Pass sensor object
        tskIDLE_PRIORITY,      // LOWEST priority - never blocks main flow
        &_taskHandle,
        0                      // Core 0
    );
    
    if (xReturned != pdPASS) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to create periodic task");
        _taskHandle = nullptr;
        return false;
    }
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Created periodic task (interval: " + 
                        std::to_string(_readInterval) + "s, priority: IDLE)");
    return true;
}

void SensorBase::stopPeriodicTask()
{
    if (_taskHandle != nullptr) {
        TaskHandle_t taskToDelete = _taskHandle;
        _taskHandle = nullptr;  // Clear atomically to prevent double-delete
        
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Stopping periodic task");
        vTaskDelete(taskToDelete);
        
        // Small delay to let scheduler cleanup
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

SensorManager::SensorManager() : _enabled(false), _i2cInitialized(false)
{
}

SensorManager::~SensorManager()
{
    deinit();
}

bool SensorManager::init()
{
    if (!_enabled) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Sensor manager disabled");
        return true;
    }
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Initializing sensor manager...");
    
    bool anySuccess = false;
    bool anyFailure = false;
    
    for (auto& sensor : _sensors) {
        // Sensors are already initialized in initFromConfig, just start periodic tasks
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Starting periodic task for sensor: " + sensor->getName());
        
        // Start periodic task for sensors with custom intervals (> 0)
        if (sensor->getReadInterval() > 0) {
            if (!sensor->startPeriodicTask()) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to start periodic task for sensor: " + sensor->getName());
                anyFailure = true;
            } else {
                anySuccess = true;
            }
        } else {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Sensor " + sensor->getName() + " will follow flow interval (no periodic task)");
            anySuccess = true;
        }
    }
    
    // Log results
    if (_sensors.empty()) {
        if (_sensorErrors.empty()) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No sensors configured");
        } else {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "All configured sensors failed to initialize");
        }
    } else if (anyFailure) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Some sensors failed to start periodic tasks");
    } else {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "All sensors started successfully");
    }
    
    // Always return true to allow device to boot even with sensor errors
    return true;
}

bool SensorManager::initFromConfig(const std::string& configFile, const std::map<std::string, SensorConfig>& sensorConfigs)
{
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Initializing sensors from parsed configuration");
    
    // First, scan GPIO configuration to find sensor pins
    int sdaPin, sclPin, onewirePin;
    scanGPIOConfig(configFile, sdaPin, sclPin, onewirePin);
    
    // Clear any previous sensor errors
    _sensorErrors.clear();
    
    // Check if any sensor is enabled
    bool anySensorEnabled = false;
    for (const auto& pair : sensorConfigs) {
        if (pair.second.enable) {
            anySensorEnabled = true;
            break;
        }
    }
    
    if (!anySensorEnabled) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No sensors enabled in configuration");
        _enabled = false;
        return true;
    }
    
    _enabled = true;
    
    // Initialize SHT3x sensor if configured
    auto sht3xIt = sensorConfigs.find("SHT3x");
    if (sht3xIt != sensorConfigs.end() && sht3xIt->second.enable) {
        const SensorConfig& config = sht3xIt->second;
        
        if (sdaPin >= 0 && sclPin >= 0) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Attempting to initialize SHT3x sensor...");
            
            // Try to initialize I2C bus
            bool i2cSuccess = false;
            for (int retry = 0; retry < SENSOR_INIT_RETRY_COUNT; retry++) {
                if (retry > 0) {
                    int delayMs = 100 * retry;  // 100ms, 200ms
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "I2C init retry " + std::to_string(retry + 1) + 
                                        " after " + std::to_string(delayMs) + "ms");
                    vTaskDelay(pdMS_TO_TICKS(delayMs));
                }
                
                if (initI2C(sdaPin, sclPin, config.i2cFreq)) {
                    i2cSuccess = true;
                    break;
                }
            }
            
            if (!i2cSuccess) {
                addSensorError("SHT3x", SensorInitStatus::FAILED_BUS_INIT, 
                              "Failed to initialize I2C bus after " + std::to_string(SENSOR_INIT_RETRY_COUNT) + " retries",
                              SENSOR_INIT_RETRY_COUNT);
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SHT3x initialization aborted - I2C bus init failed");
                
                // Add delay after I2C failure to allow GPIO states to settle before DS18B20 init
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Waiting for GPIO states to stabilize after I2C failure...");
                vTaskDelay(pdMS_TO_TICKS(200));
            } else {
                // I2C bus is ready, create and initialize sensor with retry
                auto sensor = std::make_unique<SensorSHT3x>(
                    config.sht3xAddress,
                    config.mqttTopic,
                    config.influxMeasurement,
                    config.interval,
                    config.mqttEnable,
                    config.influxEnable
                );
                
                std::stringstream ss;
                ss << "0x" << std::hex << (int)config.sht3xAddress;
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Created SHT3x sensor (addr:" + ss.str() + 
                                    ", interval:" + (config.interval < 0 ? "follow flow" : std::to_string(config.interval) + "s") + ")");
                
                // Try to initialize the sensor with retries
                bool initSuccess = false;
                for (int retry = 0; retry < SENSOR_INIT_RETRY_COUNT; retry++) {
                    if (retry > 0) {
                        int delayMs = 100 * retry;  // 100ms, 200ms
                        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "SHT3x sensor init retry " + std::to_string(retry + 1) + 
                                            " after " + std::to_string(delayMs) + "ms");
                        vTaskDelay(pdMS_TO_TICKS(delayMs));
                    }
                    
                    if (sensor->init()) {
                        initSuccess = true;
                        break;
                    }
                }
                
                if (initSuccess) {
                    _sensors.push_back(std::move(sensor));
                    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "SHT3x sensor initialized successfully");
                } else {
                    addSensorError("SHT3x", SensorInitStatus::FAILED_NO_DEVICE,
                                  "Sensor not responding at address " + ss.str() + " after " + 
                                  std::to_string(SENSOR_INIT_RETRY_COUNT) + " retries",
                                  SENSOR_INIT_RETRY_COUNT);
                    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SHT3x sensor initialization failed");
                }
            }
        } else {
            addSensorError("SHT3x", SensorInitStatus::FAILED_OTHER,
                          "I2C pins not configured in GPIO section",
                          0);
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "SHT3x enabled but I2C pins not configured in GPIO section");
        }
    }
    
    // Initialize DS18B20 sensor if configured
    auto ds18b20It = sensorConfigs.find("DS18B20");
    if (ds18b20It != sensorConfigs.end() && ds18b20It->second.enable) {
        const SensorConfig& config = ds18b20It->second;
        
        if (onewirePin >= 0) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Attempting to initialize DS18B20 sensor...");
            
            // Add initial delay to allow system boot to stabilize before 1-Wire init
            vTaskDelay(pdMS_TO_TICKS(100));
            
            auto sensor = std::make_unique<SensorDS18B20>(
                (gpio_num_t)onewirePin,
                config.mqttTopic,
                config.influxMeasurement,
                config.interval,
                config.mqttEnable,
                config.influxEnable,
                config.expectedSensors
            );
            
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Created DS18B20 sensor (GPIO:" + std::to_string(onewirePin) + 
                                ", interval:" + (config.interval < 0 ? "follow flow" : std::to_string(config.interval) + "s") + ")");
            
            // Try to initialize the sensor with retries
            bool initSuccess = false;
            for (int retry = 0; retry < SENSOR_INIT_RETRY_COUNT; retry++) {
                if (retry > 0) {
                    // Longer delays: 200ms, 400ms (for retries 2 and 3)
                    int delayMs = 200 * retry;
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "DS18B20 sensor init retry " + std::to_string(retry + 1) + 
                                        " after " + std::to_string(delayMs) + "ms");
                    vTaskDelay(pdMS_TO_TICKS(delayMs));
                }
                
                if (sensor->init()) {
                    initSuccess = true;
                    break;
                }
            }
            
            if (initSuccess) {
                _sensors.push_back(std::move(sensor));
                LogFile.WriteToFile(ESP_LOG_INFO, TAG, "DS18B20 sensor initialized successfully");
            } else {
                addSensorError("DS18B20", SensorInitStatus::FAILED_NO_DEVICE,
                              "No DS18B20 devices found on GPIO" + std::to_string(onewirePin) + 
                              " after " + std::to_string(SENSOR_INIT_RETRY_COUNT) + " retries",
                              SENSOR_INIT_RETRY_COUNT);
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "DS18B20 sensor initialization failed");
            }
        } else {
            addSensorError("DS18B20", SensorInitStatus::FAILED_OTHER,
                          "1-Wire pin not configured in GPIO section",
                          0);
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "DS18B20 enabled but 1-Wire pin not configured in GPIO section");
        }
    }
    
    // Start periodic tasks for sensors with custom intervals
    for (auto& sensor : _sensors) {
        if (sensor->getReadInterval() > 0) {
            if (!sensor->startPeriodicTask()) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to start periodic task for sensor: " + sensor->getName());
            }
        }
    }
    
    // Log summary
    if (_sensors.empty() && _sensorErrors.empty()) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No sensors configured");
    } else if (_sensors.empty() && !_sensorErrors.empty()) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "All sensors failed to initialize - device will continue to boot");
    } else if (!_sensorErrors.empty()) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Some sensors failed to initialize - " + 
                            std::to_string(_sensors.size()) + " sensor(s) working, " +
                            std::to_string(_sensorErrors.size()) + " sensor(s) failed");
    } else {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "All " + std::to_string(_sensors.size()) + 
                            " sensor(s) initialized successfully");
    }
    
    return true;  // Always return true to allow device to continue booting
}

void SensorManager::update(int flowInterval)
{
    if (!_enabled) {
        return;
    }
    
    for (auto& sensor : _sensors) {
        // For sensors with custom intervals, their periodic tasks handle reading
        // Only process "follow flow" sensors here (interval = -1)
        if (sensor->getReadInterval() > 0) {
            continue;
        }
        
        // Check if we should start a new read
        if (sensor->shouldRead(flowInterval)) {
            // Start async read (spawns ephemeral background task)
            // readData() returns immediately - conversion happens in background
            // The background task will:
            // 1. Poll hardware with vTaskDelay() yields (power efficient)
            // 2. Update sensor data when complete
            // 3. Publish to MQTT/InfluxDB
            // 4. Self-terminate via vTaskDelete(NULL)
            sensor->readData();
        }
    }
}

void SensorManager::deinit()
{
    // Stop all periodic tasks before clearing sensors
    for (auto& sensor : _sensors) {
        sensor->stopPeriodicTask();
    }
    
    _sensors.clear();
    
    if (_i2cInitialized) {
        i2c_driver_delete(I2C_NUM_0);
        _i2cInitialized = false;
    }
}

bool SensorManager::initI2C(int sda, int scl, uint32_t freq)
{
    if (_i2cInitialized) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "I2C already initialized");
        return true;
    }
    
    // First, try to delete any existing driver (in case of previous failed init)
    // This is safe - if no driver exists, it returns ESP_ERR_INVALID_STATE which we ignore
    esp_err_t err = i2c_driver_delete(I2C_NUM_0);
    if (err == ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Deleted existing I2C driver before reinit");
        vTaskDelay(pdMS_TO_TICKS(10));  // Small delay after deletion
    }
    
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)sda;
    conf.scl_io_num = (gpio_num_t)scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = freq;
    conf.clk_flags = 0; // Use default clock configuration
    
    err = i2c_param_config(I2C_NUM_0, &conf);
    if (err != ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "I2C param config failed: " + std::to_string(err));
        return false;
    }
    
    err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "I2C driver install failed: " + std::to_string(err));
        // If already installed, try to continue anyway
        if (err == ESP_ERR_INVALID_STATE) {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "I2C driver already installed, continuing...");
            _i2cInitialized = true;
            return true;
        }
        return false;
    }
    
    // Give the I2C bus time to stabilize after initialization
    vTaskDelay(pdMS_TO_TICKS(50));
    
    _i2cInitialized = true;
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "I2C initialized (SDA:" + std::to_string(sda) + 
                        ", SCL:" + std::to_string(scl) + ", Freq:" + std::to_string(freq) + ")");
    return true;
}

void SensorManager::scanGPIOConfig(const std::string& configFile, int& sdaPin, int& sclPin, int& onewirePin)
{
    sdaPin = -1;
    sclPin = -1;
    onewirePin = -1;
    
    std::ifstream ifs(configFile);
    if (!ifs.is_open()) {
        return;
    }
    
    std::string line;
    bool inGPIOSection = false;
    
    while (std::getline(ifs, line)) {
        line = trim(line);
        
        // Check for GPIO section
        if (line.find("[GPIO]") == 0) {
            inGPIOSection = true;
            continue;
        } else if (line[0] == '[') {
            inGPIOSection = false;
            continue;
        }
        
        if (!inGPIOSection || line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        // Parse GPIO configuration
        std::vector<std::string> parts = ZerlegeZeile(line);
        if (parts.size() < 2) {
            continue;
        }
        
        std::string param = toUpper(parts[0]);
        std::string value = toLower(parts[1]);
        
        // Extract GPIO number from param (e.g., "IO12" -> 12)
        int gpioNum = -1;
        if (param.find("IO") == 0) {
            if (!safeParseInt(param.substr(2), gpioNum)) {
                continue;
            }
        }
        
        // Check GPIO mode
        if (value == "i2c-sda") {
            sdaPin = gpioNum;
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Found I2C SDA on GPIO" + std::to_string(gpioNum));
        } else if (value == "i2c-scl") {
            sclPin = gpioNum;
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Found I2C SCL on GPIO" + std::to_string(gpioNum));
        } else if (value == "onewire") {
            onewirePin = gpioNum;
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Found 1-Wire on GPIO" + std::to_string(gpioNum));
        }
    }
    
    ifs.close();
}

std::string SensorManager::getJSON()
{
    if (!_enabled || (_sensors.empty() && _sensorErrors.empty())) {
        return "{}";
    }
    
    std::stringstream json;
    json << "{";
    
    // Add sensors data - each physical sensor is its own object
    json << "\"sensors\":[";
    
    bool first = true;
    for (const auto& sensor : _sensors) {
        // Add sensor-specific data
        if (sensor->getName() == "SHT3x") {
            auto* sht3x = static_cast<SensorSHT3x*>(sensor.get());
            if (sht3x) {
                if (!first) {
                    json << ",";
                }
                first = false;
                
                json << "{";
                json << "\"name\":\"SHT3x\"";
                json << ",\"id\":\"SHT3x-0x44\"";  // Could be made dynamic if we support multiple addresses
                json << ",\"status\":\"ok\"";
                json << ",\"temperature\":" << sht3x->getTemperature();
                json << ",\"humidity\":" << sht3x->getHumidity();
                json << ",\"unit_temp\":\"°C\"";
                json << ",\"unit_humidity\":\"%\"";
                json << ",\"last_read\":" << sensor->getLastReadTime();
                json << "}";
            }
        } else if (sensor->getName() == "DS18B20") {
            auto* ds18b20 = static_cast<SensorDS18B20*>(sensor.get());
            if (ds18b20) {
                int count = ds18b20->getSensorCount();
                // Create a separate object for each DS18B20 sensor on the bus
                for (int i = 0; i < count; i++) {
                    if (!first) {
                        json << ",";
                    }
                    first = false;
                    
                    json << "{";
                    json << "\"name\":\"DS18B20\"";
                    json << ",\"id\":\"" << ds18b20->getRomId(i) << "\"";
                    json << ",\"status\":\"ok\"";
                    json << ",\"temperature\":" << ds18b20->getTemperature(i);
                    json << ",\"unit\":\"°C\"";
                    json << ",\"last_read\":" << sensor->getLastReadTime();
                    json << "}";
                }
            }
        }
    }
    
    json << "]";
    
    // Add sensor errors
    if (!_sensorErrors.empty()) {
        json << ",\"errors\":[";
        first = true;
        for (const auto& error : _sensorErrors) {
            if (!first) {
                json << ",";
            }
            first = false;
            
            json << "{";
            json << "\"name\":\"" << error.sensorName << "\"";
            json << ",\"status\":\"";
            switch (error.status) {
                case SensorInitStatus::FAILED_BUS_INIT:
                    json << "bus_init_failed";
                    break;
                case SensorInitStatus::FAILED_NO_DEVICE:
                    json << "no_device";
                    break;
                case SensorInitStatus::FAILED_OTHER:
                    json << "config_error";
                    break;
                default:
                    json << "unknown";
                    break;
            }
            json << "\"";
            json << ",\"message\":\"" << error.errorMessage << "\"";
            json << ",\"retry_count\":" << error.retryCount;
            json << "}";
        }
        json << "]";
    }
    
    json << "}";
    
    return json.str();
}

void SensorManager::addSensorError(const std::string& sensorName, SensorInitStatus status, 
                                   const std::string& errorMessage, int retryCount)
{
    SensorError error;
    error.sensorName = sensorName;
    error.status = status;
    error.errorMessage = errorMessage;
    error.retryCount = retryCount;
    _sensorErrors.push_back(error);
    
    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Sensor error recorded: " + sensorName + 
                        " - " + errorMessage + " (retries: " + std::to_string(retryCount) + ")");
}
