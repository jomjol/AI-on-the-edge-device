# DS18B20 MQTT Enable

Enable or disable MQTT publishing for DS18B20 temperature sensor readings.

## Value
- `true` - Enable MQTT publishing (default)
- `false` - Disable MQTT publishing

## Description

When enabled, temperature readings from all DS18B20 sensors on the 1-Wire bus are published to MQTT broker. Each sensor publishes to a unique topic that includes its ROM ID.

## MQTT Topics

### Single Sensor
```
{MQTT_Topic}/{ROM_ID} → temperature_value
```

Example:
```
enclosure/temperature/28-0123456789AB → 24.5
```

### Multiple Sensors (Chainable)
Each sensor publishes to its own topic with ROM ID:

```
enclosure/temperature/28-0123456789AB → 24.5  (Sensor #1)
enclosure/temperature/28-FEDCBA987654 → 15.2  (Sensor #2)
enclosure/temperature/28-AABBCCDD1122 → 12.8  (Sensor #3)
```

The ROM ID uniquely identifies each physical sensor, allowing you to:
- Track specific sensor locations
- Maintain sensor identity even if bus order changes
- Configure per-sensor automations

## Message Format

**Topic**: `{base_topic}/{ROM_ID}`
**Payload**: Temperature value as string (e.g., "24.5")
**Retained**: Yes (last value persists)
**QoS**: 1 (at least once delivery)

## Home Assistant Integration

### Auto-Discovery (Manual Setup Required)

DS18B20 sensors require manual configuration in Home Assistant:

```yaml
# configuration.yaml
mqtt:
  sensor:
    - name: "Enclosure Inside Temperature"
      state_topic: "enclosure/temperature/28-0123456789AB"
      unit_of_measurement: "°C"
      device_class: temperature
      state_class: measurement
      
    - name: "Enclosure Outside Temperature"
      state_topic: "enclosure/temperature/28-FEDCBA987654"
      unit_of_measurement: "°C"
      device_class: temperature
      state_class: measurement
      
    - name: "Pipe Temperature"
      state_topic: "enclosure/temperature/28-AABBCCDD1122"
      unit_of_measurement: "°C"
      device_class: temperature
      state_class: measurement
```

### Finding ROM IDs

Check device Info page or logs during initialization:
```
[INFO] DS18B20: Sensor #1: 28-0123456789AB
[INFO] DS18B20: Sensor #2: 28-FEDCBA987654
[INFO] DS18B20: Sensor #3: 28-AABBCCDD1122
```

Or use MQTT Explorer to see published topics.

## Use Cases

### Device Health Monitoring
```ini
[DS18B20]
Enable = true
Interval = 300       ; Every 5 minutes
MQTT_Enable = true
MQTT_Topic = watermeter/enclosure/temperature
```

Home Assistant automation:
```yaml
automation:
  - alias: "Watermeter Overheat Alert"
    trigger:
      platform: numeric_state
      entity_id: sensor.enclosure_inside_temperature
      above: 65
      for: "00:10:00"
    action:
      - service: notify.mobile_app
        data:
          message: "⚠️ Watermeter overheating: {{ states('sensor.enclosure_inside_temperature') }}°C"
```

### Multi-Point Comparison
```yaml
automation:
  - alias: "Temperature Gradient Warning"
    trigger:
      platform: template
      value_template: >
        {{ (states('sensor.enclosure_inside_temperature') | float - 
            states('sensor.enclosure_outside_temperature') | float) > 25 }}
    action:
      - service: notify.mobile_app
        data:
          message: "High temperature gradient detected - check ventilation"
```

### Freeze Protection
```yaml
automation:
  - alias: "Pipe Freeze Alert"
    trigger:
      platform: numeric_state
      entity_id: sensor.pipe_temperature
      below: 2
      for: "00:05:00"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.pipe_heater
      - service: notify.mobile_app
        data:
          message: "Pipe heater activated - temperature: {{ states('sensor.pipe_temperature') }}°C"
```

## When to Disable

Disable MQTT publishing if:
- **InfluxDB only**: You only use InfluxDB for logging
- **MQTT not configured**: No MQTT broker available
- **Reduce traffic**: Minimize network activity (use InfluxDB instead)
- **Local only**: Device monitoring via web interface only

You can use:
- MQTT only: Quick alerts, real-time Home Assistant integration
- InfluxDB only: Historical analysis, Grafana dashboards
- Both: Full monitoring stack (alerts + history)

## MQTT Broker Requirements

**Minimum:**
- MQTT broker accessible on network
- Credentials configured in MQTT settings
- QoS 1 support

**Recommended:**
- Retained message support for last value persistence
- Sufficient message throughput for your interval

**Example Brokers:**
- Mosquitto (local or remote)
- Home Assistant's built-in MQTT broker
- CloudMQTT / HiveMQ Cloud
- AWS IoT Core / Azure IoT Hub

## Performance Impact

**Minimal:**
- Message size: ~10-20 bytes per sensor
- Frequency: Once per interval (e.g., every 10 minutes)
- Network: <1 KB/hour for 3 sensors at 10-min intervals

Even with multiple sensors and short intervals, MQTT overhead is negligible.

## Troubleshooting

**Messages not appearing:**
1. Verify MQTT broker is configured and connected
2. Check MQTT enable checkbox in main MQTT settings
3. Look for MQTT errors in device logs
4. Use MQTT Explorer to monitor broker
5. Verify DS18B20 is enabled and reading successfully

**Wrong topic structure:**
- Expected: `topic/ROM_ID`
- Check MQTT_Topic setting
- Verify sensors were discovered (check logs for ROM IDs)

**Values not updating:**
- Check Interval setting (not too long)
- Verify sensor read errors in logs
- Confirm MQTT connection is stable

**Home Assistant not showing sensors:**
- Manual configuration required (no auto-discovery yet)
- Check ROM IDs match configuration
- Reload MQTT integration after config changes
- Verify topic subscription

## Related Parameters

- [DS18B20 Enable](Enable.md) - Enable the sensor
- [DS18B20 Interval](Interval.md) - Reading interval
- [DS18B20 MQTT Topic](MQTT_Topic.md) - Base MQTT topic
- [DS18B20 InfluxDB Enable](InfluxDB_Enable.md) - Alternative data logging
- [MQTT Enable](../MQTT/Enable.md) - Global MQTT configuration

## Example Complete Configuration

```ini
[MQTT]
Enable = true
Uri = mqtt://192.168.1.100
User = mqtt_user
Password = mqtt_pass
MainTopic = watermeter

[GPIO]
IO3 = onewire               ; * Requires disabling USB logging

[DS18B20]
Enable = true
Interval = -1
MQTT_Enable = true
MQTT_Topic = enclosure/temperature
InfluxDB_Enable = false
```

Results in topics:
- `watermeter/enclosure/temperature/28-0123456789AB`
- `watermeter/enclosure/temperature/28-FEDCBA987654`
- etc.
