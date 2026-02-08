# SHT3x MQTT Topic

Set the base MQTT topic for publishing SHT3x temperature and humidity readings.

## Value
Default: (empty - uses `MainTopic/sht3x`)

String value representing the base topic path. Temperature and humidity are published to subtopics.

Variables like `$main_topic` can be used for dynamic topic construction.

## Description

The MQTT topic defines where temperature and humidity readings are published. The SHT3x publishes two separate values to subtopics under this base topic.

**Important**: This topic is also used by Home Assistant Discovery (HAD) to advertise sensors. If you configure a custom MQTT topic, HAD will use it to ensure Home Assistant can find your sensor data.

## Topic Structure

### Format
```
{MQTT_Topic}/temperature → temperature_value
{MQTT_Topic}/humidity → humidity_value
```

Where:
- `MQTT_Topic`: This parameter (SHT3x base topic, defaults to `MainTopic/sht3x` if empty)
- `/temperature`: Subtopic for temperature readings
- `/humidity`: Subtopic for humidity readings

### Examples

**Default Configuration (empty MQTT_Topic):**
```ini
[MQTT]
MainTopic = watermeter

[SHT3x]
MQTT_Topic = 
```

Publishes to:
```
watermeter/sht3x/temperature → 24.5
watermeter/sht3x/humidity → 45.2
```

**Custom Topic:**
```ini
[MQTT]
MainTopic = watermeter

[SHT3x]
MQTT_Topic = /custom/climate/sensor
```

Publishes to:
```
/custom/climate/sensor/temperature → 24.5
/custom/climate/sensor/humidity → 45.2
```

**Using Variable Substitution:**
```ini
[MQTT]
MainTopic = watermeter

[SHT3x]
MQTT_Topic = $main_topic/enclosure/climate
```

Publishes to:
```
watermeter/enclosure/climate/temperature → 24.5
watermeter/enclosure/climate/humidity → 45.2
```

## Topic Naming Conventions

### Recommended Patterns

**By Location:**
```
enclosure/climate           - Enclosure environmental conditions
outdoor/weather             - Outdoor weather station
basement/environment        - Basement conditions
garage/climate              - Garage climate
attic/environment           - Attic monitoring
```

**By Purpose:**
```
sensors/climate             - Generic climate monitoring
environment/indoor          - Indoor environmental data
monitoring/climate          - Climate monitoring system
device/health               - Device health monitoring
```

**By Device Type:**
```
watermeter/environment      - Watermeter enclosure
gasmeter/climate            - Gas meter enclosure
powermeter/environment      - Power meter enclosure
```

### Hierarchical Organization

**Option 1: By Location Hierarchy**
```
building/floor/room/climate
home/basement/watermeter/climate
facility/outdoor/meter/environment
```

**Option 2: By System Hierarchy**
```
meters/water/enclosure/climate
meters/gas/enclosure/climate
monitors/environment/indoor/climate
```

**Option 3: By Function Hierarchy**
```
monitoring/environmental/enclosure
monitoring/health/device/climate
systems/metering/environment
```

## Home Assistant Integration

### Auto-Discovery

When **Home Assistant Discovery** is enabled in MQTT settings, sensors are automatically discovered using the configured MQTT topic:

```ini
[MQTT]
MainTopic = watermeter
HomeassistantDiscovery = true

[SHT3x]
MQTT_Topic = /custom/climate
```

Home Assistant will automatically create sensors at:
```
sensor.custom_climate_sht3x_temperature
sensor.custom_climate_sht3x_humidity
```

**Important**: The discovery announcement uses your custom `MQTT_Topic` to match where data is actually published. If you change `MQTT_Topic`, you must:
1. Restart the device to regenerate HAD announcements
2. Or trigger republishing via the web interface (`Settings > MQTT > Publish Discovery`)

## Home Assistant Best Practices

### Descriptive Topics

Use topics that clearly indicate what's being measured:

**Good:**
```
enclosure/climate          → Clear: enclosure environmental conditions
watermeter/environment     → Clear: watermeter-related environment
device/health/climate      → Clear: device health climate data
```

**Avoid:**
```
sensor1                    → Unclear: what sensor? where?
data                       → Unclear: what data?
temp                       → Unclear: temperature where?
```

### Topic Length Considerations

**Optimal**: 2-4 levels deep
```
enclosure/climate          ✅ Good balance
device/enclosure/climate   ✅ Good balance
```

**Too shallow**:
```
climate                    ❌ Not specific enough
data                       ❌ Too generic
```

**Too deep**:
```
home/floor1/room3/device2/sensor1/climate/environmental/data  ❌ Excessive
```

## Advanced Configurations

### Multiple SHT3x Sensors (Future)

If multiple SHT3x sensors are supported in the future, use descriptive topics:

```ini
# Inside enclosure
[SHT3x_Inside]
MQTT_Topic = enclosure/inside/climate

# Outside enclosure
[SHT3x_Outside]
MQTT_Topic = enclosure/outside/climate
```

### Integration with Other Sensors

Coordinate topics across sensor types:

```ini
[SHT3x]
MQTT_Topic = enclosure/climate

[DS18B20]
MQTT_Topic = enclosure/temperature
```

Results in:
```
enclosure/climate/temperature     (SHT3x temperature)
enclosure/climate/humidity        (SHT3x humidity)
enclosure/temperature/28-XXX      (DS18B20 sensor)
```

## MQTT Explorer Organization

Good topic structure makes MQTT Explorer visualization clean:

```
watermeter/
├── enclosure/
│   ├── climate/
│   │   ├── temperature → 24.5
│   │   └── humidity → 45.2
│   └── temperature/
│       └── 28-0123456789AB → 22.3
└── meter/
    └── value → 123.456
```

## Home Assistant Device Grouping

Use consistent topic prefixes to group related sensors:

```yaml
# All enclosure sensors appear together
sensor.enclosure_climate_temperature
sensor.enclosure_climate_humidity
sensor.enclosure_temperature_inside
sensor.enclosure_temperature_outside
```

## Troubleshooting

**Topics not appearing in MQTT broker:**
1. Check global MQTT settings are correct
2. Verify SHT3x Enable = true
3. Confirm MQTT_Enable = true for SHT3x
4. Check device logs for publish errors
5. Verify MQTT broker allows topic creation

**Topics malformed:**
- Don't include leading `/` in topic (e.g., `/enclosure/climate` is wrong)
- Don't include trailing `/` in topic
- Use only alphanumeric, dash, underscore, and forward slash
- Avoid spaces (use underscore or dash instead)

**Home Assistant can't find sensors:**
1. Verify exact topic matches (case-sensitive)
2. Check for typos in configuration.yaml
3. Use MQTT Explorer to see actual published topics
4. Confirm MainTopic is correctly applied

## Migration Guide

### Changing Topic Structure

When changing topics, old data remains under old topics. To migrate:

**Step 1: Update configuration**
```ini
[SHT3x]
MQTT_Topic = new/topic/structure  # Changed from old/topic
```

**Step 2: Update Home Assistant**
```yaml
mqtt:
  sensor:
    - name: "Enclosure Temperature"
      state_topic: "watermeter/new/topic/structure/temperature"  # Updated
```

**Step 3: Clean up old topics** (optional)
- Old topics will naturally expire if retained messages have TTL
- Manually clear old topics via MQTT broker admin
- Or ignore - they don't cause issues unless broker has storage limits

## Related Parameters

- [SHT3x MQTT_Enable](MQTT_Enable.md) - Enable MQTT publishing
- [SHT3x Enable](Enable.md) - Enable the sensor
- [SHT3x Interval](Interval.md) - Set reading frequency
- [MQTT Enable](../MQTT/Enable.md) - Global MQTT configuration
- [MQTT MainTopic](../MQTT/MainTopic.md) - MQTT topic prefix
