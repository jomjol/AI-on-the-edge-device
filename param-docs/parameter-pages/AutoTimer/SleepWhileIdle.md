# Parameter `SleepWhileIdle`
Default Value: `false`

When enabled, the device enters **deep sleep** between digitization rounds instead of just idling. On wake, the device cold-boots and runs the next round.

After each round, the device waits 10 seconds (so a connected user can still reach the web UI to reconfigure) and then sleeps for the remainder of the interval.

!!! Warning
    Cold boot takes ~10-20 s on this firmware (TF model load + camera init + network bring-up). Sleep intervals shorter than ~3 minutes spend most of the cycle in boot, defeating the power saving.

!!! Note
    On the AI-on-the-edge-cam (`BOARD_ESP32_S3_ALEKSEI`), enabling [`Battery > Enabled`](../Battery) **also** drops `ETH_ENABLE` and `PER_ENABLE` low during deep sleep, killing the W5500 Ethernet PHY (~100 mA) and the peripheral regulators. Without `Battery > Enabled`, deep sleep keeps those rails powered.
