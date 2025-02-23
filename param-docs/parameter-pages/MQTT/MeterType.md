# Parameter `MeterType`
Default Value: `other`

Select the Meter Type so the sensors have the right units in Homeassistant.

Please also make sure that the selected Meter Type matches the dimension of the value provided by the meter!
Eg. if your meter provides `m³`, you need to also set it to `m³`.
Alternatively you can set the parameter `DecimalShift` to `3` so the value is converted to `liters`!

List of supported options:
- `other`
- `water_m3`
- `water_l` (Not officially supported by Homeassistant!)
- `water_gal` (Not officially supported by Homeassistant!)
- `water_ft3`
- `gas_m3`
- `gas_ft3`
- `energy_wh`
- `energy_kwh`
- `energy_mwh`
- `energy_gj` (Not officially supported by Homeassistant!)
- `temperature_c`
- `temperature_f`
- `temperature_k`

!!! Note
    Not all options are officially supported by Homeassistant!
