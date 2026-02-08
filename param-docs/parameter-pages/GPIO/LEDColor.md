# Parameter `LEDColor`
Default Value: `150 150 150`

Color of the attached WS2812 LEDs on GPIO12 in **R**ed, **G**reen, **B**lue format, from `0` (full off) to `255` (full on).

## Value Format

```
LEDColor = R G B
```

Where:
- **R** (Red): 0-255
- **G** (Green): 0-255  
- **B** (Blue): 0-255

## Examples

```ini
LEDColor = 255 0 0     # Bright red
LEDColor = 0 255 0     # Bright green
LEDColor = 0 0 255     # Bright blue
LEDColor = 255 255 255 # White (full brightness)
LEDColor = 150 150 150 # White (medium brightness, default)
LEDColor = 255 128 0   # Orange
LEDColor = 128 0 128   # Purple
```

## Usage

This setting controls the color of WS2812/NeoPixel LEDs connected to GPIO12, typically used as:
- External camera flash illumination
- Status indicator lights
- Decorative RGB lighting

## GPIO12 Compatibility

**Note:** WS2812 LEDs are safe to use on GPIO12 (unlike I²C or 1-Wire sensors) because they do not use pull-up resistors and their idle state is LOW, which does not interfere with the ESP32 boot strapping pin behavior.

See [IO12 Parameter](IO12.md) and [LEDType](LEDType.md) for more details.
