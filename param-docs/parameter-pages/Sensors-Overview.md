# External Sensors Overview

> **🚀 Quick Start**: To enable sensors, you must first configure GPIO pins in the **Advanced Settings** section. See [Configuration](#configuration) below for step-by-step instructions.

## Device Health & Safety Monitoring

External sensors can be connected to monitor the environmental conditions around your AI-on-the-edge-device. This is **especially critical for outdoor installations** where temperature and humidity can indicate imminent device failure.

## Supported Sensors

### SHT3x (Temperature & Humidity Sensor)
- **Interface**: I²C
- **Measures**: Temperature (-40°C to +125°C) and Relative Humidity (0-100%)
- **Use Cases**:
  - Monitor enclosure humidity to detect condensation risks
  - Track temperature to prevent overheating
  - Early warning system for adverse weather conditions
  - Validate proper enclosure ventilation

### DS18B20 (Temperature Sensor)
- **Interface**: 1-Wire (Dallas/Maxim)
- **Measures**: Temperature (-55°C to +125°C)
- **Chainable**: Multiple sensors on one wire, each identified by unique ROM ID
- **Use Cases**:
  - Compare inside vs. outside enclosure temperature
  - Monitor multiple temperature points (camera, power supply, ambient)
  - Detect thermal issues before component failure
  - Track diurnal temperature variations

## Why Monitor Your Device?

### 🔴 Failure Prevention
- **Humidity > 80%**: Risk of condensation on electronics → short circuits
- **Temperature > 70°C**: Component stress → reduced lifespan
- **Temperature < -10°C**: LCD display issues, battery problems
- **Rapid temperature changes**: Thermal stress on solder joints

### 🟡 Early Warning Signs
- Gradual temperature increase → cooling fan failure or dust buildup
- Increasing humidity trends → seal degradation
- Temperature spikes during operation → inadequate power supply

### 🟢 Proactive Maintenance
- Schedule enclosure cleaning based on temperature trends
- Replace desiccant packs when humidity rises
- Verify heater/cooling systems are working
- Optimize placement based on environmental data

## Example Use Cases

### Outdoor Water Meter Enclosure
**Challenge**: Device exposed to rain, snow, extreme temperatures  
**Solution**: SHT3x inside enclosure + DS18B20 outside  
**Benefit**: Get alerts when conditions approach failure thresholds

### Unheated Garage Installation  
**Challenge**: Wide temperature swings, potential freezing  
**Solution**: DS18B20 chain monitoring ambient, enclosure, and camera temps  
**Benefit**: Know when to activate heater, detect component issues

### Roof-Mounted Installation
**Challenge**: Direct sun exposure causing overheating  
**Solution**: SHT3x monitoring enclosure climate  
**Benefit**: Data to optimize ventilation, prevent thermal shutdowns

## Configuration

Sensors are configured in **three parts** (in this order):

### 1. GPIO Pin Assignment (REQUIRED FIRST STEP)

> **⚠️ Important**: GPIO configuration settings are **hidden by default** in the web interface under "Advanced Settings" or "Expert Options" to prevent accidental misconfiguration.

**To access GPIO settings:**
1. Navigate to the **Configuration** page
2. Scroll to the **GPIO** section
3. Click **"Show Expert Options"** or **"Advanced Settings"**
4. Configure the required GPIO pins:
   - **For DS18B20**: Set one GPIO pin (e.g., IO3) to `onewire` mode *
   - **For SHT3x**: Set two GPIO pins (e.g., IO3 to `i2c-sda`, IO1 to `i2c-scl`) *
5. Save and restart if required

**CRITICAL**: Do NOT use GPIO12 for sensors! It's a boot strapping pin that will prevent boot when pull-up resistors are connected.

\* Using GPIO1/GPIO3 will disable USB serial logging

**Detailed GPIO Configuration Guides:**
- [GPIO 1-Wire Configuration](GPIO/OneWire.md) - For DS18B20 sensors
- [GPIO I²C SDA Configuration](GPIO/I2C-SDA.md) - For SHT3x data line
- [GPIO I²C SCL Configuration](GPIO/I2C-SCL.md) - For SHT3x clock line

### 2. Physical Wiring

After configuring GPIO pins, connect your sensors with proper pull-up resistors:
- **DS18B20**: 4.7kΩ pull-up on DATA line
- **SHT3x**: 4.7kΩ pull-ups on both SDA and SCL lines
- **Power**: Always use 3.3V (NOT 5V for SHT3x!)

### 3. Sensor Parameter Configuration

Finally, configure sensor-specific settings in `[SHT3x]` or `[DS18B20]` sections:
- Enable the sensor
- Set reading interval (see Reading Intervals section below)
- Configure MQTT/InfluxDB publishing

See individual parameter documentation for details.

## Reading Intervals and Power Efficiency

The sensor system uses a power-efficient per-sensor task architecture:

### How It Works
- **Custom Intervals** (> 0): Each sensor gets its own FreeRTOS task that runs at the specified interval
  - Example: `Interval = 30` creates a dedicated task that reads the sensor every 30 seconds
  - Power efficient: CPU only wakes when that specific sensor needs to be read
- **Follow Flow Mode** (Interval = -1): Sensor is read during the main flow cycle
  - No dedicated task created
  - Reading happens at the same interval as the main device flow

### Examples

**High-Frequency Monitoring:**
```ini
[AutoTimer]
Interval = 10        ; Main flow runs every 10 minutes

[SHT3x]
Interval = 30        ; Dedicated task reads every 30 seconds
```
Result: 20 temperature/humidity readings per flow cycle, CPU wakes only when needed

**Follow Flow Mode:**
```ini
[AutoTimer]
Interval = 5         ; Main flow runs every 5 minutes

[DS18B20]
Interval = -1        ; Follow flow (default)
```
Result: 1 temperature reading per flow cycle, no extra CPU wake-ups

**Mixed Configuration:**
```ini
[AutoTimer]
Interval = 10        ; Main flow runs every 10 minutes

[SHT3x]
Interval = 60        ; Own task: reads every 1 minute

[DS18B20]
Interval = -1        ; Follow flow: reads every 10 minutes
```
Result: Optimal power usage with different monitoring frequencies per sensor

## Data Publishing

Sensor data is automatically published to:
- **MQTT**: For Home Assistant, Node-RED, or other automation systems
- **InfluxDB**: For long-term trending and analysis
- Both support configurable topics/measurements for easy integration

## Recommended Thresholds

Based on typical ESP32-CAM operating conditions:

| Metric | Good | Warning | Critical |
|--------|------|---------|----------|
| **Enclosure Temp** | 0-50°C | 50-65°C | >65°C |
| **Enclosure Humidity** | 20-60% | 60-80% | >80% |
| **Ambient Temp** | -20 to +40°C | Outside range | <-30°C or >50°C |

Set up alerts in Home Assistant or Grafana based on these thresholds.
