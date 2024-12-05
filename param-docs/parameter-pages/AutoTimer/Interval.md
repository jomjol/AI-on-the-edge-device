# Parameter `Interval`
Default Value: `5`

Unit: Minutes

Interval in which the Flow (Digitization Round) is run.
It will run immediately on startup and then the next time after the given interval.
If a round takes longer than this interval, the next round gets postponed until the current round completes.

If the flow gets started by a MQTT message or the REST API call, the interval automatically gets reset.

!!! Note
If you want the flow to be disabled, set an interval which is high enough (eg. 1440 = 24h).
