Various information is directly accessible over specific REST calls.

For an up-to-date list search the Github repository for [registered handlers](https://github.com/jomjol/AI-on-the-edge-device/search?q=camuri.uri)

# Often used APIs
Just append them to the IP, separated with a `/`, eg. `http://192.168.1.1/json`

## Control
* ### flow_start

* ### gpio
  The `gpio` entrypoint also support parameters:
   - `/GPIO?GPIO=12&Status=high`

* ### ota

* ### ota_page.html

* ### reboot

## Results
* ### json

* ### value
  The `value` entrypoint also support parameters:
   - `http://<IP>/value?all=true&type=value`
   - `http://<IP>/value?all=true&type=raw`
   - `http://<IP>/value?all=true&type=error`
   - `http://<IP>/value?all=true&type=prevalue`

* ### img_tmp/alg_roi.jpg
  Last captured picture

## Status
* ### statusflow

* ### rssi

* ### cpu_temperature

* ### sysinfo

* ### starttime

* ### uptime

## Camera
* ### lighton

* ### lightoff

* ### capture

* ### capture_with_flashlight

* ### save
  The `save` entrypoint also support parameters:
   - `http://<IP>/save?filename=test.jpg&delay=3`

## Logs
* ### log 
  Last part of todays log

* ### logfileact 
  Full log of today

* ### log.html
