# Parameter `MaxRateType`
Default Value: `AbsoluteChange`

Defines if the **Change Rate** is calculated as the difference between the last two readings (`AbsoluteChange` = difference) or
as the difference normalized to the interval (`RateChange` = difference per minute).

!!! Note
    This parameter must be prefixed with `<NUMBER>` followed by a dot (eg. `main.MaxRateType`). `<NUMBER>` is the name of the number sequence  defined in the ROI's.
