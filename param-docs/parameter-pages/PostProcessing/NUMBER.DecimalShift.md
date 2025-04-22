# Parameter `DecimalShift`
Default Value: `0`

Shift the decimal separator (positiv or negativ).
Eg. to move from `m³` to `liter` (`1 m³` equals `1000 liters`), you need to set it to `+3`.

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.DecimalShift`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
