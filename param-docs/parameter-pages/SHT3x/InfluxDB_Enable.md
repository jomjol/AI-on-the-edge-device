# SHT3x InfluxDB Enable

Enable or disable InfluxDB logging for SHT3x temperature and humidity sensor readings.

## Value
- `true` - Enable InfluxDB logging
- `false` - Disable InfluxDB logging (default)

## Description

When enabled, temperature and humidity readings from the SHT3x sensor are written to InfluxDB for historical tracking and analysis. Both measurements are stored as separate fields in the same measurement.

## InfluxDB Field Structure

### Measurement
Configured via [InfluxDB_Measurement](InfluxDB_Measurement.md) parameter (default: `environment`)

### Fields
The SHT3x creates two fields:
- `sht3x_temperature`: Temperature value (°C)
- `sht3x_humidity`: Humidity value (%)

**Example Data Point:**
```
Measurement: environment
Time: 2024-01-15T14:30:00Z
Fields:
  sht3x_temperature: 24.5
  sht3x_humidity: 45.2
```

**With Multiple Sensor Types:**
```
Measurement: environment
Time: 2024-01-15T14:30:00Z
Fields:
  sht3x_temperature: 24.5        (SHT3x inside enclosure)
  sht3x_humidity: 45.2           (SHT3x inside enclosure)
  temperature_28-0123456789AB: 15.2  (DS18B20 outside)
  temperature_28-FEDCBA987654: 22.3  (DS18B20 inside)
```

## Use Cases

### Long-Term Climate Trending

Track temperature and humidity patterns over time:

```ini
[SHT3x]
Enable = true
Interval = 600        ; Every 10 minutes
InfluxDB_Enable = true
InfluxDB_Measurement = environment
```

Grafana query - Daily climate overview:
```sql
SELECT 
  mean("sht3x_temperature") as "Avg Temperature",
  mean("sht3x_humidity") as "Avg Humidity",
  max("sht3x_temperature") as "Peak Temp",
  max("sht3x_humidity") as "Peak Humidity"
FROM "environment" 
WHERE time > now() - 30d 
GROUP BY time(1d)
```

Reveals: Daily cycles, seasonal trends, condensation risk periods

### Condensation Risk Analysis

Identify when humidity is dangerously high:

```sql
SELECT 
  "sht3x_temperature",
  "sht3x_humidity"
FROM "environment" 
WHERE 
  "sht3x_humidity" > 80 AND
  time > now() - 7d
```

Calculate dew point to predict condensation:
```sql
SELECT 
  "sht3x_temperature" as "temp",
  "sht3x_humidity" as "humidity",
  "sht3x_temperature" - ((100 - "sht3x_humidity") / 5) as "dew_point_approx"
FROM "environment" 
WHERE time > now() - 24h
```

### Enclosure Health Monitoring

Correlate climate with device performance:

```sql
SELECT 
  mean("sht3x_temperature") as "temperature",
  count("error") as "error_count"
FROM "environment", "device_logs"
WHERE time > now() - 30d 
GROUP BY time(1d)
```

Identify: Does device fail more often in extreme temperatures?

### Seasonal Analysis

Compare conditions across seasons:

```sql
-- Summer vs Winter
SELECT 
  mean("sht3x_temperature") as "avg_temp",
  mean("sht3x_humidity") as "avg_humidity"
FROM "environment" 
WHERE time > '2024-06-01' AND time < '2024-09-01'  -- Summer
GROUP BY time(1w)

-- Compare to winter
WHERE time > '2024-12-01' AND time < '2025-03-01'  -- Winter
```

Helps plan: Heating/cooling requirements, insulation needs

### Correlation with External Conditions

Compare inside vs outside (if you have weather data):

```sql
SELECT 
  "sht3x_temperature" as "inside_temp",
  "sht3x_humidity" as "inside_humidity",
  "weather_temperature" as "outside_temp"
FROM "environment", "weather"
WHERE time > now() - 24h
```

Reveals: Insulation effectiveness, ventilation needs

## Grafana Dashboard Examples

### Climate Overview Panel

```json
{
  "title": "Enclosure Climate",
  "targets": [
    {
      "query": "SELECT mean(\"sht3x_temperature\") FROM \"environment\" WHERE $timeFilter GROUP BY time($__interval)"
    },
    {
      "query": "SELECT mean(\"sht3x_humidity\") FROM \"environment\" WHERE $timeFilter GROUP BY time($__interval)"
    }
  ]
}
```

### Comfort Zone Visualization

Graph showing safe operating zone:
- Green: Temp 0-50°C, Humidity 20-60%
- Yellow: Temp 50-65°C, Humidity 60-80%
- Red: Above thresholds

### Heatmap of Daily Patterns

```sql
SELECT 
  mean("sht3x_temperature") as "temperature"
FROM "environment" 
WHERE $timeFilter
GROUP BY time(1h), day
```

Shows: Hottest time of day, daily cycles, weekly patterns

## Retention Policies

### Default Retention (Forever)

```ini
[InfluxDB]
Enable = true

[SHT3x]
InfluxDB_Enable = true
```

All data kept indefinitely (storage grows continuously)

### Custom Retention Policy

Create time-based retention in InfluxDB:

```sql
-- Keep raw data for 90 days
CREATE RETENTION POLICY "90_days" ON "mydb" DURATION 90d REPLICATION 1 DEFAULT

-- Downsample to hourly averages, keep 2 years
CREATE CONTINUOUS QUERY "cq_hourly_climate" ON "mydb"
BEGIN
  SELECT mean("sht3x_temperature") AS "temperature",
         mean("sht3x_humidity") AS "humidity"
  INTO "2_years"."environment"
  FROM "environment"
  GROUP BY time(1h)
END
```

Result: Detailed data for 3 months, hourly averages for 2 years

## Data Volume Estimates

### Storage Requirements

Assuming one SHT3x sensor:

**High-frequency logging** (30 second intervals):
- Data points per day: 2,880
- Storage per day: ~35 KB (with compression)
- Storage per year: ~12.5 MB

**Follow-flow logging** (5 minute intervals):
- Data points per day: 288
- Storage per day: ~3.5 KB
- Storage per year: ~1.3 MB

**Recommended**: Start with follow-flow (Interval = -1), increase frequency only if needed for specific monitoring requirements.

## Prerequisites

### InfluxDB Configuration Required

Before enabling SHT3x InfluxDB logging:

1. **Global InfluxDB must be enabled:**
```ini
[InfluxDB]
Enable = true
Uri = http://192.168.1.100:8086
Database = watermeter
```

2. **InfluxDB server must be accessible:**
- Check network connectivity
- Verify database exists
- Confirm authentication (if required)

3. **Test connection in device logs:**
Look for InfluxDB connection success message

### Verification Steps

After enabling:

1. Check device logs for InfluxDB write success
2. Query InfluxDB directly:
```sql
SELECT * FROM environment WHERE time > now() - 5m
```
3. Verify fields exist:
```sql
SHOW FIELD KEYS FROM environment
```
Should show: `sht3x_temperature` and `sht3x_humidity`

## Troubleshooting

**No data in InfluxDB:**
1. Verify global InfluxDB is enabled: `[InfluxDB] Enable = true`
2. Check InfluxDB connection in device logs
3. Confirm SHT3x sensor is reading (check Info page)
4. Verify InfluxDB_Enable = true for SHT3x specifically
5. Check InfluxDB server logs for write errors

**Data not visible in Grafana:**
1. Verify Grafana data source points to correct InfluxDB
2. Check measurement name matches: `environment` (or custom)
3. Verify field names: `sht3x_temperature`, `sht3x_humidity`
4. Use InfluxDB CLI to verify data exists
5. Check Grafana query syntax

**Intermittent data gaps:**
- Check network stability
- Verify InfluxDB server isn't overloaded
- Check device logs for write failures
- Consider increasing Interval to reduce write frequency

**Storage growing too fast:**
- Reduce reading frequency (increase Interval)
- Implement retention policy (see above)
- Enable InfluxDB compression
- Consider downsampling old data

## Performance Considerations

### Write Frequency Impact

**High-frequency writes** (every 30 seconds):
- Pros: Detailed data, catch rapid changes
- Cons: Network traffic, storage usage, server load
- When: Monitoring rapid climate changes, troubleshooting

**Medium-frequency writes** (every 5 minutes):
- Pros: Good balance of detail and efficiency
- Cons: May miss rapid fluctuations
- When: General monitoring, most use cases

**Follow-flow writes** (Interval = -1, typically 5-10 minutes):
- Pros: Minimal overhead, aligned with meter readings
- Cons: Lower temporal resolution
- When: Long-term trending, minimal infrastructure

### Batch Writing

InfluxDB handles batching internally. To optimize:
- Use consistent intervals (not random)
- Avoid very high frequency (< 30 seconds) unless necessary
- Monitor InfluxDB write queue metrics

## Related Parameters

- [SHT3x Enable](Enable.md) - Enable the sensor
- [SHT3x InfluxDB_Measurement](InfluxDB_Measurement.md) - Set measurement name
- [SHT3x Interval](Interval.md) - Set reading frequency
- [InfluxDB Enable](../InfluxDB/Enable.md) - Global InfluxDB configuration
- [InfluxDB Uri](../InfluxDB/Uri.md) - InfluxDB server connection
