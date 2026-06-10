# Parameter `Enabled`
Default Value: `false`

Activates battery voltage monitoring on the **AI-on-the-edge-cam** (`BOARD_ESP32_S3_ALEKSEI`). Has no effect on other board variants.

When enabled:

- Vbatt is read from GPIO2 (ADC1\_CH1) through the on-board 10k/10k divider.
- New MQTT topics `<maintopic>/battery_voltage` and `<maintopic>/battery_percent` are published each round, with Home Assistant autodiscovery.
- A battery indicator appears on the dashboard (Overview) and a "Battery" section appears on the System Info page.
- Deep sleep ([`AutoTimer > SleepWhileIdle`](../AutoTimer/SleepWhileIdle)) additionally drops `ETH_ENABLE` and `PER_ENABLE` low, cutting standby current from ~100-150 mA to ~10-30 µA.

!!! Note
    Leave this **disabled** on PoE- or USB-only devices. The battery percent will read 100% when on USB (Vbatt back-fed through Schottky D7 reads above any real cell voltage) and the deep-sleep current optimisation will kill Ethernet during sleep.

!!! Note
    **USB plugged in does not auto-disable sleep.** This PCB has no VBUS-sense GPIO and TP4057 CHRG/STDBY pins drive only LEDs, so the firmware can't tell whether you're on USB or battery during charging. Use the **"Stay awake" button on the dashboard** to keep the device responsive for OTA updates, or wire CHRG to a spare GPIO (see hardware-mod notes in the SleepWhileIdle docs).
