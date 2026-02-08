# SHT3x InfluxDB Measurement

Set the InfluxDB measurement name for SHT3x temperature and humidity sensor data.

## Value
Default: `environment`

String value representing the InfluxDB measurement name where sensor data is stored.

## Description

In InfluxDB, a **measurement** is analogous to a database table. All SHT3x sensor readings are written to this measurement with two fields: `sht3x_temperature` and `sht3x_humidity`.

## Measurement Structure

### Format
```
Measurement: {InfluxDB_Measurement}
Fields:
  sht3x_temperature: {temperature_value}
  sht3x_humidity: {humidity_value}
Timestamp: {read_time}
```

### Example Data Point

**Single SHT3x Sensor:**
```
Measurement: environment
Time: 2024-01-15T14:30:00Z
Fields:
  sht3x_temperature = 24.5
  sht3x_humidity = 45.2
```

**Multiple Sensor Types Combined:**
```
Measurement: environment
Time: 2024-01-15T14:30:00Z
Fields:
  sht3x_temperature = 24.5           (SHT3x inside)
  sht3x_humidity = 45.2              (SHT3x inside)
  temperature_28-0123456789AB = 15.2 (DS18B20 outside)
  temperature_28-FEDCBA987654 = 22.3 (DS18B20 inside)
```

## Naming Conventions

### Recommended Measurement Names

**By Data Category:**
```
environment      - Environmental monitoring (recommended)
sensors          - Generic sensor data
climate          - Climate-specific data
monitoring       - General monitoring data
device_health    - Device health metrics
enclosure        - Enclosure-specific data
```

**By Location:**
```
enclosure_env    - Enclosure environmental data
outdoor_env      - Outdoor environmental data
basement_sensors - Basement sensor readings
garage_climate   - Garage climate monitoring
```

**By System:**
```
watermeter_env   - Watermeter environmental data
gasmeter_env     - Gas meter environmental data
meter_health     - Meter health monitoring
```

### Best Practices

**DO:**
- ✅ Use lowercase names
- ✅ Use underscores for word separation
- ✅ Keep names descriptive but concise (< 30 characters)
- ✅ Use consistent naming across devices
- ✅ Group related sensors in same measurement

**DON'T:**
- ❌ Use spaces (breaks queries)
- ❌ Use special characters except underscore and dash
- ❌ Use overly long names
- ❌ Mix unrelated data types in same measurement
- ❌ Use database-reserved keywords

## Querying Data

### Basic Queries

**Get recent readings:**
```sql
SELECT sht3x_temperature, sht3x_humidity 
FROM environment 
WHERE time > now() - 1h
```

**Calculate averages:**
```sql
SELECT 
  mean(sht3x_temperature) as avg_temp,
  mean(sht3x_humidity) as avg_humidity
FROM environment 
WHERE time > now() - 24h 
GROUP BY time(1h)
```

**Find extremes:**
```sql
SELECT 
  max(sht3x_temperature) as max_temp,
  min(sht3x_temperature) as min_temp,
  max(sht3x_humidity) as max_humidity,
  min(sht3x_humidity) as min_humidity
FROM environment 
WHERE time > now() - 7d
```

### Advanced Queries

**Correlation analysis:**
```sql
SELECT 
  sht3x_temperature,
  sht3x_humidity,
  sht3x_temperature - ((100 - sht3x_humidity) / 5) as dew_point_estimate
FROM environment 
WHERE time > now() - 24h
```

**Multi-sensor comparison:**
```sql
SELECT 
  sht3x_temperature as "Inside Temp",
  temperature_28-0123456789AB as "Outside Temp",
  sht3x_humidity as "Inside Humidity"
FROM environment 
WHERE time > now() - 24h
```

**Conditional filtering:**
```sql
SELECT * 
FROM environment 
WHERE 
  sht3x_humidity > 80 OR
  sht3x_temperature > 60
AND time > now() - 30d
```

## Grafana Integration

### Datasource Configuration

1. Add InfluxDB datasource in Grafana
2. Set database name (e.g., `watermeter`)
3. Configure measurement name: `environment`

### Panel Queries

**Temperature graph:**
```sql
SELECT mean("sht3x_temperature") 
FROM "environment" 
WHERE $timeFilter 
GROUP BY time($__interval) fill(null)
```

**Humidity graph:**
```sql
SELECT mean("sht3x_humidity") 
FROM "environment" 
WHERE $timeFilter 
GROUP BY time($__interval) fill(null)
```

**Combined panel:**
```sql
SELECT 
  mean("sht3x_temperature") as "Temperature",
  mean("sht3x_humidity") as "Humidity"
FROM "environment" 
WHERE $timeFilter 
GROUP BY time($__interval)
```

### Dashboard Variables

Create measurement variable for dynamic dashboards:

```
Name: measurement
Query: SHOW MEASUREMENTS
```

Then use in queries:
```sql
SELECT * FROM "$measurement" WHERE $timeFilter
```

## Organization Strategies

### Single Measurement (Recommended for Small Deployments)

Store all sensor data in one measurement:

```ini
[SHT3x]
InfluxDB_Measurement = environment

[DS18B20]
InfluxDB_Measurement = environment
```

**Pros:**
- Simple queries
- Easy to correlate data
- Single dashboard for all sensors

**Cons:**
- All data mixed together
- Harder to apply different retention policies

### Multiple Measurements (Better for Large Deployments)

Separate by sensor type or location:

```ini
[SHT3x]
InfluxDB_Measurement = climate

[DS18B20]
InfluxDB_Measurement = temperature
```

**Pros:**
- Clear data separation
- Different retention policies per type
- Better query performance for specific types

**Cons:**
- More complex queries for correlation
- Multiple dashboard panels needed

### Hierarchical Naming

Use consistent prefixes for organization:

```
device_climate        - Device climate data
device_temperature    - Device temperature sensors
system_health        - System health metrics
meter_readings       - Meter reading data
```

## Data Retention

### Apply Retention by Measurement

**Keep climate data longer:**
```sql
-- Create retention policy for climate data
CREATE RETENTION POLICY "climate_2years" ON "mydb" 
  DURATION 104w 
  REPLICATION 1

-- Use for SHT3x data
[SHT3x]
InfluxDB_Measurement = climate
```

**Shorter retention for temperature:**
```sql
-- Only keep temperature data 90 days
CREATE RETENTION POLICY "temp_90days" ON "mydb" 
  DURATION 90d 
  REPLICATION 1

[DS18B20]
InfluxDB_Measurement = temperature
```

### Downsampling Strategy

Keep raw data short-term, aggregates long-term:

```sql
-- Downsample to hourly averages
CREATE CONTINUOUS QUERY "cq_hourly_climate" ON "mydb"
BEGIN
  SELECT 
    mean("sht3x_temperature") AS "temperature",
    mean("sht3x_humidity") AS "humidity"
  INTO "longterm"."environment_hourly"
  FROM "environment"
  GROUP BY time(1h)
END
```

Result:
- Raw data: 90 days retention
- Hourly averages: 2 years retention
- Storage optimized

## Migration and Changes

### Changing Measurement Name

When changing measurement names, old data remains in old measurement.

**Step 1: Update configuration**
```ini
[SHT3x]
InfluxDB_Measurement = new_measurement_name  # Changed from environment
```

**Step 2: Update Grafana dashboards**
```sql
-- Change FROM clause
SELECT * FROM "new_measurement_name" WHERE $timeFilter
```

**Step 3: Optional - migrate old data**
```sql
-- Copy old data to new measurement
SELECT * INTO "new_measurement_name" FROM "environment" GROUP BY *
```

**Step 4: Optional - delete old data**
```sql
DROP MEASUREMENT "environment"
```

## Troubleshooting

**Measurement not appearing:**
1. Check InfluxDB_Enable = true
2. Verify sensor is reading (Info page)
3. Check device logs for write errors
4. Verify InfluxDB server is accessible
5. Run: `SHOW MEASUREMENTS` in InfluxDB CLI

**Wrong measurement name in database:**
- Check configuration spelling
- Verify no typos in InfluxDB_Measurement
- Case-sensitive - `environment` ≠ `Environment`

**Cannot query measurement:**
```sql
-- Check if measurement exists
SHOW MEASUREMENTS

-- Check fields in measurement
SHOW FIELD KEYS FROM "environment"

-- Should show: sht3x_temperature, sht3x_humidity
```

**Grafana can't find measurement:**
1. Verify datasource connection
2. Check database name is correct
3. Confirm measurement name (case-sensitive)
4. Try refresh schema in Grafana
5. Use InfluxDB CLI to verify data exists

**Data in wrong measurement:**
- Configuration was changed after data was written
- Old data stays in old measurement
- New data goes to new measurement
- Either migrate data or accept both measurements exist

## Performance Considerations

### Query Performance

**Optimize queries:**
- Index by time automatically applied
- Use WHERE time clauses to limit data
- Avoid SELECT * on large datasets
- Use GROUP BY time($__interval) in Grafana
- Consider downsampling for old data

**Measurement size impact:**
- Fewer measurements = simpler queries
- More measurements = better organization
- Typically not a concern for sensor data volumes

### Write Performance

**Single measurement** (all sensors):
- Fewer measurement lookups
- Simpler write path
- Good for < 1000 writes/second

**Multiple measurements** (per sensor type):
- Better write distribution
- Easier to partition data
- Better for > 1000 writes/second

For typical home automation: Single measurement is fine

## Related Parameters

- [SHT3x InfluxDB_Enable](InfluxDB_Enable.md) - Enable InfluxDB logging
- [SHT3x Enable](Enable.md) - Enable the sensor
- [SHT3x Interval](Interval.md) - Set reading frequency
- [InfluxDB Enable](../InfluxDB/Enable.md) - Global InfluxDB configuration
- [InfluxDB Database](../InfluxDB/Database.md) - InfluxDB database name
