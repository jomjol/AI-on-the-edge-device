# Parameter `PowerDownCameraBetweenRounds`
Default Value: `false`

When enabled, the camera sensor is put into **low-power standby** via its I2C sleep register at the end of each digitization round and woken at the start of the next. Useful for OV5640 reports of running-hot during long idle periods between captures.

Effect on sensor current:

- **OV5640 / OV3660:** ~200 mA → ~20 µA in standby (register 0x3008).
- **OV2640:** active → standby via register 0x09 bit 4.

!!! Note
    Wake adds ~100 ms before the next picture. If your interval is in minutes this is negligible; if your interval is in seconds (high-frequency capture) the overhead may matter.

!!! Note
    **Already implied by `SleepWhileIdle`.** When [`AutoTimer > SleepWhileIdle`](../AutoTimer/SleepWhileIdle) is on, the cold-boot wake re-inits the camera from scratch each round, so the camera is effectively powered down between rounds anyway. Enable **this** setting when you want the thermal benefit without the cold-boot overhead of full deep sleep -- e.g. on a USB- or PoE-powered device where idle current isn't a concern but camera heat is.

!!! Note
    This is a sensor-level standby (I2C register write), not a power-rail cut. It works on every board variant -- including non-battery boards that don't have a way to gate camera power independently.
