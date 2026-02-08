#include "ClassFlowSensors.h"
#include "ClassFlowControll.h"
#include "sensor_manager.h"
#include "ClassLogFile.h"
#include "../../include/defines.h"

#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <climits>

static const char *TAG = "FLOW_SENSORS";

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

ClassFlowSensors::ClassFlowSensors() : _initialized(false)
{
    SetInitialParameter();
}

ClassFlowSensors::ClassFlowSensors(std::vector<ClassFlow*>* lfc) : _initialized(false)
{
    SetInitialParameter();
    ListFlowControll = lfc;
}

ClassFlowSensors::ClassFlowSensors(std::vector<ClassFlow*>* lfc, ClassFlow *_prev) : _initialized(false)
{
    SetInitialParameter();
    previousElement = _prev;
    ListFlowControll = lfc;
}

ClassFlowSensors::~ClassFlowSensors()
{
    if (_sensorManager) {
        _sensorManager->deinit();
    }
}

void ClassFlowSensors::SetInitialParameter(void)
{
    disabled = false;
    _sensorManager = nullptr;
    _flowController = nullptr;
    _initialized = false;
    _configParsed = false;
}

bool ClassFlowSensors::ReadParameter(FILE* pfile, std::string& aktparamgraph)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "ReadParameter called");
    
    // Check if this is a sensor section ([SHT3x] or [DS18B20])
    aktparamgraph = trim(aktparamgraph);
    
    if (aktparamgraph.size() == 0) {
        if (!this->GetNextParagraph(pfile, aktparamgraph)) {
            return false;
        }
    }
    
    std::string upperGraph = toUpper(aktparamgraph);
    std::string sensorType;
    
    if (upperGraph.compare("[SHT3X]") == 0) {
        sensorType = "SHT3x";
    } else if (upperGraph.compare("[DS18B20]") == 0) {
        sensorType = "DS18B20";
    } else {
        // Not a sensor section
        return false;
    }
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Found sensor section: " + aktparamgraph);
    
    // Get or create configuration for this sensor type
    SensorConfig& config = _sensorConfigs[sensorType];
    
    // Section found uncommented - enable the sensor (section comment/uncomment is the way to enable/disable)
    config.enable = true;
    
    // Set default InfluxDB measurement if not already set
    // Note: mqttTopic is intentionally left empty so sensors use main MQTT topic by default
    if (config.influxMeasurement.empty()) {
        config.influxMeasurement = "environment";
    }
    
    // Parse parameters from the file
    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph)) {
        std::vector<std::string> splitted = ZerlegeZeile(aktparamgraph);
        if (splitted.size() < 2) {
            continue;
        }
        
        std::string param = toUpper(splitted[0]);
        std::string value = splitted[1];
        
        if (param == "INTERVAL") {
            if (!safeParseInt(value, config.interval)) {
                LogFile.WriteToFile(ESP_LOG_WARN, TAG, sensorType + ": Invalid interval value: " + value);
            }
        } else if (param == "MQTT_ENABLE") {
            config.mqttEnable = (toUpper(value) == "TRUE" || value == "1");
        } else if (param == "MQTT_TOPIC") {
            config.mqttTopic = value;
        } else if (param == "INFLUXDB_ENABLE") {
            config.influxEnable = (toUpper(value) == "TRUE" || value == "1");
        } else if (param == "INFLUXDB_MEASUREMENT") {
            config.influxMeasurement = value;
        } else if (sensorType == "SHT3x") {
            // SHT3x-specific parameters
            if (param == "ADDRESS") {
                unsigned long tempAddress;
                // Support both hex (0x44) and decimal (68) formats
                int base = 0; // auto-detect base
                if (value.find("0x") == 0 || value.find("0X") == 0) {
                    base = 16;
                }
                if (safeParseULong(value, tempAddress, base)) {
                    if (tempAddress <= 0xFF) {
                        config.sht3xAddress = static_cast<uint8_t>(tempAddress);
                    } else {
                        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "SHT3x: Address out of range: " + value);
                    }
                } else {
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "SHT3x: Invalid address value: " + value);
                }
            } else if (param == "I2C_FREQUENCY") {
                unsigned long tempFreq;
                if (safeParseULong(value, tempFreq, 10)) {
                    if (tempFreq <= UINT32_MAX) {
                        config.i2cFreq = static_cast<uint32_t>(tempFreq);
                    } else {
                        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "SHT3x: I2C frequency out of range: " + value);
                    }
                } else {
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "SHT3x: Invalid I2C frequency value: " + value);
                }
            }
        } else if (sensorType == "DS18B20") {
            // DS18B20-specific parameters
            if (param == "EXPECTEDSENSORS") {
                if (safeParseInt(value, config.expectedSensors)) {
                    // Validate: must be -1 (auto-detect) or positive integer (>0)
                    if (config.expectedSensors < -1 || config.expectedSensors == 0) {
                        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "DS18B20: ExpectedSensors must be -1 (auto-detect) or positive, got: " + value);
                        config.expectedSensors = -1;  // Fallback to auto-detect
                    }
                } else {
                    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "DS18B20: Invalid ExpectedSensors value: " + value);
                }
            }
        }
    }
    
    _configParsed = true;
    
    return true;
}

void ClassFlowSensors::initializeEarly()
{
    // Check if we have parsed configuration
    if (!_configParsed) {
        // No sensor sections found in config - nothing to initialize
        return;
    }
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Starting early sensor initialization");
    
    // Check if already initialized
    if (_initialized) {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Sensors already initialized - skipping early initialization");
        return;
    }
    
    if (disabled) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Sensors disabled - skipping early initialization");
        return;
    }
    
    // Create sensor manager
    _sensorManager = std::make_unique<SensorManager>();
    
    // Pass the parsed configuration to the sensor manager
    if (!_sensorManager->initFromConfig(CONFIG_FILE, _sensorConfigs)) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to initialize sensors during early init");
        // Still mark as initialized to prevent double initialization
        _initialized = true;
        return;
    }
    
    _initialized = true;
    
    // Log initialization summary
    if (_sensorManager->hasSensorErrors()) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Sensors initialized with errors - check logs for details");
    } else if (_sensorManager->getSensors().empty()) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No sensors configured");
    } else {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "All sensors initialized successfully during early init");
    }
    
    // Perform first sensor reading if sensors are enabled
    if (_sensorManager && _sensorManager->isEnabled()) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Performing first sensor reading");
        
        // Get flow interval (use 0 for immediate read on first call)
        int flowIntervalSeconds = 0;
        if (_flowController) {
            float intervalMinutes = _flowController->getAutoInterval();
            flowIntervalSeconds = (int)(intervalMinutes * 60);
        }
        
        // Perform first update
        _sensorManager->update(flowIntervalSeconds);
        
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "First sensor reading completed");
    }
}

bool ClassFlowSensors::doFlow(std::string time)
{
    if (disabled) {
        return true;
    }
    
    // Initialize on first run if we have configuration and not yet initialized
    // (This handles the case where initializeEarly wasn't called)
    if (!_initialized && _configParsed) {
        // Create sensor manager
        _sensorManager = std::make_unique<SensorManager>();
        
        // Pass the parsed configuration to the sensor manager
        if (!_sensorManager->initFromConfig(CONFIG_FILE, _sensorConfigs)) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to initialize sensors");
            // Still return true to allow device to continue booting
        }
        
        _initialized = true;
        
        // Log initialization summary
        if (_sensorManager->hasSensorErrors()) {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Sensors initialized with errors - check logs for details");
        } else if (_sensorManager->getSensors().empty()) {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "No sensors configured");
        } else {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "All sensors initialized successfully");
        }
    }
    
    if (!_sensorManager || !_sensorManager->isEnabled()) {
        return true;
    }
    
    // Get the flow interval from the controller for "follow flow" mode
    // The AutoInterval is in minutes, we need to convert to seconds
    int flowIntervalSeconds = 0;
    
    if (_flowController) {
        float intervalMinutes = _flowController->getAutoInterval();
        flowIntervalSeconds = (int)(intervalMinutes * 60);  // Convert minutes to seconds
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Using flow interval: " + 
                            std::to_string(intervalMinutes) + " min (" + 
                            std::to_string(flowIntervalSeconds) + " sec)");
    } else {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Flow controller not set, using default interval");
    }
    
    // Update sensors that are in "follow flow" mode (interval = -1)
    // Sensors with custom intervals are handled by their own periodic tasks
    _sensorManager->update(flowIntervalSeconds);
    
    return true;
}
