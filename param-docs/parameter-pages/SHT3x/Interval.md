# SHT3x Interval

Set the reading interval for the SHT3x temperature and humidity sensor.

## Value
- `-1` - Follow flow cycle (default) - Read on every main flow execution
- `0` or positive integer - Custom interval in seconds

## Description

Controls how often the sensor is read and data is published.

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

[SHT3x]
Interval = -1        ; Read sensor every 10 minutes (follows flow)
```

Result: Sensor read every 10 minutes, aligned with meter readings.

### Custom Interval Mode (Interval > 0)

**For advanced monitoring scenarios**

Set a specific interval in seconds, independent of the main flow cycle.

**Use Cases:**
- **High-frequency monitoring**: Interval = 60 (every minute) for rapid temperature changes
- **Low-frequency logging**: Interval = 3600 (every hour) to reduce data volume
- **Different cadence**: Flow every 5 min, sensors every 15 min

**Example 1: Rapid Environmental Monitoring**
```ini
[AutoTimer]
Interval = 10        ; Flow every 10 minutes

[SHT3x]
Interval = 60        ; Read sensor every 1 minute
```

Use when: Monitoring rapidly changing conditions (e.g., heater cycling, direct sun exposure)

**Example 2: Low-Frequency Trending**
```ini
[AutoTimer]
Interval = 2         ; Flow every 2 minutes

[SHT3x]
Interval = 900       ; Read sensor every 15 minutes
```

Use when: Long-term trending, reduce InfluxDB storage

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
Good for: Detecting rapid temperature/humidity changes, weather events

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

**SHT3x Measurement Time:**
- High repeatability: ~15ms
- Medium repeatability: ~6ms
- Low repeatability: ~4ms

The sensor is fast, so reading every minute is not a problem.

**Data Volume:**
- Every minute: 1440 readings/day, 43,200/month
- Every 5 minutes: 288 readings/day, 8,640/month
- Every hour: 24 readings/day, 720/month

Consider InfluxDB storage and retention policies.

## Example Scenarios

### Scenario 1: Correlation Analysis
**Goal**: Understand if temperature affects meter accuracy

```ini
[AutoTimer]
Interval = 5         ; Read meter every 5 minutes

[SHT3x]
Interval = -1        ; Read environment every 5 minutes (synced)
```

Now you can correlate temperature with meter readings in Grafana:
- Does meter read high when temp > 30°C?
- Does humidity affect OCR accuracy?

### Scenario 2: Thermal Event Detection
**Goal**: Get alert when enclosure overheats

```ini
[SHT3x]
Interval = 120       ; Check every 2 minutes
MQTT_Enable = true
```

In Home Assistant:
```yaml
automation:
  - alias: "Enclosure Overheat Alert"
    trigger:
      platform: numeric_state
      entity_id: sensor.enclosure_temperature
      above: 65
      for: "00:04:00"  # 2 consecutive readings
    action:
      - service: notify.mobile_app
        data:
          message: "Watermeter enclosure temperature critical: {{ states('sensor.enclosure_temperature') }}°C"
```

### Scenario 3: Diurnal Trending
**Goal**: Understand daily temperature cycle

```ini
[SHT3x]
Interval = 600       ; Every 10 minutes
InfluxDB_Enable = true
```

Grafana query:
```
SELECT mean("temperature") 
FROM "environment" 
WHERE time > now() - 7d 
GROUP BY time(1h)
```

Reveals: Peak temperature at 3 PM, lowest at 5 AM, helps plan heater activation

## Minimum Interval

While technically you could set `Interval = 1` (every second), this is **not recommended**:
- Unnecessary wear on sensor
- Floods MQTT broker
- Fills InfluxDB quickly
- No benefit (environment doesn't change that fast)

**Minimum recommended**: 30 seconds for debugging, 60 seconds for monitoring

## Related Parameters

- [SHT3x Enable](Enable.md) - Enable the sensor
- [SHT3x MQTT Enable](MQTT_Enable.md) - Publish to MQTT
- [SHT3x InfluxDB Enable](InfluxDB_Enable.md) - Log to InfluxDB
- [AutoTimer Interval](../AutoTimer/Interval.md) - Main flow cycle

## Troubleshooting

**Readings not updating:**
1. Check sensor is enabled
2. Verify interval is not too long (e.g., 86400 = once per day)
3. Look in device logs for read errors
4. Confirm MQTT/InfluxDB is configured if expecting data

**Too much data:**
- Increase interval to reduce volume
- Adjust InfluxDB retention policy
- Use downsampling in Grafana

**Missed critical events:**
- Decrease interval for faster response
- Use Home Assistant automation with state change triggers
- Consider multiple intervals (frequent for alerts, slow for trending)
