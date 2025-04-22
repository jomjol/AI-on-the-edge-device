# Parameter `MaxRateType`
Default Value: `AbsoluteChange`

Defines if the **Change Rate** is calculated as the difference between the last two readings (`AbsoluteChange` = difference) or
as the difference normalized to the interval (`RateChange` = difference per minute).

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.MaxRateType`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
