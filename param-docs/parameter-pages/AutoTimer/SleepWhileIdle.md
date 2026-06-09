# Parameter `SleepWhileIdle`
Default Value: `false`

When enabled, the device enters **deep sleep** between digitization rounds instead of just idling. On wake, the device cold-boots and runs the next round.

After each round, the device waits 10 seconds (so a connected user can still reach the web UI to reconfigure) and then sleeps for the remainder of the interval.

!!! Warning
    Cold boot takes ~10-20 s on this firmware (TF model load + camera init + network bring-up). Sleep intervals shorter than ~3 minutes spend most of the cycle in boot, defeating the power saving.

!!! Note
    On the AI-on-the-edge-cam (`BOARD_ESP32_S3_ALEKSEI`), enabling [`Battery > Enabled`](../Battery) **also** drops `ETH_ENABLE` and `PER_ENABLE` low during deep sleep, killing the W5500 Ethernet PHY (~100 mA) and the peripheral regulators. Without `Battery > Enabled`, deep sleep keeps those rails powered.

!!! Note
    **USB power skips sleep.** When `Battery > Enabled` is also on, the device samples `Vbatt` before each sleep. If it reads above 4.25 V the Schottky D7 is back-feeding the battery rail from USB +5V, i.e. the device is plugged in -- in that case sleep is skipped and the device idles normally. Makes OTA updates and config tweaks practical without unplugging the battery. Requires `Battery > Enabled` because that's the only way to detect USB presence on this hardware.

!!! Note
    **Camera-only power down without deep sleep.** If you want to reduce camera heat without the sleep cycle's boot overhead, use [`TakeImage > Power Down Camera Between Rounds`](../TakeImage/PowerDownCameraBetweenRounds) instead. That option puts just the camera sensor into I2C standby between rounds.
