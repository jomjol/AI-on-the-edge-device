# Parameter `<NUMBER>.PreValueHysterese`
Default Value: `2`

Threshold parameter for negative rate detection / Hysteresis function.
This parameter is intended to compensate for small reading fluctuations that occur when the counter does not change its value for a long time (e.g. at night).
It is only applied to the last position of the read value.

Example:

    PreValue: 123.4567, PreValueHysterese: 2 >>> Threshold: +/- 0.0002
    PreValue: 123.45678, PreValueHysterese: 5 >>> Threshold: +/- 0.00005

!!! Note
    Further explanation will be added later.

Range: `1` .. `9`.

![](img/PreValueHysterese.png)
