# Parameter `MeterType`
Default Value: `other`

Select the Meter Type so the sensors have the right units in Homeassistant.

!!! Note
    For `Watermeter` you need to have Homeassistant 2022.11 or newer!

Please also make sure that the selected Meter Type matches the dimension of the value provided by the meter!
Eg. if your meter provides `m³`, you need to also set it to `m³`.
Alternatively you can set the parameter `DecimalShift` to `3` so the value is converted to `liters`!
