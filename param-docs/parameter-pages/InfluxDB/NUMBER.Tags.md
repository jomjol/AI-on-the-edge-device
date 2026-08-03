# Parameter `Tags`
Default Value: `undefined`

Optional set of InfluxDB v1 tags to attach to the measurement, as comma-separated `key=value` pairs (e.g.: `host=gasmeter,source=ai-on-the-edge,type=counter`).

!!! Note
    See section `InfluxDBv2` for InfluxDB v2 support!

!!! Note
    If you edit the config file manually, you must prefix this parameter with `<NUMBER>` followed by a dot (eg. `main.Tags`). The reason is that this parameter is specific for each `<NUMBER>` (`<NUMBER>` is the name of the number sequence defined in the ROI's).
