# Parameter `ContinuousLastDigit`
Default Value: `false`

Enable this if the last digit (the one directly before the analog pointers) does not jump, but turns
continuously together with the first analog pointer: the digit is about halfway to the next number
when the pointer is at 5, and already shows the next number shortly before the pointer reaches 0.

With such a digit the normal transition logic gives a value that is one unit too high while the
pointer is between about 9.5 and 0 (usually rejected as `Rate too high`). With this option the digit
is calculated as `digit - pointer / 10`, rounded, which needs no tuning.
[`AnalogToDigitTransitionStart`](NUMBER.AnalogToDigitTransitionStart) is not used when it is enabled.

!!! Warning
    Only enable it for digits that really turn continuously. For a digit that jumps to the next
    number (even an early or late one) it gives wrong results.

Only used on combination of digits and analog pointers, and needs a digit model with decimal output
(e.g. `dig-cont`).

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.ContinuousLastDigit`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
