# DS18B20 MQTT Topic

Set the base MQTT topic for publishing DS18B20 temperature readings.

## Value
Default: (empty - uses `MainTopic/ds18b20`)

String value representing the base topic path. Each sensor's ROM ID is appended to create unique topics.

Variables like `$main_topic` can be used for dynamic topic construction.

## Description

The MQTT topic defines where temperature readings are published. For multiple DS18B20 sensors, each sensor publishes to a unique subtopic using its ROM ID.

**Important**: This topic is also used by Home Assistant Discovery (HAD) to advertise sensors. If you configure a custom MQTT topic, HAD will use it to ensure Home Assistant can find your sensor data.

## Topic Structure

### Format
```
{MQTT_Topic}/{ROM_ID}/temperature
```

Where:
- `MQTT_Topic`: This parameter (DS18B20 base topic, defaults to `MainTopic/ds18b20` if empty)
- `ROM_ID`: Unique 64-bit ROM ID of each sensor (format: 28-XXXXXXXXXXXX)

### Examples

**Default Configuration (empty MQTT_Topic):**
```ini
[MQTT]
MainTopic = watermeter

[DS18B20]
MQTT_Topic = 
```

Publishes to:
```
watermeter/ds18b20/28-0123456789AB/temperature
```

**Custom Topic:**
```ini
[MQTT]
MainTopic = watermeter

[DS18B20]
MQTT_Topic = /qwe/ds18b
```

Publishes to:
```
/qwe/ds18b/28-0123456789AB/temperature
```

**Using Variable Substitution:**
```ini
[MQTT]
MainTopic = watermeter

[DS18B20]
MQTT_Topic = $main_topic/sensors/temperature
```

Publishes to:
```
watermeter/sensors/temperature/28-0123456789AB/temperature
```

**Multiple Sensors (3 sensors):**
```
watermeter/ds18b20/28-0123456789AB/temperature → 24.5  (Inside)
watermeter/ds18b20/28-FEDCBA987654/temperature → 15.2  (Outside)
watermeter/ds18b20/28-AABBCCDD1122/temperature → 12.8  (Pipe)
```

## Topic Naming Conventions

### Recommended Patterns

**By Location:**
```
enclosure/temperature
outdoor/temperature
basement/temperature
garage/temperature
```

**By Purpose:**
```
sensors/temperature
environment/temperature
monitoring/temperature
```

**By Device:**
```
watermeter/temperature
gasmeter/temperature
device/temperature
```

### Hierarchical Topics

Use hierarchy for organization:

**Option 1: By Location Hierarchy**
```
home/basement/watermeter/temperature
home/garage/gasmeter/temperature
```

**Option 2: By System Hierarchy**
```
monitoring/watermeter/enclosure/temperature
monitoring/watermeter/pipe/temperature
```

**Option 3: By Sensor Type**
```
sensors/temperature/enclosure
sensors/temperature/outdoor
sensors/humidity/enclosure
```

## Home Assistant Integration

### Auto-Discovery

When **Home Assistant Discovery** is enabled in MQTT settings, sensors are automatically discovered using the configured MQTT topic:

```ini
[MQTT]
MainTopic = watermeter
HomeassistantDiscovery = true

[DS18B20]
MQTT_Topic = /custom/sensors/temp
```

Home Assistant will automatically create sensors at:
```
sensor.custom_sensors_temp_ds18b20_28_0123456789ab_temperature
```

**Important**: The discovery announcement uses your custom `MQTT_Topic` to match where data is actually published. If you change `MQTT_Topic`, you must:
1. Restart the device to regenerate HAD announcements
2. Or trigger republishing via the web interface (`Settings > MQTT > Publish Discovery`)

### Manual Configuration

Match your topic in Home Assistant:

```yaml
mqtt:
  sensor:
    - name: "Watermeter Enclosure Inside"
      state_topic: "watermeter/enclosure/temperature/28-0123456789AB"
      unit_of_measurement: "°C"
      device_class: temperature
      
    - name: "Watermeter Enclosure Outside"
      state_topic: "watermeter/enclosure/temperature/28-FEDCBA987654"
      unit_of_measurement: "°C"
      device_class: temperature
      
    - name: "Watermeter Pipe"
      state_topic: "watermeter/enclosure/temperature/28-AABBCCDD1122"
      unit_of_measurement: "°C"
      device_class: temperature
```

### Topic Wildcards

You can use MQTT wildcards for monitoring:

**Subscribe to all sensors:**
```
watermeter/enclosure/temperature/#
```

**Subscribe to all temperature topics:**
```
watermeter/+/temperature/#
```

## Best Practices

### DO:
- ✅ Use descriptive names (e.g., `enclosure/temperature` not `temp1`)
- ✅ Use lowercase for consistency
- ✅ Use forward slashes for hierarchy
- ✅ Keep topics reasonably short
- ✅ Group related sensors under common parent

### DON'T:
- ❌ Use spaces (use underscores or hyphens)
- ❌ Use special characters (stick to alphanumeric, `-`, `_`, `/`)
- ❌ Make overly deep hierarchies (3-4 levels is enough)
- ❌ Use leading or trailing slashes
- ❌ Include ROM IDs in base topic (system adds them automatically)

## Multiple Device Installations

If you have multiple devices (multiple water meters), use device-specific topics:

**Device 1: Main Watermeter**
```ini
[MQTT]
MainTopic = home/watermeter1

[DS18B20]
MQTT_Topic = enclosure/temperature
```

**Device 2: Secondary Watermeter**
```ini
[MQTT]
MainTopic = home/watermeter2

[DS18B20]
MQTT_Topic = enclosure/temperature
```

Results in:
```
home/watermeter1/enclosure/temperature/28-XXXX → 24.5
home/watermeter2/enclosure/temperature/28-YYYY → 22.1
```

## Topic Organization Examples

### Example 1: Simple Installation
**One device, multiple sensors in same enclosure**

```ini
[DS18B20]
MQTT_Topic = temperature
```

Topics:
```
watermeter/temperature/28-0123456789AB
watermeter/temperature/28-FEDCBA987654
```

### Example 2: Detailed Location Tracking
**Multiple measurement points**

Use descriptive topics if you want to emphasize location:

```ini
# For sensors you can identify by physical location
MQTT_Topic = sensors/temperature
```

Then in Home Assistant, name each sensor by its location:
```yaml
- name: "Enclosure Inside Temp"
  state_topic: "watermeter/sensors/temperature/28-0123456789AB"
  
- name: "Enclosure Outside Temp"
  state_topic: "watermeter/sensors/temperature/28-FEDCBA987654"
  
- name: "Inlet Pipe Temp"
  state_topic: "watermeter/sensors/temperature/28-AABBCCDD1122"
```

### Example 3: Multi-Sensor System
**Both DS18B20 and SHT3x sensors**

```ini
[SHT3x]
MQTT_Topic = climate

[DS18B20]
MQTT_Topic = temperature
```

Clear separation:
```
watermeter/climate/temperature → 24.5°C (from SHT3x)
watermeter/climate/humidity → 45%
watermeter/temperature/28-0123456789AB → 24.3°C (from DS18B20 #1)
watermeter/temperature/28-FEDCBA987654 → 15.2°C (from DS18B20 #2)
```

## Finding ROM IDs

To configure Home Assistant, you need to know each sensor's ROM ID.

**Method 1: Check Device Logs**
Look for startup messages:
```
[INFO] DS18B20: Sensor #1: 28-0123456789AB
[INFO] DS18B20: Sensor #2: 28-FEDCBA987654
[INFO] DS18B20: Sensor #3: 28-AABBCCDD1122
```

**Method 2: Use MQTT Explorer**
- Connect to your MQTT broker
- Browse to your base topic
- See all published subtopics with ROM IDs

**Method 3: Check Info Page**
The device Info page shows discovered sensors with ROM addresses.

## Changing Topics

**Important**: If you change MQTT_Topic:
1. Update Home Assistant sensor configurations
2. Old topics will no longer receive updates
3. Consider using retained messages so last value persists

**Migration Strategy:**
1. Note old topic names and ROM IDs
2. Change MQTT_Topic in config
3. Update Home Assistant configuration.yaml
4. Reload MQTT integration
5. Verify new topics are working
6. Remove old topics from broker if needed

## Performance Impact

MQTT topic length has minimal impact:
- `temperature` vs `enclosure/temperature/monitoring` - negligible difference
- Focus on clarity over brevity
- Typical message: topic + value = 50-100 bytes total

## Troubleshooting

**Topics not appearing:**
1. Verify MQTT_Enable is true
2. Check MQTT broker connection
3. Use MQTT Explorer to monitor broker
4. Look for errors in device logs

**Wrong topic structure:**
- Verify you're setting base topic only (ROM ID is added automatically)
- Check for typos in topic name
- Ensure no leading/trailing slashes

**Home Assistant not finding sensors:**
- ROM IDs in configuration must match actual sensor ROM IDs
- Check case sensitivity (topics are case-sensitive)
- Reload MQTT integration after changes

**Topics clashing with other devices:**
- Use unique MainTopic for each device
- Or use unique MQTT_Topic per device
- Ensure devices don't share the same full topic path

## Related Parameters

- [DS18B20 Enable](Enable.md) - Enable the sensor
- [DS18B20 Interval](Interval.md) - Reading interval
- [DS18B20 MQTT Enable](MQTT_Enable.md) - Enable MQTT publishing
- [MQTT MainTopic](../MQTT/MainTopic.md) - Global topic prefix
- [MQTT Enable](../MQTT/Enable.md) - Global MQTT configuration

## Example Complete Configuration

```ini
[MQTT]
Enable = true
Uri = mqtt://192.168.1.100
User = mqtt_user
Password = mqtt_pass
MainTopic = home/watermeter
ClientName = watermeter-device

[GPIO]
IO3 = onewire               ; * Requires disabling USB logging

[DS18B20]
Enable = true
Interval = 300
MQTT_Enable = true
MQTT_Topic = enclosure/temperature
```

Resulting topics:
```
home/watermeter/enclosure/temperature/28-0123456789AB → 24.5
home/watermeter/enclosure/temperature/28-FEDCBA987654 → 15.2
home/watermeter/enclosure/temperature/28-AABBCCDD1122 → 12.8
```
