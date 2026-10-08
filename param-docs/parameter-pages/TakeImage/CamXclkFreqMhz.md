# Parameter `CamXclkFreqMhz`

**Camera clock frequency (MHz)**

Optional values: `8`, `10`, `16`, `20`. Without an active value, the firmware build default is used (normally `20`).

Enable the override under **Show Expert Parameters** on the Configuration page, or add it to `config.ini`:

```ini
[TakeImage]
CamXclkFreqMhz = 8
```

Disable the checkbox or comment out the parameter to restore the build default.

The setting is read from the active `[TakeImage]` section before camera initialization and requires a reboot. The build flag `XCLK_FREQ_MHZ` remains the default when this setting is absent or invalid.

On affected ESP32-CAM boards, the camera clock causes severe Wi-Fi packet loss even when the signal is strong. Lowering the frequency can improve communication. Frequency compatibility, exposure and image quality depend on the camera module; verify real captures, alignment and meter readings after changing it.

Background: [Upstream issue #3558](https://github.com/jomjol/AI-on-the-edge-device/issues/3558).
