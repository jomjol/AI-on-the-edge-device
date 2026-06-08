# Parameter `Enabled`
Default Value: `false`

Activates battery voltage monitoring on the **AI-on-the-edge-cam** (`BOARD_ESP32_S3_ALEKSEI`). Has no effect on other board variants.

When enabled:

- Vbatt is read from GPIO2 (ADC1\_CH1) through the on-board 10k/10k divider.
- New MQTT topics `<maintopic>/battery_voltage` and `<maintopic>/battery_percent` are published each round, with Home Assistant autodiscovery.
- A battery indicator appears on the dashboard (Overview) and a "Battery" section appears on the System Info page.
- Deep sleep (`AutoTimer > SleepWhileIdle`) additionally drops `ETH_ENABLE` and `PER_ENABLE` low, cutting standby current from ~100-150 mA to ~10-30 µA.

!!! Note
    Leave this **disabled** when running on PoE or USB power. There's no functional downside to enabling on a battery-powered unit; on a non-battery unit, the battery percent will read 100% (USB back-feeds the Vbatt rail through Schottky D7) and the deep-sleep current optimisation will kill Ethernet during sleep.
