# DS18B20 InfluxDB Measurement

Set the InfluxDB measurement name for DS18B20 temperature sensor data.

## Value
Default: `environment`

String value representing the InfluxDB measurement name where sensor data is stored.

## Description

In InfluxDB, a **measurement** is analogous to a database table. All DS18B20 sensor readings are written to this measurement, with each sensor creating a unique field identified by its ROM ID.

## Measurement Structure

### Format
```
Measurement: {InfluxDB_Measurement}
Field: temperature_{ROM_ID}
Value: {temperature}
Timestamp: {read_time}
```

### Example with Single Sensor
```
Measurement: environment
Time: 2024-01-15T14:30:00Z
Field: temperature_28-0123456789AB = 24.5
```

### Example with Multiple Sensors
```
Measurement: environment
Time: 2024-01-15T14:30:00Z
Fields:
  temperature_28-0123456789AB = 24.5  (Inside)
  temperature_28-FEDCBA987654 = 15.2  (Outside)
  temperature_28-AABBCCDD1122 = 12.8  (Pipe)
```

## Naming Conventions

### Recommended Measurement Names

**By Data Category:**
```
environment      - Environmental monitoring (recommended)
sensors          - Generic sensor data
temperature      - Temperature-specific data
monitoring       - General monitoring data
device_health    - Device health metrics
```

**By Location:**
```
enclosure_env    - Enclosure environmental data
outdoor_env      - Outdoor environmental data
basement_sensors - Basement sensor readings
```

**By System:**
```
watermeter_env   - Watermeter environmental data
gasmeter_env     - Gas meter environmental data
```

### Best Practices

**DO:**
- ✅ Use lowercase names
- ✅ Use underscores for word separation
- ✅ Keep names descriptive but concise
- ✅ Use consistent naming across devices
- ✅ Group related sensors in same measurement

**DON'T:**
- ❌ Use spaces (breaks queries)
- ❌ Use special characters except underscore
- ❌ Use overly long names
- ❌ Mix unrelated data in same measurement
- ❌ Include ROM IDs in measurement name (they're in field names)

## Sharing Measurements

### Multiple Sensor Types in One Measurement

You can store multiple sensor types together:

```ini
[SHT3x]
InfluxDB_Measurement = environment

[DS18B20]
InfluxDB_Measurement = environment
```

Result in InfluxDB:
```
Measurement: environment
Time: 2024-01-15T14:30:00Z
Fields:
  temperature = 24.5                      (from SHT3x)
  humidity = 45.2                         (from SHT3x)
  temperature_28-0123456789AB = 24.3      (from DS18B20 #1)
  temperature_28-FEDCBA987654 = 15.2      (from DS18B20 #2)
```

**Benefits:**
- All environmental data in one place
- Easy to query together
- Correlate different sensor types

### Separate Measurements per Sensor Type

Or keep them separate:

```ini
[SHT3x]
InfluxDB_Measurement = climate

[DS18B20]
InfluxDB_Measurement = temperature
```

**Benefits:**
- Clear separation
- Independent retention policies
- Easier to manage if one sensor type is temporary

## Grafana Queries

### Query Single Measurement
```sql
SELECT 
  mean("temperature_28-0123456789AB") as "Inside",
  mean("temperature_28-FEDCBA987654") as "Outside"
FROM "environment"
WHERE $timeFilter
GROUP BY time($__interval)
```

### Query Multiple Measurements
```sql
SELECT 
  mean("temperature") as "SHT3x",
  mean("temperature_28-0123456789AB") as "DS18B20"
FROM "climate", "temperature"
WHERE $timeFilter
GROUP BY time($__interval)
```

### All Fields in Measurement
```sql
SHOW FIELD KEYS FROM "environment"
```

Returns:
```
temperature
humidity
temperature_28-0123456789AB
temperature_28-FEDCBA987654
temperature_28-AABBCCDD1122
```

## Use Cases

### Case 1: Unified Environmental Monitoring
**Goal:** All environmental data together

```ini
[SHT3x]
InfluxDB_Measurement = environment

[DS18B20]
InfluxDB_Measurement = environment
```

Grafana dashboard shows:
- Inside temperature (SHT3x)
- Inside humidity (SHT3x)
- Outside temperature (DS18B20)
- Pipe temperature (DS18B20)

All correlated by timestamp.

### Case 2: Separate Temperature Measurements
**Goal:** Different retention for different sensors

```ini
[SHT3x]
InfluxDB_Measurement = climate_realtime

[DS18B20]
InfluxDB_Measurement = temperature_longterm
```

InfluxDB retention policies:
```sql
-- Keep realtime data for 7 days
CREATE RETENTION POLICY "realtime" ON "mydb" DURATION 7d REPLICATION 1

-- Keep long-term temperature for 2 years
CREATE RETENTION POLICY "longterm" ON "mydb" DURATION 730d REPLICATION 1
```

Different policies for different needs.

### Case 3: Multi-Device Installation
**Goal:** Multiple devices writing to same InfluxDB

```ini
# Device 1: Main watermeter
[DS18B20]
InfluxDB_Measurement = watermeter1_environment

# Device 2: Secondary watermeter
[DS18B20]
InfluxDB_Measurement = watermeter2_environment
```

Keeps data from different devices separate.

## Measurement Tags (Advanced)

InfluxDB measurements can have tags for better organization. While the DS18B20 implementation doesn't currently add tags, you could enhance it to add:

**Potential future tags:**
- `location`: inside/outside/pipe
- `device_id`: unique device identifier
- `sensor_type`: DS18B20

This would enable queries like:
```sql
SELECT mean("temperature_*")
FROM "environment"
WHERE "location" = 'outside'
AND $timeFilter
```

## Performance Considerations

### Write Performance
- Multiple sensors writing to same measurement: No performance impact
- InfluxDB handles many fields efficiently
- Typical write: <1ms

### Query Performance
- Fewer measurements = Simpler queries
- Many fields in one measurement: Still fast
- Use appropriate time ranges and GROUP BY

### Storage
Measurement name has minimal storage impact:
- Short name: `env` = 3 bytes
- Long name: `watermeter_environment` = 23 bytes
- Difference per point: ~20 bytes
- Over 1 million points: ~20 MB difference

Use readable names; storage difference is negligible.

## Changing Measurements

**Important:** Changing the measurement name creates a NEW measurement. Old data remains in the old measurement.

**Migration Steps:**
1. Note current measurement name
2. Update configuration with new name
3. Wait for data to accumulate in new measurement
4. Update Grafana queries to use new measurement
5. Optionally: Migrate old data or set up parallel queries

**Example Migration Query:**
```sql
-- Copy data from old to new measurement
SELECT * INTO "new_environment" FROM "environment" WHERE time > now() - 30d
```

## Troubleshooting

**Data not appearing:**
1. Verify InfluxDB is enabled and connected
2. Check measurement name doesn't have typos
3. Use InfluxDB CLI to check measurements:
   ```
   SHOW MEASUREMENTS
   ```
4. Query specific measurement:
   ```
   SELECT * FROM "environment" ORDER BY time DESC LIMIT 10
   ```

**Can't find measurement in Grafana:**
- Refresh Grafana datasource
- Check measurement name spelling
- Verify data has been written (check with CLI)
- Confirm correct database selected

**Wrong field structure:**
- Each DS18B20 sensor should create `temperature_{ROM_ID}` field
- Check ROM IDs in device logs
- Verify sensors are reading successfully

**Measurement name conflicts:**
- Ensure different devices use different measurements
- Or use tags to differentiate
- Check existing measurements: `SHOW MEASUREMENTS`

## Related Parameters

- [DS18B20 Enable](Enable.md) - Enable the sensor
- [DS18B20 Interval](Interval.md) - Reading interval
- [DS18B20 InfluxDB Enable](InfluxDB_Enable.md) - Enable InfluxDB logging
- [InfluxDB Enable](../InfluxDB/Enable.md) - Global InfluxDB configuration
- [InfluxDB Database](../InfluxDB/Database.md) - Database name

## Example Complete Configuration

### Simple Configuration
```ini
[InfluxDB]
Enable = true
Uri = http://192.168.1.100:8086
Database = homeautomation

[DS18B20]
Enable = true
Interval = 300
InfluxDB_Enable = true
InfluxDB_Measurement = environment
```

### Advanced Multi-Sensor Configuration
```ini
[InfluxDB]
Enable = true
Uri = http://192.168.1.100:8086
Database = homeautomation
User = influx_user
Password = influx_pass

[SHT3x]
Enable = true
Interval = -1
InfluxDB_Enable = true
InfluxDB_Measurement = environment

[DS18B20]
Enable = true
Interval = -1
InfluxDB_Enable = true
InfluxDB_Measurement = environment
```

**Result:** All sensors write to `environment` measurement:
```
Measurement: environment
Fields:
  - temperature (from SHT3x)
  - humidity (from SHT3x)
  - temperature_28-0123456789AB (from DS18B20 #1)
  - temperature_28-FEDCBA987654 (from DS18B20 #2)
  - temperature_28-AABBCCDD1122 (from DS18B20 #3)
```

Perfect for unified environmental monitoring dashboards!
