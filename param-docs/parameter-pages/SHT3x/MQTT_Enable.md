# SHT3x MQTT Enable

Enable or disable MQTT publishing for SHT3x temperature and humidity sensor readings.

## Value
- `true` - Enable MQTT publishing (default)
- `false` - Disable MQTT publishing

## Description

When enabled, temperature and humidity readings from the SHT3x sensor are published to MQTT broker. Both measurements are published to the same base topic with separate `/temperature` and `/humidity` subtopics.

## MQTT Topics

### Topic Structure
```
{MQTT_Topic}/temperature → temperature_value
{MQTT_Topic}/humidity → humidity_value
```

Example with default topic `enclosure/climate`:
```
enclosure/climate/temperature → 24.5
enclosure/climate/humidity → 45.2
```

## Message Format

**Temperature Topic**: `{base_topic}/temperature`  
**Humidity Topic**: `{base_topic}/humidity`  
**Payload**: Numeric value as string (e.g., "24.5" or "45.2")  
**Retained**: Yes (last value persists)  
**QoS**: 1 (at least once delivery)

## Home Assistant Integration

### Auto-Discovery (Manual Setup Required)

Configure SHT3x sensors in Home Assistant:

```yaml
# configuration.yaml
mqtt:
  sensor:
    - name: "Enclosure Temperature"
      state_topic: "enclosure/climate/temperature"
      unit_of_measurement: "°C"
      device_class: temperature
      state_class: measurement
      
    - name: "Enclosure Humidity"
      state_topic: "enclosure/climate/humidity"
      unit_of_measurement: "%"
      device_class: humidity
      state_class: measurement
```

### Complete Configuration Example

```ini
[MQTT]
Enable = true
Uri = mqtt://192.168.1.100:1883
MainTopic = watermeter

[SHT3x]
Enable = true
Address = 0x44
Interval = -1
MQTT_Enable = true
MQTT_Topic = enclosure/climate
```

Publishes to:
```
watermeter/enclosure/climate/temperature → 24.5
watermeter/enclosure/climate/humidity → 45.2
```

## Use Cases

### Enclosure Health Monitoring

**Problem**: Device in outdoor enclosure exposed to condensation risk  
**Solution**: Monitor humidity to detect water ingress

Home Assistant automation:
```yaml
automation:
  - alias: "Alert: High Enclosure Humidity"
    trigger:
      - platform: numeric_state
        entity_id: sensor.enclosure_humidity
        above: 80
        for:
          minutes: 10
    action:
      - service: notify.mobile_app
        data:
          message: "Enclosure humidity is {{ states('sensor.enclosure_humidity') }}% - check for water ingress!"
```

### Temperature-Controlled Cooling

**Problem**: Overheating in direct sunlight  
**Solution**: Monitor temperature to trigger ventilation fan

```yaml
automation:
  - alias: "Enclosure Cooling Fan"
    trigger:
      - platform: numeric_state
        entity_id: sensor.enclosure_temperature
        above: 50
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.enclosure_fan
  
  - alias: "Stop Cooling Fan"
    trigger:
      - platform: numeric_state
        entity_id: sensor.enclosure_temperature
        below: 40
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.enclosure_fan
```

### Combined Climate Alerts

Monitor both temperature and humidity:

```yaml
automation:
  - alias: "Alert: Extreme Conditions"
    trigger:
      - platform: template
        value_template: >
          {{ 
            states('sensor.enclosure_temperature')|float > 60 or
            states('sensor.enclosure_humidity')|float > 85
          }}
    action:
      - service: notify.mobile_app
        data:
          title: "⚠️ Enclosure Climate Warning"
          message: >
            Temperature: {{ states('sensor.enclosure_temperature') }}°C
            Humidity: {{ states('sensor.enclosure_humidity') }}%
```

## Node-RED Integration

Example flow to log extreme conditions:

```json
[
  {
    "type": "mqtt in",
    "topic": "watermeter/enclosure/climate/temperature",
    "name": "Temperature"
  },
  {
    "type": "mqtt in",
    "topic": "watermeter/enclosure/climate/humidity",
    "name": "Humidity"
  },
  {
    "type": "function",
    "func": "if (msg.payload > 60) { return msg; }"
  }
]
```

## Troubleshooting

**No MQTT messages:**
1. Verify global MQTT is enabled: `[MQTT] Enable = true`
2. Check MQTT broker connection in device logs
3. Confirm SHT3x sensor is reading (check Info page)
4. Verify MQTT_Enable = true for SHT3x specifically

**Messages not retained:**
- Check MQTT broker configuration
- Ensure broker supports retained messages
- Some brokers require explicit permission for retained messages

**Home Assistant not showing sensor:**
1. Check MQTT integration is configured
2. Verify topic matches configuration
3. Listen to MQTT topic with MQTT Explorer to confirm publishing
4. Check Home Assistant logs for MQTT errors

## Performance Considerations

### Publishing Frequency

**High-frequency publishing** (Interval = 30 seconds):
- Pros: Rapid response to changes
- Cons: Higher network traffic, more broker load

**Follow-flow publishing** (Interval = -1, typical 5-10 minutes):
- Pros: Minimal overhead, data aligned with meter readings
- Cons: Slower response to rapid changes

**Recommended**: Use Interval = -1 unless rapid climate response is needed

### Network Usage

Typical message sizes:
- Temperature: ~30 bytes
- Humidity: ~30 bytes
- Total per reading: ~60 bytes

At 5-minute intervals: ~17 KB/day  
At 1-minute intervals: ~86 KB/day

## Related Parameters

- [SHT3x Enable](Enable.md) - Enable the sensor
- [SHT3x MQTT_Topic](MQTT_Topic.md) - Configure base topic
- [SHT3x Interval](Interval.md) - Set reading frequency
- [MQTT Enable](../MQTT/Enable.md) - Global MQTT configuration
- [MQTT Uri](../MQTT/Uri.md) - MQTT broker connection
