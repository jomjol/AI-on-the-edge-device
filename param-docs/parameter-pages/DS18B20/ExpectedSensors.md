# DS18B20 ExpectedSensors

Define the expected number of DS18B20 sensors on the 1-Wire bus to enable retry logic during initialization.

## Value
- `-1` - Auto-detect mode (default) - Accept any number of sensors found on first successful scan
- Positive integer (e.g., `1`, `2`, `3`) - Expected sensor count with validation and retry

## Description

Controls the ROM search behavior during sensor initialization to improve reliability in environments with electrical interference or timing issues.

### Auto-Detect Mode (ExpectedSensors = -1)

**Default behavior - Recommended for most users**

The system performs a ROM search and accepts whatever sensors are found on the first successful scan.

**Behavior:**
- Performs ROM search once at startup
- Accepts any positive number of sensors found
- No retries performed
- Fast initialization (typical: 100-200ms per sensor)

**Use when:**
- Sensor count is flexible or unknown
- Reliable electrical environment
- Quick startup is priority
- Testing/development with varying sensor counts

**Example:**
```ini
[DS18B20]
ExpectedSensors = -1        ; Auto-detect (default)
```

### Validation Mode (ExpectedSensors > 0)

**For production installations with known sensor counts**

The system validates the detected sensor count against the expected value and retries if there's a mismatch.

**Behavior:**
- Performs up to 5 ROM search attempts
- Validates detected count matches expected value
- Increasing retry delays: 100ms, 150ms, 200ms, 250ms, 300ms
- Keeps best result (highest sensor count) if retries exhausted
- Continues initialization with partial detection if needed

**Use when:**
- Fixed installation with known sensor count
- Noisy electrical environment (interference)
- Timing issues during startup
- Need validation that all sensors are detected
- Production/critical monitoring systems

**Example:**
```ini
[GPIO]
IO3 = onewire              ; * 3 DS18B20 sensors wired in parallel

[DS18B20]
ExpectedSensors = 3         ; Expect exactly 3 sensors
MQTT_Enable = true
```
\* Requires disabling USB serial logging. **DO NOT use GPIO12** - boot strapping pin prevents boot!

## How Retry Logic Works

When `ExpectedSensors` is set to a positive value, the initialization process becomes more robust:

### Retry Sequence

1. **First Attempt (0ms delay)**
   - Performs ROM search
   - If count matches expected → Success, continues
   - If count < expected → Retry

2. **Retry 1 (100ms delay)**
   - Waits 100ms for bus to stabilize
   - Performs ROM search
   - If count matches expected → Success
   - Otherwise, continues retrying

3. **Retries 2-5 (150ms, 200ms, 250ms, 300ms delays)**
   - Progressive delays allow more time for bus stabilization
   - Each retry performs fresh ROM search
   - Tracks highest sensor count seen across all attempts

4. **After 5 Attempts**
   - If expected count achieved → Success
   - If not, uses best result (most sensors found)
   - Logs warning but continues operation
   - Better to have partial data than fail completely

### Example Scenarios

**Scenario 1: Perfect Detection**
```
ExpectedSensors = 3
Attempt 1: Found 3 sensors → Success immediately
Total time: ~200ms
```

**Scenario 2: Timing Issue Resolved by Retry**
```
ExpectedSensors = 3
Attempt 1: Found 2 sensors → Retry
Attempt 2 (100ms delay): Found 3 sensors → Success
Total time: ~400ms
```

**Scenario 3: Partial Detection (Hardware Issue)**
```
ExpectedSensors = 3
Attempt 1: Found 2 sensors → Retry
Attempt 2: Found 2 sensors → Retry
Attempt 3: Found 2 sensors → Retry
Attempt 4: Found 2 sensors → Retry
Attempt 5: Found 2 sensors → Uses best result
Warning: "Found 2 sensor(s) but expected 3 - continuing with detected sensors"
Total time: ~1200ms
```

## Choosing the Right Value

### Use ExpectedSensors = -1 (Auto-Detect) When:

✅ **Development/Testing**
- Experimenting with different sensor configurations
- Don't know final sensor count yet
- Rapid prototyping phase

✅ **Flexible Installations**
- Sensor count may vary over time
- Hot-swapping sensors (requires restart anyway)
- Not critical if count changes

✅ **Known Reliable Setup**
- No electrical interference
- Strong, short wiring
- Indoor, controlled environment
- Never had detection issues

### Use ExpectedSensors = 1, 2, 3, etc. When:

✅ **Production Systems**
- Fixed installation with known sensor count
- Need validation during startup
- Early warning if sensor fails

✅ **Challenging Environments**
- Long wire runs (>5 meters)
- Electrical noise (motors, pumps nearby)
- Outdoor installations
- Have experienced detection issues before

✅ **Critical Monitoring**
- All sensors must be operational
- Want to catch hardware failures early
- Compliance/regulatory requirements

## Performance Impact

### Auto-Detect Mode (ExpectedSensors = -1)
- **Best case**: Single ROM search (~200ms for 3 sensors)
- **Worst case**: Same as best case
- **Typical startup**: 200-500ms

### Validation Mode (ExpectedSensors = 3)
- **Best case**: Single ROM search (~200ms) if all sensors found immediately
- **Worst case**: 5 retries with delays (~2 seconds)
- **Typical startup**: 200-800ms (most succeed on first or second try)

The additional time is negligible during device startup and provides significant reliability improvement.

## Best Practices

### Wiring for Reliable Detection

Even with retry logic, proper wiring is essential:

**Do's:**
- Use proper pull-up resistor (4.7kΩ recommended)
- Keep wires as short as practical
- Use twisted pair or shielded cable for long runs
- Secure all connections
- Use quality sensors

**Don'ts:**
- Don't use wire runs longer than 10 meters without special consideration
- Don't connect/disconnect sensors while powered (restart required anyway)
- Don't mix different cable types in same bus
- Don't use weak/loose connections

### Recommended Configurations

**1-2 Sensors, Short Wires (<3m), Indoor:**
```ini
ExpectedSensors = -1        ; Auto-detect is fine
```

**3+ Sensors OR Long Wires (>3m) OR Outdoor:**
```ini
ExpectedSensors = 3         ; Your actual count
```

**Testing New Installation:**
```ini
ExpectedSensors = -1        ; Start with auto-detect
```
Monitor logs for detection issues. If problems occur, switch to exact count.

## Troubleshooting

### Warning: "Found 2 sensor(s) but expected 3"

**Possible causes:**
1. **Wiring issue**: Check connections, solder joints
2. **Sensor failure**: One sensor may be defective
3. **Power issue**: Insufficient current for all sensors
4. **Cable too long**: Signal degradation over distance
5. **Interference**: Nearby electrical noise

**Steps:**
1. Check device logs for ROM IDs detected
2. Physically verify all sensors are connected
3. Test sensors individually to isolate faulty one
4. Check power supply voltage (3.3V stable)
5. Reduce wire length if possible
6. Add/check pull-up resistor

### Field Locked/Disabled in UI

If the ExpectedSensors field appears locked:
1. Ensure this documentation file exists (it does now!)
2. Rebuild web interface tooltips (done during build process)
3. Hard refresh browser (Ctrl+Shift+R)

### No Sensors Detected at All

If no sensors are detected even with retries:
1. Verify GPIO pin is configured as `onewire`
2. Check physical wiring (data, ground, power)
3. Verify sensors are DS18B20 (not DHT22 or other types)
4. Test sensor(s) with different device/Arduino to confirm working
5. Check pull-up resistor is present and correct value

## Example Configurations

### Example 1: Simple Indoor Setup
**Scenario**: Single sensor monitoring enclosure temperature

```ini
[GPIO]
IO3 = onewire               ; * Requires disabling USB logging

[DS18B20]
ExpectedSensors = 1         ; Validate sensor detected
Interval = -1
MQTT_Enable = true
MQTT_Topic = enclosure/temperature
```

**Benefit**: Immediate alert at startup if sensor disconnected or failed.

### Example 2: Multi-Point Monitoring
**Scenario**: Three sensors (inside, outside, pipe) in outdoor enclosure

```ini
[GPIO]
IO3 = onewire              ; * 3 sensors on same bus

[DS18B20]
ExpectedSensors = 3         ; Validate all 3 detected
Interval = 300              ; Read every 5 minutes
MQTT_Enable = true
MQTT_Topic = outdoor/temperature
InfluxDB_Enable = true
InfluxDB_Measurement = environment
```

**Benefit**: Retry logic compensates for outdoor electrical interference. Logs warning if sensor count drops (maintenance needed).

### Example 3: Development/Testing
**Scenario**: Experimenting with different sensor configurations

```ini
[GPIO]
IO3 = onewire               ; * Requires disabling USB logging

[DS18B20]
ExpectedSensors = -1        ; Auto-detect whatever is connected
Interval = -1
MQTT_Enable = true
```

**Benefit**: No need to update config when adding/removing test sensors.

## Log Output Examples

### Successful Detection with Expected Count
```
[INFO] DS18B20: Initializing DS18B20 sensor on GPIO3
[INFO] DS18B20: Expected sensor count: 3
[INFO] DS18B20: === DS18B20 ROM Search (startup only) ===
[INFO] DS18B20: Scanning 1-Wire bus for DS18B20 devices...
[INFO] DS18B20: ROM search found expected 3 sensor(s) on retry 1
[INFO] DS18B20: ROM search complete: Found 3 DS18B20 sensor(s)
[INFO] DS18B20: Discovered ROM IDs (will be used for all future reads):
[INFO] DS18B20:   Sensor #1: 28-0123456789AB
[INFO] DS18B20:   Sensor #2: 28-FEDCBA987654
[INFO] DS18B20:   Sensor #3: 28-AABBCCDD1122
[INFO] DS18B20: === DS18B20 initialization complete ===
```

### Detection with Retry
```
[INFO] DS18B20: Expected sensor count: 3
[INFO] DS18B20: Scanning 1-Wire bus for DS18B20 devices...
[WARN] DS18B20: ROM search found 2 sensor(s), expected 3
[WARN] DS18B20: ROM search retry 2 after 150ms
[INFO] DS18B20: ROM search found expected 3 sensor(s) on retry 2
[INFO] DS18B20: ROM search complete: Found 3 DS18B20 sensor(s)
```

### Partial Detection (Warning)
```
[INFO] DS18B20: Expected sensor count: 3
[WARN] DS18B20: ROM search found 2 sensor(s), expected 3
[WARN] DS18B20: ROM search retry 2 after 150ms
[WARN] DS18B20: ROM search found 2 sensor(s), expected 3
[WARN] DS18B20: ROM search retry 3 after 200ms
[WARN] DS18B20: ROM search found 2 sensor(s), expected 3
[WARN] DS18B20: ROM search retry 4 after 250ms
[WARN] DS18B20: ROM search found 2 sensor(s), expected 3
[WARN] DS18B20: ROM search retry 5 after 300ms
[WARN] DS18B20: Using best ROM search result: 2 sensor(s)
[WARN] DS18B20: Found 2 sensor(s) but expected 3 - continuing with detected sensors
[INFO] DS18B20: ROM search complete: Found 2 DS18B20 sensor(s)
```
This indicates a hardware issue - check wiring and sensors.

## Related Parameters

- [DS18B20 Interval](Interval.md) - Set reading frequency
- [DS18B20 MQTT Enable](MQTT_Enable.md) - Publish to MQTT
- [DS18B20 MQTT Topic](MQTT_Topic.md) - MQTT topic configuration
- [DS18B20 InfluxDB Enable](InfluxDB_Enable.md) - Log to InfluxDB
- [DS18B20 InfluxDB Measurement](InfluxDB_Measurement.md) - InfluxDB measurement name
- [GPIO OneWire](../GPIO/OneWire.md) - GPIO pin configuration for 1-Wire bus

## Technical Details

### ROM Search Algorithm

The DS18B20 uses the Dallas/Maxim 1-Wire protocol for device enumeration:

1. **Bus Reset**: Master pulls line low, sensors respond with presence pulse
2. **Search ROM Command (0xF0)**: Initiates device discovery
3. **Bit-by-bit Search**: Master reads bit + complement bit for each ROM bit
4. **Discrepancy Resolution**: Handles multiple devices on same bus
5. **CRC Validation**: Ensures ROM ID integrity (8-bit CRC)
6. **Family Code Check**: Verifies device is DS18B20 (0x28)

### Why Retries Help

**Timing Issues:**
- 1-Wire protocol is software-based (bit-banged)
- Sensitive to CPU load and interrupts
- ESP32 background tasks can cause timing variations
- Retry with delay allows system to stabilize

**CRC Errors:**
- Electrical noise can corrupt data
- Long wires increase susceptibility
- Retry often succeeds on second attempt
- Multiple attempts improve overall reliability

**Bus Initialization:**
- Some sensors need time to power up
- Capacitance in long wires affects timing
- First reset may not detect all devices
- Delay + retry gives sensors time to stabilize

### Implementation Notes

- Retry logic is **only during initialization** (not on every read)
- ROM IDs are **cached** after successful detection
- Future reads use cached ROM IDs (no re-scanning)
- Retries add 0-2 seconds to startup time (one-time cost)
- No runtime performance impact after initialization

## Migration Guide

If upgrading from a version without ExpectedSensors parameter:

**Existing config files:**
- Parameter is optional
- Default value is `-1` (auto-detect)
- No changes needed to existing configs
- Behavior remains the same unless you set ExpectedSensors

**To enable retry logic:**
1. Count your connected DS18B20 sensors
2. Add `ExpectedSensors = <count>` to DS18B20 section
3. Save and restart device
4. Check logs to verify detection

**Example migration:**
```ini
# Before (still works):
[DS18B20]
Interval = -1
MQTT_Enable = true

# After (adds validation):
[DS18B20]
ExpectedSensors = 3         ; NEW: Enable retry logic
Interval = -1
MQTT_Enable = true
```

## Summary

- **Default (-1)**: Auto-detect, fast, works for most cases
- **Positive integer**: Validates count, enables retries, more robust
- **Use exact count**: For production systems with known hardware
- **Use auto-detect**: For development, testing, flexible setups
- **Retry logic**: Improves reliability without runtime overhead
- **Backward compatible**: Existing configs continue working unchanged
