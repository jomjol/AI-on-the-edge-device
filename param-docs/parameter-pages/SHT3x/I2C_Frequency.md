# SHT3x I2C_Frequency

Set the I²C bus frequency (clock speed) for communication with the SHT3x sensor.

## Value
- `100000` - 100 kHz (default, most reliable)
- `400000` - 400 kHz (fast mode, for short wires)

## Description

The I²C bus frequency determines how fast data is transferred between the ESP32 and the SHT3x sensor.

### Standard Mode - 100 kHz (Recommended)

**Default: 100000 Hz (100 kHz)**

Use this setting for:
- **Maximum reliability** - Works in most conditions
- **Longer wire runs** - Up to 1 meter without issues
- **Noisy environments** - Better noise immunity
- **Multiple sensors** - When sharing the I²C bus

**Advantages:**
- More robust communication
- Less sensitive to capacitance on the bus
- Lower EMI (electromagnetic interference)
- Works with longer wires and more devices

### Fast Mode - 400 kHz

**Advanced: 400000 Hz (400 kHz)**

Use this setting only if:
- Wire length is **very short** (< 10 cm)
- You need **faster read times** (rare requirement)
- Bus capacitance is low (few devices, short traces)

**Considerations:**
- More susceptible to noise and interference
- Requires shorter wires
- May fail with poor connections or long cable runs
- Not recommended for outdoor or industrial installations

## ⚠️ Prerequisites - GPIO Configuration Required!

**Before configuring I²C frequency, you MUST set up GPIO pins for I²C communication.**

### Quick GPIO Setup

1. Navigate to the **Configuration** page
2. Scroll to the **GPIO** section
3. Click **"Show Expert Options"** or **"Advanced Settings"** to reveal GPIO configuration
4. Configure two pins:
   - One pin set to **`i2c-sda`** (e.g., IO3) *
   - One pin set to **`i2c-scl`** (e.g., IO1) *
5. Save configuration and restart

See detailed guides:
- [GPIO I²C SDA Configuration](../GPIO/I2C-SDA.md)
- [GPIO I²C SCL Configuration](../GPIO/I2C-SCL.md)
- [SHT3x Enable](Enable.md) - Complete setup walkthrough

## When to Change from Default

### Keep at 100 kHz if:
- ✅ Sensor works reliably (default setting)
- ✅ Using standard wiring (jumper wires, breadboard)
- ✅ Wire length > 10 cm
- ✅ Outdoor or harsh environment
- ✅ Multiple I²C devices on the bus

### Consider 400 kHz only if:
- ⚠️ Very short, direct PCB traces or custom board
- ⚠️ Need faster update rates (usually not needed)
- ⚠️ Confirmed stable communication at higher speed
- ⚠️ Indoor, controlled environment

**Default is best for 99% of users.**

## Configuration Example

```ini
[GPIO]
IO3 = i2c-sda                ; * Requires disabling USB logging
IO1 = i2c-scl                ; * Requires disabling USB logging

[SHT3x]
Enable = true
Address = 0x44
Interval = -1
I2C_Frequency = 100000   ; Standard mode (recommended)
```

## Troubleshooting

**Sensor not detected or intermittent readings:**
1. **First, try lowering frequency** to 100000 Hz
2. Check wire length - should be < 50 cm for 100 kHz
3. Verify 4.7kΩ pull-up resistors are installed on SDA and SCL
4. Inspect for loose connections or poor solder joints
5. Check for electrical noise sources near I²C wires
6. Ensure GPIO pins are properly configured

**Changing from 400 kHz to 100 kHz:**
If experiencing issues at 400 kHz:
1. Set `I2C_Frequency = 100000`
2. Save and restart device
3. Issues should resolve if related to bus speed

**Fast mode (400 kHz) not working:**
- Reduce wire length to < 10 cm
- Use twisted pair wiring (twist SDA and GND together, SCL and VDD together)
- Add/reduce pull-up resistor values (try 2.2kΩ for very short wires)
- Verify no other I²C devices conflict
- Fall back to 100 kHz (recommended)

## Technical Background

### I²C Bus Capacitance

The maximum speed is limited by **total bus capacitance** from:
- Wire length (longer = more capacitance)
- Number of devices connected
- PCB trace capacitance
- Connector capacitance

**Standard Mode (100 kHz):** Works up to 400 pF bus capacitance  
**Fast Mode (400 kHz):** Requires < 200 pF bus capacitance

Typical wire: ~50-100 pF per meter → stick to 100 kHz for reliability.

### Performance Impact

A single SHT3x read takes ~15-20 ms regardless of I²C speed because:
- Sensor measurement time is 12.5 ms (fixed by sensor)
- Data transfer is < 1 ms even at 100 kHz

**Conclusion:** Using 400 kHz provides negligible benefit for typical sensor applications.

## Related Parameters

- [SHT3x Enable](Enable.md) - Enable the sensor
- [SHT3x Address](Address.md) - I²C address selection
- [SHT3x Interval](Interval.md) - Reading frequency
- [GPIO I²C SDA](../GPIO/I2C-SDA.md) - Data line configuration
- [GPIO I²C SCL](../GPIO/I2C-SCL.md) - Clock line configuration
