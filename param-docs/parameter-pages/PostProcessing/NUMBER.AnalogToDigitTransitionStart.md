# Parameter `AnalogToDigitTransitionStart`
Default Value: `9.2`

This can be used if you have wrong values, but the recognition of the individual ROIs are correct.
Look for the start of changing of the first digit and note the analog pointer value behind.
Set it here. Only used on combination of digits and analog pointers.
See [here](../Watermeter-specific-analog---digit-transition) for details.

Range: `6.0` .. `9.9`.

!!! Note
    This parameter must be prefixed with `<NUMBER>` followed by a dot (eg. `main.AnalogToDigitTransitionStart`). `<NUMBER>` is the name of the number sequence  defined in the ROI's.
