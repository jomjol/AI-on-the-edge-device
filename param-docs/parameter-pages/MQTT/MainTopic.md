# Parameter `MainTopic`
Default Value: `watermeter`

MQTT main topic, under which the counters are published.

The single value will be published with the following key: `MAINTOPIC/NUMBER/RESULT_TOPIC`

With:

- `NUMBER`: The name of the value (a meter might have more than one value). 
  The names get defined in the analog and digital ROI configuration (defaults to `main`).
- `RESULT_TOPIC`: Automatically filled with the right name, eg. `value`, `rate`, `timestamp`, `error`, ....

The general connection status can be found in `MAINTOPIC/CONNECTION`. 
See [MQTT Result Topics](../MQTT-API#result) for a full list of topics.

!!! Note
    The main topic is allowed to contain `/` which can be used to split it into multiple levels, eg. `/basement/meters/watermeter/1/` if you have multiple water meters in your basement.
