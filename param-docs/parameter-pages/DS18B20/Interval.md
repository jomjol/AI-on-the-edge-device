# DS18B20 Interval

Set the reading interval for DS18B20 temperature sensor(s).

## Value
- `-1` - Follow flow cycle (default) - Read on every main flow execution
- `0` or positive integer - Custom interval in seconds

## Description

Controls how often the sensor(s) are read and data is published.

### Follow Flow Mode (Interval = -1)

**Recommended for most users**

When set to `-1`, the sensor is read every time the main flow executes. This synchronizes sensor readings with meter readings.

**Benefits:**
- Sensor data timestamped with meter data
- Correlate environmental conditions with meter readings
- No additional timer overhead
- Simple configuration

**Example:**
```ini
[AutoTimer]
Interval = 10        ; Main flow every 10 minutes

[DS18B20]
Interval = -1        ; Read sensor every 10 minutes (follows flow)
```

Result: Sensor read every 10 minutes, aligned with meter readings.

### Custom Interval Mode (Interval > 0)

**For advanced monitoring scenarios**

Set a specific interval in seconds, independent of the main flow cycle.

**Note:** As of version [current], custom intervals are now properly supported through a background task. Sensors with custom intervals will be read at their specified rate, even if shorter than the flow interval.

**Use Cases:**
- **High-frequency monitoring**: Interval = 60 (every minute) for rapid temperature changes
- **Low-frequency logging**: Interval = 3600 (every hour) to reduce data volume
- **Different cadence**: Flow every 5 min, sensors every 15 min

**Example 1: Rapid Temperature Monitoring**
```ini
[AutoTimer]
Interval = 10        ; Flow every 10 minutes

[DS18B20]
Interval = 60        ; Read sensor every 1 minute
```

Use when: Monitoring rapidly changing conditions (e.g., pipe temperature, outdoor exposure)

**Example 2: Low-Frequency Trending**
```ini
[AutoTimer]
Interval = 2         ; Flow every 2 minutes

[DS18B20]
Interval = 900       ; Read sensor every 15 minutes
```

Use when: Long-term trending, reduce InfluxDB storage

## Multiple Sensors

When multiple DS18B20 sensors are chained on the same 1-Wire bus, they are all read during each cycle. The interval applies to the entire bus, not individual sensors.

**Reading Time:**
- Single sensor: ~800ms (750ms conversion + 50ms communication)
- Three sensors: ~2.4 seconds (sequential reads)

## Choosing the Right Interval

### For Device Health Monitoring

**General Purpose (Recommended):**
```ini
Interval = -1        ; Follow flow cycle
```
Good for: Most installations, balanced data collection

**Outdoor Extreme Weather:**
```ini
Interval = 300       ; Every 5 minutes
```
Good for: Detecting rapid temperature changes, weather events

**Indoor Stable Environment:**
```ini
Interval = 1800      ; Every 30 minutes
```
Good for: Basement, garage, minimal variation

**Failure Detection:**
```ini
Interval = 60        ; Every 1 minute
```
Good for: Debugging overheating, tracking cooling system performance

### Performance Considerations

**DS18B20 Measurement Time:**
- 12-bit resolution: 750ms conversion time
- Total read time: ~800ms per sensor

**Data Volume with 3 sensors:**
- Every minute: 4,320 readings/day, 129,600/month
- Every 5 minutes: 864 readings/day, 25,920/month
- Every hour: 72 readings/day, 2,160/month

Consider InfluxDB storage and retention policies.

## Example Scenarios

### Scenario 1: Multi-Point Temperature Monitoring
**Goal**: Track inside, outside, and pipe temperatures

```ini
[GPIO]
IO3 = onewire               ; * Requires disabling USB logging       ; 3 sensors on same wire

[DS18B20]
Enable = true
Interval = -1        ; Follow flow (e.g., every 10 min)
MQTT_Enable = true
MQTT_Topic = enclosure/temperature
```

MQTT output:
```
enclosure/temperature/28-0123456789AB → 24.5  (inside)
enclosure/temperature/28-FEDCBA987654 → 15.2  (outside)
enclosure/temperature/28-AABBCCDD1122 → 12.8  (pipe)
```

### Scenario 2: Freeze Protection
**Goal**: Alert when pipe temperature drops too low

```ini
[DS18B20]
Interval = 120       ; Check every 2 minutes
MQTT_Enable = true
```

In Home Assistant:
```yaml
automation:
  - alias: "Pipe Freeze Alert"
    trigger:
      platform: numeric_state
      entity_id: sensor.pipe_temperature
      below: 2
      for: "00:04:00"  # 2 consecutive readings
    action:
      - service: notify.mobile_app
        data:
          message: "⚠️ Pipe temperature: {{ states('sensor.pipe_temperature') }}°C - Freeze risk!"
```

### Scenario 3: Thermal Gradient Analysis
**Goal**: Understand temperature distribution

```ini
[DS18B20]
Interval = 600       ; Every 10 minutes
InfluxDB_Enable = true
InfluxDB_Measurement = environment
```

Grafana query:
```
SELECT 
  mean("temperature_28-0123456789AB") as "Inside",
  mean("temperature_28-FEDCBA987654") as "Outside",
  mean("temperature_28-AABBCCDD1122") as "Pipe"
FROM "environment" 
WHERE time > now() - 7d 
GROUP BY time(1h)
```

Reveals: Temperature gradients, insulation effectiveness, heat loss patterns

## How Custom Intervals Work (Technical Details)

When you set a custom interval (e.g., `Interval = 5`), the system uses a background task to read sensors independently of the main flow cycle.

**Background Task Behavior:**
- Runs every 1 second checking if sensors need reading
- Minimal CPU overhead - only checks timestamps
- Low priority to not interfere with main flow
- Automatically starts when sensors are initialized

**Example Scenario:**
```ini
[AutoTimer]
Interval = 5        ; Flow every 5 minutes (300 seconds)

[DS18B20]
Interval = 5        ; Read sensor every 5 seconds
```

**What happens:**
1. Main flow runs every 300 seconds
2. Background task wakes up every 1 second
3. Checks if 5 seconds elapsed since last sensor read
4. If yes, reads sensor and publishes to MQTT/InfluxDB
5. Result: ~60 sensor readings per flow cycle

**Performance Impact:**
- CPU: Minimal (task mostly sleeps)
- Memory: 4KB for background task stack
- Sensor read time: ~800ms per DS18B20 (unchanged)

## Minimum Interval

While technically you could set `Interval = 1` (every second), this is **not recommended**:
- Unnecessary wear on sensor
- Floods MQTT broker
- Fills InfluxDB quickly
- No benefit (environment doesn't change that fast)

**Minimum recommended**: 30 seconds for debugging, 60 seconds for monitoring

## Related Parameters

- [DS18B20 Enable](Enable.md) - Enable the sensor
- [DS18B20 MQTT Enable](MQTT_Enable.md) - Publish to MQTT
- [DS18B20 MQTT Topic](MQTT_Topic.md) - MQTT topic configuration
- [DS18B20 InfluxDB Enable](InfluxDB_Enable.md) - Log to InfluxDB
- [DS18B20 InfluxDB Measurement](InfluxDB_Measurement.md) - InfluxDB measurement name
- [GPIO OneWire](../GPIO/OneWire.md) - GPIO pin configuration
- [AutoTimer Interval](../AutoTimer/Interval.md) - Main flow cycle

## Troubleshooting

**Readings not updating:**
1. Check sensor is enabled
2. Verify interval is not too long (e.g., 86400 = once per day)
3. Look in device logs for read errors
4. Confirm GPIO pin is configured for 1-Wire mode
5. Verify MQTT/InfluxDB is configured if expecting data

**Too much data:**
- Increase interval to reduce volume
- Adjust InfluxDB retention policy
- Use downsampling in Grafana

**Missed critical events:**
- Decrease interval for faster response
- Use Home Assistant automation with state change triggers
- Consider multiple intervals (frequent for alerts, slow for trending)

**Slow reads with multiple sensors:**
- Each sensor takes ~800ms to read
- 3 sensors = ~2.4 seconds per cycle
- This is normal and acceptable for typical intervals (>= 60 seconds)
