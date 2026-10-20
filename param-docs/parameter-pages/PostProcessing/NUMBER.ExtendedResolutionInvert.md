# Parameter `ExtendedResolutionInvert`
Default Value: `false`

Inverts the extra fractional digit that is added by [`NUMBER.ExtendedResolution`](NUMBER.ExtendedResolution).
Use this when your wheel transition direction is opposite and the intermediate values count in the wrong direction.

Mapping:
- `0 -> 0`
- `1 <-> 9`
- `2 <-> 8`
- `3 <-> 7`
- `4 <-> 6`
- `5 -> 5`

!!! Note
    This parameter only affects the derived fractional digit that is added by `ExtendedResolution`.

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.ExtendedResolutionInvert`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
