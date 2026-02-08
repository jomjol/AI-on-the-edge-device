# GPIO I²C SDA Mode

Configure a GPIO pin as I²C SDA (data line) for SHT3x temperature & humidity sensor.

> **📍 Configuration Context**: This GPIO setting is part of the **advanced/expert settings** in the web UI. You need to configure **both SDA and SCL** pins **before** enabling the SHT3x sensor. See [SHT3x Enable](../SHT3x/Enable.md) for complete setup instructions.

## Value
Select `i2c-sda` from the GPIO mode dropdown.

## When to Use This Setting

Configure a GPIO pin as `i2c-sda` when you want to:
- Connect an SHT3x sensor to monitor temperature and humidity
- Enable environmental monitoring for outdoor installations
- Track enclosure conditions to prevent device failures

**Important**: You must also configure a second GPIO pin as `i2c-scl` (clock line). See [GPIO I²C SCL](I2C-SCL.md).

**Next Step**: After configuring both SDA and SCL GPIO pins, go to the [SHT3x] configuration section and set `Enable = true`.

## GPIO12 Not Available

**Note:** GPIO12 is not available for I²C SDA in the configuration UI. This is a boot strapping pin that requires GPIO12 to be LOW at boot for standard 3.3V flash. I²C sensors need hardware pull-up resistors that hold GPIO12 HIGH during boot, causing boot failure.

**Use GPIO1 or GPIO3 instead** (disables USB serial logging). See [IO12 documentation](IO12.md) for technical details on why WS2812 LEDs work but I²C sensors don't.

## Compatible GPIO Pins

| GPIO | Recommended | Notes |
|------|-------------|-------|
| **IO1** | ✅ **YES** | UART TX - disables USB serial logging |
| **IO3** | ✅ **YES** | UART RX - disables USB serial logging |
| **IO13** | ✅ **YES** | Safe - 1-line SD mode is active (SD uses only GPIO2, 14, 15) |
| **IO12** | ❌ **NEVER USE** | **STRAPPING PIN** - pull-up will prevent boot! |
| **IO0** | ❌ **NEVER USE** | Boot mode selection strapping pin |

**Recommended Configuration:**
- SDA: IO3 (with 4.7kΩ pull-up to 3.3V) *disables USB serial logging*
- SCL: IO1 (with 4.7kΩ pull-up to 3.3V) *disables USB serial logging*

Alternative (preserves USB serial logging):
- SDA: IO13
- SCL: IO1 or IO3

## Wiring

```
SHT3x Sensor      ESP32-CAM
------------      ---------
VDD       -----> 3.3V
GND       -----> GND
SDA       -----> GPIO3 (with 4.7kΩ pull-up to 3.3V) *
SCL       -----> GPIO1 (with 4.7kΩ pull-up to 3.3V) *

* Using GPIO1/GPIO3 will disable USB serial logging in configuration
```

**Important**: External 4.7kΩ pull-up resistors are required for both SDA and SCL lines.

## Safety Considerations

⚠️ **Do NOT use these pins**:
- **IO12** - **CRITICAL: Strapping pin!** Pull-up resistor will prevent boot entirely
- **IO0** - Boot mode selection strapping pin
- **IO2** - SD card D0
- **IO4** - SD card D1 / Flash LED
- **IO14** - SD card CLK
- **IO15** - SD card CMD

Using these pins can prevent the device from booting or corrupt the SD card.

## Related Parameters

- [I²C SCL](GPIO-I2C-SCL.md) - Clock line configuration
- [SHT3x Enable](../SHT3x/Enable.md) - Enable the sensor
- [SHT3x I2C Frequency](../SHT3x/I2C_Frequency.md) - Bus speed configuration

## Example Configuration

```ini
[GPIO]
IO3 = i2c-sda
IO1 = i2c-scl
# NOTE: USB serial logging must be disabled when using GPIO1/GPIO3

[SHT3x]
Enable = true
Address = 0x44
I2C_Frequency = 100000
```

## Troubleshooting

**Device won't boot when sensor connected:**
- **CAUSE**: GPIO12 strapping pin conflict with pull-up resistor
- **SOLUTION**: Use GPIO1/GPIO3 instead of GPIO12
- **Boot error logs**: May show `invalid header: 0xffffffff` or `ets_main.c` errors

**Sensor not detected:**
1. Check wiring connections
2. Verify pull-up resistors are installed (4.7kΩ to 3.3V)
3. Confirm USB logging is disabled if using GPIO1/GPIO3
4. Try lowering I2C frequency to 100kHz
5. Use I²C scanner tool to detect address

**Bus errors:**
- Check for short circuits on SDA/SCL lines
- Verify sensor is powered (3.3V, not 5V!)
- Ensure proper grounding
- Check for electromagnetic interference
