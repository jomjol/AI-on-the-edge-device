# Parameter `SleepWhileIdle`
Default Value: `false`

When enabled, the device enters **deep sleep** between digitization rounds instead of just idling. On wake, the device cold-boots and runs the next round.

After each round, the device waits [`SleepGraceSeconds`](./SleepGraceSeconds) (default 10 s) for a connected user to reach the web UI or toggle the Stay-Awake override before sleeping for the remainder of the interval.

!!! Warning
    Cold boot takes ~10-20 s on this firmware (TF model load + camera init + network bring-up). Sleep intervals shorter than ~3 minutes spend most of the cycle in boot, defeating the power saving.

!!! Note
    On the AI-on-the-edge-cam (`BOARD_ESP32_S3_ALEKSEI`), enabling [`Battery > Enabled`](../Battery) **also** drops `ETH_ENABLE` and `PER_ENABLE` low during deep sleep, killing the W5500 Ethernet PHY (~100 mA) and the peripheral regulators. Without `Battery > Enabled`, deep sleep keeps those rails powered.

!!! Note
    **Updating a sleeping device.** The **Stay-Awake override** pauses deep sleep for OTA updates and config changes. It is persisted in NVS, so it survives the cold-boot wake -- the device stays reachable until the override is turned off again. Three ways to set it:

    1. The **"Stay awake" / "Resume sleep" button** on the dashboard (Overview page).
    2. A **short press of the BOOT button** while the device is sleeping: it wakes immediately with the override enabled. Press briefly and release -- holding BOOT through the wake-up can drop the ESP32 into its serial bootloader until the next reset.
    3. **MQTT**: publish `ON` / `OFF` to `<maintopic>/ctrl/stay_awake` (a "Stay Awake" switch is auto-discovered in Home Assistant). Send the command retained so it survives until the device next wakes; the current state is published on `<maintopic>/stay_awake`.

    To increase the wake window between rounds without disabling sleep entirely, raise [`SleepGraceSeconds`](./SleepGraceSeconds) to 60-120 s.

!!! Note
    **Automatic USB detection is not implemented** because this PCB has no VBUS-sense GPIO. TP4057 `CHRG` and `STDBY` drive only LEDs; Vbatt tracks the cell during charging and only crosses 4.25 V briefly at full float. Use the manual override above.

!!! Note
    **Camera-only power down without deep sleep.** If you want to reduce camera heat without the sleep cycle's boot overhead, use [`TakeImage > Power Down Camera Between Rounds`](../TakeImage/PowerDownCameraBetweenRounds) instead. That option puts just the camera sensor into I2C standby between rounds.
