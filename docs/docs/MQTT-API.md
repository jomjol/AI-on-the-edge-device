# General Information
The device is capable to register to a MQTT broker to publish data and subscribe to specific topics.

The MQTT service has to be enabled and configured properly in the device configuration via web interface (`Settings` -> `Configuration` -> section `MQTT`)

The following parameters have to be defined:
* URI
* MainTopic (optional, if not set, the hostname is used)
* ClientID (optional, if not set, `AIOTED-` + the MAC address gets used to make sure the ID is unique)
* User (optional)
* Password (optional)
* RetainFlag (optional)

# Published topics

## Status
`MainTopic`/{status topic}, e.g. `watermeter/status`
* ### Connection

* ### Interval

* ### MAC

* ### IP

* ### Hostname

* ### Uptime

* ### FreeMem

* ### WifiRSSI

* ### CPUTemp

* ### Status

## Result
`MainTopic`/{NumberName}/{result topic}, e.g. `watermeter/main/value`

* ### Value

* ### Raw

* ### Error

* ### JSON

* ### Rate

* ### Rate_per_time_unit
  The time Unit gets set with the Homeassistant Discovery, eg. `h` or `m` (minutes)

* ### Rate_per_digitalization_round
  The `interval` defines when the next round gets triggered

* ### Changeabsolut

* ### Timestamp

* ### JSON
  All relevant results in JSON syntax

## GPIO
`MainTopic`/{GPIO topic}, e.g. `watermeter/GPIO/GPIO12`

* ### GPIO/GPIO{PinNumber}
  Depending on device configuration (`Settings` --> `Configuration` --> Chapter `GPIO`)


# Subscibed topics
`MainTopic`/{subscribed topic}, e.g. `watermeter/ctrl/flow_start`

## Control

* ### Ctrl/flow_start
  Trigger a flow start by publishing to this topic (any character, length > 0)

* ### GPIO/GPIO{PinNumber}
  Depending on device configuration (`Settings` --> `Configuration` --> Chapter `GPIO`)