# Parameter `MeterType`
Default Value: `other`

Select the Meter Type so the sensors have the right units in Homeassistant.

Please also make sure that the selected Meter Type matches the dimension of the value provided by the meter!
Eg. if your meter provides `m³`, you need to also set it to `m³`.
Alternatively you can set the parameter `DecimalShift` to `3` so the value is converted to `liters`!

List of supported options:

- `other`
- `water_m3` (uses `m^3/min` as rate)
- `water_l`  (uses `l/h` as rate, not officially supported by Homeassistant!)
- `water_gal`  (uses `gal/h` as rate, not officially supported by Homeassistant!)
- `water_ft3` (uses `ft^3/min` as rate)
- `gas_m3` (uses `m^3/min` as rate)
- `gas_ft3` (uses `ft^3/min` as rate)
- `energy_wh` (uses `W` as rate)
- `energy_kwh` (uses `KW` as rate)
- `energy_mwh` (uses `MW` as rate)
- `energy_gj`  (uses `GJ/h` as rate, not officially supported by Homeassistant!)
- `temperature_c` (uses `+C/min` as rate)
- `temperature_f` (uses `°F/min` as rate)
- `temperature_k` (uses `K/min` as rate)

!!! Note
    Not all options are supported by Homeassistant, see `SensorDeviceClass.VOLUME_FLOW_RATE` in [https://developers.home-assistant.io/docs/core/entity/sensor/#available-device-classes](https://developers.home-assistant.io/docs/core/entity/sensor/#available-device-classes)!
