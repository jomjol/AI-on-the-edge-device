# Reed Contact Support for AI-on-the-edge-device

## Overview

Reed contact support has been added to the AI-on-the-edge-device to enable intermediate meter readings between periodic image analysis. This feature is particularly useful for meters with magnets (gas, water, etc.) that rotate with each digit increment.

## Use Case

Many meters have a magnet embedded in the last digit that rotates with each unit increment. By placing a reed contact (magnetic switch) in the meter's reed contact slot, you can:

1. **Track intermediate values** - Get readings between scheduled image analysis cycles
2. **Increase temporal resolution** - Capture faster-rotating digits with high accuracy
3. **Automatic drift correction** - Image analysis serves as automatic drift correction for missed or double reed contact triggers
4. **Real-time updates** - Publish values to MQTT/Home Assistant as they change

## Configuration

### GPIO Setup

First, configure the GPIO pin as an input:

```ini
[GPIO]
IO12 = input-pullup rising-and-falling 10 false false MY_REED_CONTACT
```

Where:
- `IO12` - GPIO pin number (0, 1, 3, 4, 12, 13 are available)
- `input-pullup` - Input mode with pull-up resistor (typical for reed contacts)
- `rising-and-falling` - Interrupt type for debouncing
- `10` - PWM duty resolution (not used for input)
- `false` - MQTT enable (optional)
- `false` - HTTP enable (optional)
- `MY_REED_CONTACT` - Custom name for the GPIO

### Reed Contact Configuration

Add reed contact parameters on a separate line:

```ini
ReedContactIO12 = true on-close 500 1 true
```

Where:
- `true` - Enable reed contact feature
- `on-close` - Trigger type: `on-close` or `on-open`
- `500` - Debounce time in milliseconds (0-10000)
- `1` - Digit index to increment (0 = first/highest digit)
- `true` - Publish on increment (MQTT publication enabled)

## Configuration Parameters

### Enable Reed Contact
- **Type**: boolean (`true`/`false`)
- **Default**: `false`
- **Description**: Enables or disables the reed contact feature for this GPIO

### Trigger Type
- **Options**: `on-close`, `on-open`
- **Default**: `on-close`
- **Description**:
  - `on-close`: Triggers when the reed contact closes (magnet approaches)
  - `on-open`: Triggers when the reed contact opens (magnet leaves)
  
  The choice depends on your meter's magnet position and reed contact slot orientation.

### Debounce Time
- **Type**: integer (milliseconds)
- **Range**: 0-10000
- **Default**: 500
- **Description**: Debouncing prevents false triggers from contact bounce. A typical value of 500ms is sufficient for most reed contacts. Increase if seeing duplicate triggers, decrease if missing triggers.

### Digit Index
- **Type**: integer
- **Range**: 0-10
- **Default**: 1
- **Description**: Index of the digit to increment when reed contact triggers. Index 0 is the leftmost (most significant) digit. For most meters, the last rotating digit is at index 1 or later, depending on your configuration.

### Publish on Increment
- **Type**: boolean (`true`/`false`)
- **Default**: `true`
- **Description**: When enabled, publishes the new value via MQTT after a reed contact increment. Useful for real-time updates in home automation systems.

## Example Configurations

### Single Reed Contact (Gas Meter)

```ini
[GPIO]
IO12 = input-pullup rising-and-falling 10 false false GAS_METER

ReedContactIO12 = true on-close 500 1 true
```

### Multiple Reed Contacts (Gas + Water Meters)

```ini
[GPIO]
IO12 = input-pullup rising-and-falling 10 false false GAS_METER
IO13 = input-pullup rising-and-falling 10 false false WATER_METER

ReedContactIO12 = true on-close 500 1 true
ReedContactIO13 = true on-close 500 1 true
```

### With MQTT Publishing

```ini
[GPIO]
IO12 = input-pullup rising-and-falling 10 true false GAS_METER

ReedContactIO12 = true on-close 500 1 true
```

When MQTT is enabled, reed contact increments will be published to:
```
<MainTopic>/GPIO/GAS_METER
```

## How It Works

1. **GPIO Monitoring**: The GPIO handler continuously monitors configured GPIO pins
2. **Debouncing**: When a reed contact state change is detected, debouncing logic prevents spurious triggers
3. **Trigger Detection**: If the debounced signal matches the configured trigger type (on-close or on-open), a trigger event is generated
4. **Value Increment**: The digit value at the specified index is incremented by the smallest unit for that decimal place
5. **MQTT Publication**: If enabled, the new value is published immediately
6. **Image-based Correction**: During the next image analysis cycle, the camera-based reading serves as drift correction

## Technical Details

### Debouncing Algorithm

- On GPIO interrupt, the handler checks the current GPIO state
- If the state differs from the last recorded state and the debounce timer has elapsed, the reed contact is triggered
- The debounce timer is reset after a trigger to prevent duplicate events

### Value Increment Calculation

The increment amount is calculated as:
```
increment = 1.0 / 10^(DecimalShift + Nachkomma)
```

Where:
- `DecimalShift`: Configured decimal shift for the digit
- `Nachkomma`: Number of decimal places for the digit

This ensures proper decimal place handling for different meter types.

### Timestamp Tracking

Reed contact-triggered increments update:
- `PreValue`: The previous valid value
- `Value`: The current value
- `timeStampLastPreValue`: Timestamp of the increment

## Troubleshooting

### Reed Contact Not Triggering

1. **Check GPIO Configuration**: Verify the GPIO is configured as `input` or `input-pullup`
2. **Verify Wiring**: Ensure the reed contact is properly connected to the GPIO pin and GND
3. **Check Position**: Verify the reed contact is positioned in the meter's reed contact slot
4. **Test with Multimeter**: Check if the reed contact opens/closes as the magnet passes
5. **Change Debounce Time**: If triggers are unreliable, change debounce

### Duplicate or Missing Triggers

1. **Adjust Debounce Time**: 
   - Increase if seeing duplicate triggers (try 1000ms)
   - Decrease if missing triggers (try 250ms)
2. **Check Magnet Speed**: Very fast magnet rotations may need shorter debounce times
3. **Verify GPIO State**: Check GPIO interrupt type is set to `rising-and-falling`

### Incorrect Digit Incremented

1. **Check Digit Index**: Verify the digit index in reed contact configuration
2. **Review Decimal Configuration**: Check DecimalShift and Nachkomma settings
3. **Verify NUMBERS Array**: Ensure the digit index exists in your configuration

### MQTT Not Publishing

1. **Enable MQTT**: Set `enable MQTT` to `true` in GPIO configuration
2. **Check MQTT Settings**: Verify MQTT server is configured and connected
3. **Verify Topic**: Check if the message is published to the correct MQTT topic
4. **Check Logs**: Review device logs for MQTT-related errors

## Limitations

- Reed contacts detect individual rotations, so accuracy depends on magnet position and rotation speed
- Bouncing contacts can cause duplicate triggers (mitigated by debouncing)
- Missing triggers due to contact bounce or magnet speed (mitigated by image-based drift correction)
- Only supports predefined GPIO pins (0, 1, 3, 4, 12, 13)

## Integration with Image Analysis

Reed contact increments and image-based readings work together:

1. **Frequency**: Reed contacts provide updates whenever the magnet passes
2. **Accuracy**: Image analysis provides high-accuracy reference readings
3. **Correction**: Image analysis detects and corrects drift from missed/double reed triggers
4. **Reporting**: Both methods contribute to the final meter reading

## Related Configuration

See also:
- [GPIO Documentation](../param-docs/parameter-pages/GPIO/)
- [PostProcessing Configuration](../param-docs/parameter-pages/PostProcessing/)
- [MQTT Configuration](../param-docs/parameter-pages/MQTT/)

## Future Enhancements

Potential improvements for future versions:
- UI support for reed contact configuration in the web interface
- Additional GPIO pins support
- Advanced debouncing algorithms (e.g., state machine-based)
- Reed contact statistics and diagnostics
- Per-digit trigger configuration
