# Parameter `LEDType`
Default Value: `WS2812`

Type of WS2812x addressable RGB LED connected to GPIO12.

## Supported Types

- **WS2812** - Original (default)
- **WS2812B** - Most common variant
- **WS2812B_NEWVARIANT** - Newer timing
- **WS2812B_OLDVARIANT** - Older timing
- **WS2812C** - WS2812C variant

## GPIO12 Compatibility

WS2812 LEDs work on GPIO12 (unlike I²C/1-Wire sensors) because they use actively driven data (no pull-up resistors), so idle state is LOW and doesn't affect boot strapping.

See [IO12](IO12.md) and [LEDColor](LEDColor.md) for details.
