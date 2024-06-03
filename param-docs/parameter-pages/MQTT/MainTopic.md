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

The nodeId for the Home Assistant MQTT Service Discovery must follow the schema `<discovery_prefix>/<component>/[<node_id>/]<object_id>/config`. The node_id is not configurable but derived from the `MainTopic` by stripping any but the last topic level. A `MainTopic` with the value `home/basement/watermeter` is transformed into the node_id `watermeter`, resulting in the discovery topic `homeassistant/sensor/watermeter/value/config` for the current value.
