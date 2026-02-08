# DS18B20 InfluxDB Enable

Enable or disable InfluxDB logging for DS18B20 temperature sensor readings.

## Value
- `true` - Enable InfluxDB logging
- `false` - Disable InfluxDB logging (default)

## Description

When enabled, temperature readings from all DS18B20 sensors are written to InfluxDB for historical tracking and analysis. Each sensor creates a unique field with its ROM ID included in the field name.

## InfluxDB Field Structure

### Measurement
Configured via [InfluxDB_Measurement](InfluxDB_Measurement.md) parameter (default: `environment`)

### Fields
Each sensor creates a field named: `temperature_{ROM_ID}`

**Single Sensor:**
```
Measurement: environment
Field: temperature_28-0123456789AB
Value: 24.5
```

**Multiple Sensors (3 sensors):**
```
Measurement: environment
Fields:
  - temperature_28-0123456789AB: 24.5  (Inside)
  - temperature_28-FEDCBA987654: 15.2  (Outside)
  - temperature_28-AABBCCDD1122: 12.8  (Pipe)
```

ROM IDs ensure each sensor's data is uniquely identified even if sensors are added/removed.

## Use Cases

### Long-Term Trending
Track temperature patterns over days, weeks, months:

```ini
[DS18B20]
Enable = true
Interval = 600        ; Every 10 minutes
InfluxDB_Enable = true
InfluxDB_Measurement = environment
```

Grafana query:
```sql
SELECT 
  mean("temperature_28-0123456789AB") as "Inside",
  mean("temperature_28-FEDCBA987654") as "Outside"
FROM "environment" 
WHERE time > now() - 30d 
GROUP BY time(1h)
```

Reveals: Daily cycles, seasonal trends, anomalies

### Capacity Planning
Understand when heating/cooling is needed:

```sql
SELECT 
  max("temperature_28-0123456789AB") as "Peak",
  min("temperature_28-0123456789AB") as "Low"
FROM "environment" 
WHERE time > now() - 7d 
GROUP BY time(1d)
```

Helps plan: Heater scheduling, insulation upgrades

### Multi-Point Correlation
Compare temperatures across locations:

```sql
SELECT 
  mean("temperature_28-0123456789AB") - mean("temperature_28-FEDCBA987654") as "Gradient"
FROM "environment" 
WHERE time > now() - 24h 
GROUP BY time(1h)
```

Reveals: Heat loss, insulation effectiveness

### Root Cause Analysis
When issues occur, look back at environmental conditions:

```sql
SELECT 
  "temperature_28-0123456789AB",
  "temperature_28-FEDCBA987654"
FROM "environment" 
WHERE time >= '2024-01-15T14:00:00Z' 
  AND time <= '2024-01-15T15:00:00Z'
```

Correlate with: Device failures, reading errors, anomalies

## InfluxDB vs MQTT

| Feature | InfluxDB | MQTT |
|---------|----------|------|
| **Purpose** | Historical storage | Real-time alerts |
| **Retention** | Configurable (days to years) | Last value only (retained) |
| **Query** | Powerful SQL-like queries | Simple subscribe |
| **Visualization** | Grafana dashboards | Home Assistant cards |
| **Alerting** | Kapacitor, Grafana | Home Assistant automation |
| **Storage** | Database on disk | In-memory + retained |
| **Best For** | Trending, analysis | Immediate response |

### Recommended Combinations

**Option 1: Both Enabled (Full Stack)**
```ini
MQTT_Enable = true
InfluxDB_Enable = true
```
- Real-time alerts via MQTT → Home Assistant
- Historical analysis via InfluxDB → Grafana
- Best for: Production monitoring

**Option 2: InfluxDB Only**
```ini
MQTT_Enable = false
InfluxDB_Enable = true
```
- Focused on data analysis
- Reduced network traffic
- Best for: Data logging, trending

**Option 3: MQTT Only**
```ini
MQTT_Enable = true
InfluxDB_Enable = false
```
- Real-time monitoring only
- Minimal storage requirements
- Best for: Simple alert systems

## Grafana Dashboards

### Basic Temperature Panel
```json
{
  "targets": [
    {
      "query": "SELECT mean(\"temperature_28-0123456789AB\") FROM \"environment\" WHERE $timeFilter GROUP BY time($__interval)",
      "alias": "Enclosure Inside"
    },
    {
      "query": "SELECT mean(\"temperature_28-FEDCBA987654\") FROM \"environment\" WHERE $timeFilter GROUP BY time($__interval)",
      "alias": "Enclosure Outside"
    }
  ]
}
```

### Temperature Gradient
```json
{
  "targets": [
    {
      "query": "SELECT mean(\"temperature_28-0123456789AB\") - mean(\"temperature_28-FEDCBA987654\") FROM \"environment\" WHERE $timeFilter GROUP BY time($__interval)",
      "alias": "Inside - Outside Gradient"
    }
  ]
}
```

### Daily Min/Max
```json
{
  "targets": [
    {
      "query": "SELECT max(\"temperature_28-0123456789AB\") as \"Peak\", min(\"temperature_28-0123456789AB\") as \"Low\" FROM \"environment\" WHERE $timeFilter GROUP BY time(1d)",
      "alias": "Daily Range"
    }
  ]
}
```

## Performance Considerations

### Write Frequency
Based on your interval:
- Interval = 60s: 1,440 points/day per sensor
- Interval = 300s: 288 points/day per sensor
- Interval = 600s: 144 points/day per sensor

### Storage Requirements
Approximate storage per sensor:
- 1 point ≈ 100-150 bytes (compressed)
- 1 day at 5-min intervals: 288 points ≈ 35 KB
- 1 month: ~1 MB per sensor
- 1 year: ~12 MB per sensor

With 3 sensors: ~36 MB/year (very manageable)

### Retention Policies
Configure in InfluxDB to manage storage:

```sql
CREATE RETENTION POLICY "one_week" ON "mydb" DURATION 7d REPLICATION 1
CREATE RETENTION POLICY "one_month" ON "mydb" DURATION 30d REPLICATION 1
CREATE RETENTION POLICY "one_year" ON "mydb" DURATION 365d REPLICATION 1
```

### Downsampling
Create continuous queries for long-term data:

```sql
CREATE CONTINUOUS QUERY "cq_temperature_1h" ON "mydb"
BEGIN
  SELECT mean("temperature_28-0123456789AB") as "temperature_28-0123456789AB"
  INTO "one_year"."environment"
  FROM "environment"
  GROUP BY time(1h)
END
```

Keeps: Hourly averages forever, raw data for 30 days

## When to Enable

**Enable InfluxDB if:**
- ✅ You want historical trend analysis
- ✅ You use Grafana dashboards
- ✅ You need long-term data retention
- ✅ You want to correlate with other metrics
- ✅ You need advanced queries and analytics

**Disable InfluxDB if:**
- ❌ You only need real-time alerts (use MQTT)
- ❌ InfluxDB not configured on network
- ❌ Limited storage available
- ❌ Temporary testing/debugging only

## InfluxDB Configuration Requirements

**Prerequisites:**
1. InfluxDB server installed and running
2. Database created
3. InfluxDB section configured in device settings:
   - Uri
   - Database name
   - Credentials (if required)

**Minimum InfluxDB Config:**
```ini
[InfluxDB]
Enable = true
Uri = http://192.168.1.100:8086
Database = homeautomation
User = influx_user
Password = influx_pass
```

## Data Quality

### Accurate Timestamps
InfluxDB stores precise timestamps for each reading, enabling:
- Exact time correlation
- Sub-second precision
- Time-based queries

### Missing Data Handling
If sensor read fails:
- No data point written (better than writing error values)
- Grafana shows gap in graph
- Can detect sensor failures by missing data

## Querying Examples

### Current Temperature
```sql
SELECT last("temperature_28-0123456789AB") 
FROM "environment"
```

### Average Over Time
```sql
SELECT mean("temperature_28-0123456789AB") 
FROM "environment" 
WHERE time > now() - 24h
```

### Temperature Range
```sql
SELECT 
  max("temperature_28-0123456789AB") as "Max",
  min("temperature_28-0123456789AB") as "Min",
  max("temperature_28-0123456789AB") - min("temperature_28-0123456789AB") as "Range"
FROM "environment" 
WHERE time > now() - 7d
```

### Hourly Averages
```sql
SELECT mean("temperature_28-0123456789AB") 
FROM "environment" 
WHERE time > now() - 7d 
GROUP BY time(1h)
```

### Multiple Sensors
```sql
SELECT 
  mean("temperature_28-0123456789AB") as "Inside",
  mean("temperature_28-FEDCBA987654") as "Outside",
  mean("temperature_28-AABBCCDD1122") as "Pipe"
FROM "environment" 
WHERE time > now() - 30d 
GROUP BY time(1d)
```

## Troubleshooting

**Data not appearing in InfluxDB:**
1. Verify InfluxDB is enabled globally
2. Check InfluxDB connection in device logs
3. Verify database exists
4. Check credentials
5. Confirm DS18B20 sensors are reading successfully
6. Use InfluxDB CLI to check for data:
   ```
   USE homeautomation
   SELECT * FROM environment ORDER BY time DESC LIMIT 10
   ```

**Old data but no recent data:**
- Check if device is reading sensors (look at logs)
- Verify interval isn't too long
- Check InfluxDB connection status

**Missing some sensors:**
- Check that all sensors are discovered (ROM search at startup)
- Verify all sensors read successfully
- Look for errors in device logs

**Too much data:**
- Increase read interval
- Set up retention policies
- Use downsampling with continuous queries

## Related Parameters

- [DS18B20 Enable](Enable.md) - Enable the sensor
- [DS18B20 Interval](Interval.md) - Reading interval (affects write frequency)
- [DS18B20 InfluxDB Measurement](InfluxDB_Measurement.md) - Measurement name
- [DS18B20 MQTT Enable](MQTT_Enable.md) - Alternative data publishing
- [InfluxDB Enable](../InfluxDB/Enable.md) - Global InfluxDB configuration

## Example Complete Configuration

```ini
[InfluxDB]
Enable = true
Uri = http://192.168.1.100:8086
Database = homeautomation
User = influx_user
Password = influx_pass

[GPIO]
IO3 = onewire               ; * Requires disabling USB logging

[DS18B20]
Enable = true
Interval = 300        ; Every 5 minutes
MQTT_Enable = true    ; For alerts
InfluxDB_Enable = true
InfluxDB_Measurement = environment
```

**Result:**
- Real-time alerts via MQTT
- Long-term trending via InfluxDB
- Data written every 5 minutes
- Unique field per sensor: `temperature_{ROM_ID}`
