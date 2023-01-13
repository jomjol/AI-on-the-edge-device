# Welcome to the AI-on-the-edge-device
<img src="images/icon/watermeter.svg" width="100px">

Artificial intelligence based systems have been established in our every days live. Just think of speech or image recognition. Most of the systems relay on either powerful processors or a direct connection to the cloud for doing the calculations up there. With the increasing power of modern processors the AI systems are coming closer to the end user - which is usually called **edge computing**.
Here this edge computing is brought into a practical oriented example, where a AI network is implemented on a ESP32 device so: **AI on the edge**.

This projects allows you to digitalize your **analoge** water, gas, power and other meters using cheap and easily available hardware.

All you need is an [ESP32 board with a supported camera](https://jomjol.github.io/AI-on-the-edge-device-docs/Hardware-Compatibility/) and a bit of a practical hand.

<img src="images/esp32-cam.png" width="200px">

## Key features
- Tensorflow Lite (TFlite) integration - including easy to use wrapper
- Inline Image processing (feature detection, alignment, ROI extraction)
- **Small** and **cheap** device (3x4.5x2 cm³, < 10 EUR)
- camera and illumination integrated
- Web surface to administrate and control
- OTA-Interface to update directly through the web interface
- Full integration into Homeassistant
- Support for Influx DB 1
- MQTT
- REST API

## Workflow
The device takes a photo of your meter at a defined interval. It then extracts the Regions of Interest (ROI's) out of it and runs them through an artificial inteligence. As a result, you get the digitalized value of your meter.

There are several options what to do with that value. Either send it to a MQTT broker, write it to an InfluxDb or simply provide it throug a REST API.

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/idea.jpg" width="600"> 

## Impressions
### AI-on-the-edge-device on a Water Meter
<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter_all.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/main.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/size.png" width="200"> 

### Web Interface (Water Meter)
<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter.jpg" width="600"> 

### AI-on-the-edge-device on a Electrical Power Meter
<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/powermeter.jpg" width="600"> 


## Setup
There is a growing [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/) which provides you with a lot of information.
Head there to get a start, set it up and configure it.

There are also a articles in the German Heise magazine "make:" about the setup and the technical background (behind a paywall) : [DIY - Setup](https://www.heise.de/select/make/2021/2/2103513300897420296)

For further background information, head to [Neural Networks](https://www.heise.de/select/make/2021/6/2126410443385102621), [Training Neural Networks](https://www.heise.de/select/make/2022/1/2134114065999161585) and [Programming on the ESP32](https://www.heise.de/select/make/2022/2/2204010051597422030) 

### Download
The latest available version is available on the [Releases page](https://github.com/jomjol/AI-on-the-edge-device/releases).

### Flashing of the ESP32
Initially you will have to flash the ESP32 through an USB connection. Later an update is possible directly over the Air (OTA).

There are different ways to flash your ESP32:
- [Web Installer and Console](https://jomjol.github.io/AI-on-the-edge-device/index.html) (Webbrowser based tool to flash the ESP32 and extract the Log over USB)
- Flash Tool from Espressif
- ESPtool (Command Line Tool)

See the [Docu](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/) for more information.

### Flashing the SD-Card
The SD-Card must be flashed separately, see the [Docu](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/) for details.

## Casing

A 3d-printable housing can be found here:
  - https://www.thingiverse.com/thing:4573481 (Water Meter)
  - https://www.thingiverse.com/thing:5028229 (Power Meter)
  - https://www.thingiverse.com/thing:5224101 (Gas Meter)
  - https://www.thingiverse.com/thing:4571627 (ESP32-Cam housing only)

## Build it yourself
See [Build Instructions](code/README.md).

## Donate
If you would like to support the developer with a cup of coffee you can do that via [Paypal](https://www.paypal.com/donate?hosted_button_id=8TRSVYNYKDSWL).

<form action="https://www.paypal.com/donate" method="post" target="_top">
<input type="hidden" name="hosted_button_id" value="8TRSVYNYKDSWL" />
<input type="image" src="https://www.paypalobjects.com/en_US/DK/i/btn/btn_donateCC_LG.gif" border="0" name="submit" title="PayPal - The safer, easier way to pay online!" alt="Donate with PayPal button" />
<img alt="" border="0" src="https://www.paypal.com/en_DE/i/scr/pixel.gif" width="1" height="1" />
</form>
If you have any technical topics, you can create an [Issue](https://github.com/jomjol/AI-on-the-edge-device/issues). 

In other cases you can contact the developer via email: <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/mail.jpg" height="25"> 

## Changes and History 
See [Changelog](Changelog.md)

## Tools

* Logfile downloader and combiner (Thx to [reserve85](https://github.com/reserve85))
  * Files see ['/tools/logfile-tool'](tbd), How-to see [Docu](https://jomjol.github.io/AI-on-the-edge-device-docs/outdated--Gasmeter-Log-Downloader/)

## Additional Ideas
There are some ideas and feature requests which are not followed currently - mainly due to capacity reasons on side of the developer. They are collected here: [FeatureRequest.md](FeatureRequest.md)

------

