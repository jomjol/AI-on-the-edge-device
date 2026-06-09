# Parameter `SleepGraceSeconds`
Default Value: `10`

Range: `0` – `600` seconds

Length of the **grace window** between the end of a digitization round and the start of deep sleep. Only effective when [`SleepWhileIdle`](./SleepWhileIdle) is `true`.

During the grace window the device is fully awake and reachable via web UI / MQTT / OTA. Use this to push an OTA update or toggle the Stay-Awake override (dashboard button) without having to time it to the start of a fresh round.

!!! Note
    Set to a longer value (e.g. **60-120**) on a battery-powered device that you occasionally need to update over the air. The longer the window, the more battery you spend on standby per round.

!!! Note
    Set to **0** if you want sleep to start immediately at end-of-round and have already pre-set the Stay-Awake override via the dashboard. Saves ~10 mA·s per round.

!!! Note
    The grace window is also when the Battery_ReadVoltage() sample for MQTT publish happens, so very small values may shorten the time the regulators have had to settle after the round. Keep at least a couple of seconds unless you have a reason.
