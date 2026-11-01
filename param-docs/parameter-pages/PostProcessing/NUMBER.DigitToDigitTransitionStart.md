# Parameter `DigitToDigitTransitionStart`
Default Value: `9.7`

Only used between two digit ROIs (digit models `dig-cont` / `dig-class100`), not for the analog pointer to digit transition
(see [`AnalogToDigitTransitionStart`](../NUMBER.AnalogToDigitTransitionStart)).

While the lower digit has not yet crossed zero (e.g. it shows `9.8`) and the current digit already shows `x.0` .. `x.3`,
the firmware assumes the current digit runs ahead of the lower one and reduces it by 1 (`3.2` → `2`).
This parameter sets from which value of the lower digit that reduction starts.

On meters where the higher digit starts to turn early and slowly together with the lower digit (e.g. the lower digit at `9.8`
while the higher digit only shows `3.3`), the reduction is wrong: the reading drops by one step of the higher digit
(e.g. `429` instead of `439`) and is rejected as `Neg. Rate` until the lower digit has crossed zero.
Check the logged digit images (`ROIImagesLocation`) during such a transition. If the recognition is correct but the
value is too low, increase this parameter. `10.0` disables the reduction.

Range: `9.0` .. `10.0`.

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.DigitToDigitTransitionStart`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
