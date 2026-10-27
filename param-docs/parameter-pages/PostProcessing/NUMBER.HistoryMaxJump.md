# Parameter `HistoryMaxJump`
Default Value: `auto` (≈ 100 × the most-significant analog dial step)

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Upper bound (in the meter's value units, after `DecimalShift`) on how much the reading may change
in a single cycle when `HistoryReconcile` is enabled. A change larger than this is treated as a
gross misread (e.g. a whole leading digit recognised wrongly during a multi-dial cascade rollover)
and is rejected; the previous value is held.

If left unset, it is **derived automatically** from the dial scale (about 100 steps of the
most-significant analog dial), so it adapts to your meter's unit and `DecimalShift`. Set an explicit
value only to override: above the largest plausible real change between two cycles (consumption per
cycle plus head-room for a few missed cycles), and well below the magnitude of a leading-digit misread.

Only relevant when `HistoryReconcile` is `true`.

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.HistoryMaxJump`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
