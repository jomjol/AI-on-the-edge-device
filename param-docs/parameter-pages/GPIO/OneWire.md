# GPIO 1-Wire Mode

Configure a GPIO pin for 1-Wire protocol used by DS18B20 temperature sensors.

> **📍 Configuration Context**: This GPIO setting is part of the **advanced/expert settings** in the web UI. You need to configure this **before** enabling DS18B20 sensors. See [DS18B20 Enable](../DS18B20/Enable.md) for complete setup instructions.

## GPIO12 Not Available

**Note:** GPIO12 is not available for 1-Wire in the configuration UI. This is a boot strapping pin that requires GPIO12 to be LOW at boot for standard 3.3V flash. 1-Wire sensors need a hardware pull-up resistor that holds GPIO12 HIGH during boot, causing boot failure.

**Use GPIO3 or GPIO1 instead** (disables USB serial logging). See [IO12 documentation](IO12.md) for technical details on why WS2812 LEDs work but 1-Wire sensors don't.

## Value
Select `onewire` from the GPIO mode dropdown.

## When to Use This Setting

Configure a GPIO pin as `onewire` when you want to:
- Connect DS18B20 temperature sensor(s) to monitor device temperature
- Enable environmental monitoring for outdoor installations
- Set up multi-sensor temperature chains for thermal analysis

**Next Step**: After configuring this GPIO pin, go to the [DS18B20] configuration section and set `Enable = true`.

## Compatible GPIO Pins

| GPIO | Recommended | Notes |
|------|-------------|-------|
| **IO3** | ✅ **YES** | UART RX - disables USB serial logging |
| **IO1** | ✅ **YES** | UART TX - disables USB serial logging |
| **IO13** | ✅ **YES** | Safe - 1-line SD mode is active (SD uses only GPIO2, 14, 15) |
| **IO12** | ❌ **NEVER USE** | **STRAPPING PIN** - pull-up will prevent boot! |
| **IO0** | ❌ **NEVER USE** | Boot mode selection strapping pin |

**Recommended**: GPIO3 or GPIO1 (disables USB serial logging), or GPIO13 (preserves USB serial logging)

## Wiring

### Single DS18B20
```
DS18B20 Sensor    ESP32-CAM
--------------    ---------
VDD       -----> 3.3V
GND       -----> GND
DATA      -----> GPIO3 (with 4.7kΩ pull-up to 3.3V) *

* Using GPIO3/GPIO1 will disable USB serial logging in configuration
```

### Chained DS18B20 (Multiple Sensors)
```
            4.7kΩ Pull-up
3.3V -------|-----|
            |     |
          Data    |
ESP32 ---|--+     |
         |  |     |
    DS18B20_1     |
         |  |     |
    DS18B20_2     |
         |  |     |
    DS18B20_3     |
         |        |
GND  ---|--------|
```

All sensors share the same 3 wires. Each is identified by its unique 64-bit ROM code.

## Multiple Sensor Use Case

**Indoor vs. Outdoor Temperature Monitoring:**

Chain 3 DS18B20 sensors on one GPIO:
1. **Inside enclosure** - Monitor electronics temperature
2. **Outside enclosure** - Ambient air temperature
3. **Water pipe** - Actual measured object temperature

Each publishes to MQTT with ROM ID for identification:
- `enclosure/temperature/28-01234567890A` (inside)
- `enclosure/temperature/28-ABCDEF123456` (outside)
- `enclosure/temperature/28-FEDCBA987654` (pipe)

## Safety Considerations

⚠️ **Do NOT use these pins**:
- **IO12** - **CRITICAL: Strapping pin!** Pull-up resistor will prevent boot entirely
- **IO0** - Boot mode selection strapping pin
- **IO2** - SD card D0
- **IO4** - SD card D1 / Flash LED
- **IO14, IO15** - SD card CLK/CMD

✅ **SAFE TO USE**:
- **IO13** - Free in 1-line SD mode (has internal pull-up enabled)

## Related Parameters

- [DS18B20 Enable](../DS18B20/Enable.md) - Enable the sensor
- [DS18B20 Interval](../DS18B20/Interval.md) - Reading frequency

## Example Configuration

```ini
[GPIO]
IO3 = onewire
# NOTE: USB serial logging must be disabled when using GPIO1/GPIO3

[DS18B20]
Enable = true
Interval = 300           ; Read every 5 minutes
MQTT_Enable = true
MQTT_Topic = enclosure/temperature
```

## Sensor Chaining Benefits

### Multi-Point Monitoring
- **Temperature gradient analysis**: Detect hot spots
- **Thermal management**: Track cooling effectiveness
- **Failure prediction**: Spot abnormal temperature rises
- **Environment vs. Device**: Compare ambient to internal temps

### Example Thermal Monitoring
```
Outdoor Meter Reader Installation:
├─ DS18B20 #1: Outside air (-15°C to +45°C range)
├─ DS18B20 #2: Inside enclosure (monitor for overheating)
└─ DS18B20 #3: ESP32-CAM module (direct component temp)

Alert if:
- Outside < -20°C → Activate heater
- Enclosure > 60°C → Cooling issue
- Camera module > 70°C → Thermal shutdown imminent
```

## Troubleshooting

**Device won't boot when sensor connected:**
- **CAUSE**: GPIO12 strapping pin conflict with pull-up resistor
- **SOLUTION**: Use GPIO3 or GPIO1 instead of GPIO12
- **Boot error logs**: May show `invalid header: 0xffffffff` or `ets_main.c` errors

**No sensor detected:**
1. Check wiring and 4.7kΩ pull-up resistor (to 3.3V, not 5V!)
2. Verify sensor is DS18B20 (not DHT22 or other)
3. Confirm USB logging is disabled if using GPIO1/GPIO3
4. Test sensor individually before chaining
5. Check ROM IDs in logs

**Intermittent readings:**
- Cable too long (max ~100m for reliable operation)
- Electromagnetic interference
- Weak pull-up resistor (use 4.7kΩ, not 10kΩ)
- Poor connections

**Wrong temperature values:**
- Check sensor is in correct environment
- Verify ROM ID matches physical sensor
- Sensor may be damaged (replace if readings impossible)
