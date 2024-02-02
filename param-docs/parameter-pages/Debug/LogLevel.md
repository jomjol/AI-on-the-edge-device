# Parameter `LogLevel`
Default Value: `1` (`ERROR`)
Define the log level for the logging to the SD-Card.

Available options:

- `1`: `ERROR`
- `2`: `WARNING`
- `3`: `INFO`
- `4`: `DEBUG`

As higher the level, as more log messages get written to the SD-Card.

!!! Warning
    `DEBUG` or `INFO` might damage the SD-Card if enabled long term due to excessive writes to the SD-Card!
    A SD-Card has limited write cycles. Since the device does not do [Wear Leveling](https://en.wikipedia.org/wiki/Wear_leveling), this can wear out your SD-Card!
