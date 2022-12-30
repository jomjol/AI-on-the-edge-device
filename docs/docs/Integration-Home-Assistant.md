# Integration into Home Assistant
There are 3 ways to get the data into your Home Assistant:
1. Using MQTT (Automatically Setup Entities using Homeassistant MQTT Discovery)
1. Using MQTT (Manually Setup Entities)
2. Using REST calls

The first one is the easier way if you already have MQTT in use.

## Using MQTT (Automatically Setup Entities using Homeassistant MQTT Discovery)

:bangbang: This feature will be available with the next release!

Starting with Version `>12.0.1`, AI-on-the-edge-devices support Homeassistant Discovery.
 1. Check [here](https://www.home-assistant.io/docs/mqtt/discovery/) to learn more about it and how to enable it in Homeassistant.
 1. You also have to enable it in the MQTT settings of your device:
  
    ![grafik](https://user-images.githubusercontent.com/1783586/199350781-e2a59eeb-b5bb-407b-9c0d-2aafab50daab.png)

    Make sure to select the right Meter Type to get the right units!

On the next start of the device, it will send discovery topics and Homeassistant should pick them up and show them under `Settings > Integrations > MQTT`:

![grafik](https://user-images.githubusercontent.com/1783586/199352538-ddcc3484-39ef-44f4-a853-53286807d30b.png) 
![grafik](https://user-images.githubusercontent.com/1783586/199352565-9b0ade28-cb43-47b4-821f-7909cad41a73.png)
![grafik](https://user-images.githubusercontent.com/1783586/199352619-217df627-4b87-4fa0-86a2-f5347c452fdb.png)


### Using MQTT (Manually Setup Entities)
First make sure with an MQTT client (for example [MQTT Explorer](http://mqtt-explorer.com/)) that MQTT works as expected and to get a list of the available topics!

Then add a sensor for each property:
```yaml
mqtt:
  sensor:
    - state_topic: "wasserzaehler/main/value"
      name: "Watermeter Value"
      unique_id: watermeter_value
      unit_of_measurement: 'm³'
      state_class: total_increasing
      device_class: water # Needs Homeassistant 2022.11!
      icon: 'mdi:water-pump'
      availability_topic: wasserzaehler/connection
      payload_available: connected
      payload_not_available: connection lost

    - state_topic: "wasserzaehler/main/rate"
      name: "Watermeter Rate"
      unique_id: watermeter_rate
      unit_of_measurement: 'm³/min'
      state_class: measurement
      device_class: water # Needs Homeassistant 2022.11!
      icon: 'mdi:water-pump'
      availability_topic: wasserzaehler/connection
      payload_available: connected
      payload_not_available: connection lost

    - state_topic: "wasserzaehler/main/error"
      name: "Watermeter Error"
      unique_id: watermeter_error
      icon: "mdi:water-alert"
      availability_topic: wasserzaehler/connection
      payload_available: connected
      payload_not_available: connection lost    

    - state_topic: "wasserzaehler/uptime"
      name: "Watermeter Uptime"
      unique_id: watermeter_uptime
      unit_of_measurement: 's'
      state_class: measurement
      device_class: duration
      entity_category: diagnostic
      icon: "mdi:timer-outline"
      availability_topic: wasserzaehler/connection
      payload_available: connected
      payload_not_available: connection lost
```
If you run the discovery once, you can also extract the information from there (MQTT Info, untested):
```yaml
mqtt: # Extracted form the Discovery but untested!
  sensor:
      - name: Value
        unique_id: wasserzaehler-main_value
        icon: mdi:gauge
        state_topic: wasserzaehler/main/value
        unit_of_measurement: m³
        device_class: water
        state_class: total_increasing
        availability_topic: wasserzaehler/connection
        payload_available: connected
        payload_not_available: connection lost
```

If you want to convert the `m³` to `l`, use a template sensor:
```yaml
template:
  - sensor:
    - name: "Watermeter in l"
      unique_id: watermeter_in_l
      icon: "mdi:gauge"
      state: "{{ states('sensor.watermeter_value')|float(default=0) * 1000 }}" # Convert 1 m3 => 1000 l
      unit_of_measurement: l
      availability: "{{ states('sensor.watermeter_value') not in ['unknown', 'unavailable', 'none'] }}"
```

If you you want to have the consumption per day, you can use an [Utility Meter](https://www.home-assistant.io/integrations/utility_meter/).
it is a helper and can be used to reset the total increasing values once a day

```yaml
utility_meter:
  utility_meter_gas_per_day:
    source: sensor.gasmeter_value
    cycle: daily

  utility_meter_water_per_day:
    source: sensor.watermeter_value
    cycle: daily
```

Note that you also can add it using the UI.

### Examples
![grafik](https://user-images.githubusercontent.com/1783586/193472069-4135736e-e63a-4afb-8009-5b97aa5c9ac5.png)

![grafik](https://user-images.githubusercontent.com/1783586/193472091-1484aac4-ddc2-48ba-896c-28370963fc2d.png)

### Statistics Graph
Creating Statistics Graphs (eg. usage per day) is easy using the [Energy Dashboard](https://www.home-assistant.io/home-energy-management/):
![grafik](https://user-images.githubusercontent.com/1783586/193471893-d8ab8f5f-0906-4076-8926-8b5a69a24bce.png)

Note that there seems to be a bug in the graph, see https://github.com/home-assistant/frontend/issues/13995!


### InfluxDb Graphs
If you have setup InfluxDB already, it is also possible to fetch statistics from there, eg. daily usage:
```
from(bucket: "HomeAssistant")
|> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r["entity_id"] == "wasserverbrauch_tag")
  |> filter(fn: (r) => r["_field"] == "value")
  |> timeShift(duration: -1d)
  |> aggregateWindow(every: 1d, fn: max, createEmpty: false)
  |> yield(name: "mean")
```

![grafik](https://user-images.githubusercontent.com/1783586/193473347-c81fc301-c52f-4af0-9fcb-56fab12cacac.png)


## Using REST
When using REST, Home Assistant has to periodically call an URL on the ESP32 which in return provides the requested data.

See [REST API](https://github.com/jomjol/AI-on-the-edge-device/wiki/REST-API) for a list of available URLs.

The most practical one is the `json` entrypoint which provides the most relevant data JSON formated:
`http://<IP>/json`
This would return:
```JSON
{
"main":
  {
    "value": "512.3020",
    "raw": "0512.3020",
    "error": "no error",
    "rate": 0.000000,
    "timestamp": "2022-10-02T20:32:06"
   [..]
  }
}
```

To do such a REST call, you need to create a REST sensor:
```yaml
sensor:
- platform: rest
  name: "Gasmeter JSON" 
  resource: http://<IP>/json
  json_attributes:
    - main
  value_template: '{{ value_json.value }}'
  headers:
    Content-Type: application/json
  scan_interval: 60

template:
  sensor:
  - name: "Gasmeter Value from JSON"
    unique_id: gas_meter_value_from_json
    state: "{{ state_attr('sensor.gasmeter_json','main')['value'] }}"
    unit_of_measurement: 'm³'

  - name: "Watermeter Value from JSON"
    unique_id: water_meter_value_from_json
     state: >-
            {{ state_attr('sensor.watermeter_json','main')['value'] | float }}
     unit_of_measurement: 'm³'
     device_class: water
     state_class: total_increasing
     icon: mdi:gauge

```
See also https://community.home-assistant.io/t/rest-sensor-nested-json/243420/9


#### Photo
REST can also be used to show the photo of the last round:

![grafik](https://user-images.githubusercontent.com/1783586/193546075-b247942f-9106-47a4-a64b-42ff96dd9078.png)

To access it, use `http://<IP>/img_tmp/alg_roi.jpg` resp `http://<IP>/img_tmp/raw.jpg`.
