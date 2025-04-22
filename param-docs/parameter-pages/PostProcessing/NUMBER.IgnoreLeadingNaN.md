# Parameter `IgnoreLeadingNaN`
Default Value: `true`

Leading `N`'s will be deleted before further processing.
This is only relevant for models which use `N`!
See [here](../Choosing-the-Model) for details.

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.IgnoreLeadingNaN`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
