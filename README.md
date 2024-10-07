# Welcome to the AI-on-the-edge-device
<img src="images/icon/watermeter.svg" width="100px">

Artificial intelligence-based systems have become established in our everyday lives. Just think of speech or image recognition. Most of the systems rely on either powerful processors or a direct connection to the cloud for doing the calculations there. With the increasing power of modern processors, the AI systems are coming closer to the end user, a concept known as **edge computing**.
In this project, edge computing is demonstrated through a practical example, where an AI network is implemented on an ESP32 device, hence: **AI on the edge**.

This project allows you to digitize your **analog** water, gas, power and other meters using cheap and readily available hardware.

All you need is an [ESP32 board with a supported camera](https://jomjol.github.io/AI-on-the-edge-device-docs/Hardware-Compatibility/) and some practical skills.

<img src="images/esp32-cam.png" width="200px">

## Key features
- Tensorflow Lite (TFLite) integration – including easy-to-use wrapper
- Inline image processing (feature detection, alignment, ROI extraction)
- **Small** and **cheap** device (3 x 4.5 x 2 cm³, < 10 EUR)
- Integrated camera and illumination
- Web interface for administration and control
- OTA interface for updating directly via the web interface
- Full integration with Home Assistant
- Support for Influx DB 1 and 2
- MQTT
- REST API

## Workflow
The device captures a photo of your meter at set intervals. It then extracts the Regions of Interest (ROIs) from the image and runs them through artificial intelligence. As a result, you get the digitized value of your meter.

There are several options for what to do with that value. Either send it to an MQTT broker, write it to an InfluxDb or simply provide access to it via a REST API.

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/idea.jpg" width="600"> 

## Impressions
### AI-on-the-edge-device on a Water Meter
<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter_all.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/main.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/size.png" width="200"> 

### Web Interface (Water Meter)
<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter.jpg" width="600"> 

### AI-on-the-edge-device on a Electrical Power Meter
<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/powermeter.jpg" width="600"> 


## Setup
There is growing [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/) which provides you with a lot of information. Head there to get a start, set it up and configure it.

There are also articles in the German Heise magazine "make:" about the setup and technical background (behind a paywall): [DIY - Setup](https://www.heise.de/select/make/2021/2/2103513300897420296)

A lot of people created useful YouTube videos which might help you getting started.
Here a small selection:

- [youtube.com/watch?v=HKBofb1cnNc](https://www.youtube.com/watch?v=HKBofb1cnNc)
- [youtube.com/watch?v=yyf0ORNLCk4](https://www.youtube.com/watch?v=yyf0ORNLCk4)
- [youtube.com/watch?v=XxmTubGek6M](https://www.youtube.com/watch?v=XxmTubGek6M)
- [youtube.com/watch?v=mDIJEyElkAU](https://www.youtube.com/watch?v=mDIJEyElkAU)
- [youtube.com/watch?v=SssiPkyKVVs](https://www.youtube.com/watch?v=SssiPkyKVVs)
- [youtube.com/watch?v=MAHE_QyHZFQ](https://www.youtube.com/watch?v=MAHE_QyHZFQ)
- [youtube.com/watch?v=Uap_6bwtILQ](https://www.youtube.com/watch?v=Uap_6bwtILQ)

For further background information, head to [Neural Networks](https://www.heise.de/select/make/2021/6/2126410443385102621), [Training Neural Networks](https://www.heise.de/select/make/2022/1/2134114065999161585) and [Programming on the ESP32](https://www.heise.de/select/make/2022/2/2204010051597422030).

### Download
The latest available version can be found on the [Releases page](https://github.com/jomjol/AI-on-the-edge-device/releases).

### Flashing the ESP32
Initially you will have to flash the ESP32 via a USB connection. Later updates are possible directly over the air (OTA using Wi-Fi).

There are different ways to flash your ESP32:
- The preferred way is the [Web Installer and Console](https://jomjol.github.io/AI-on-the-edge-device/index.html) which is a browser-based tool to flash the ESP32 and extract the log over USB:
![](images/web-installer.png)
- Flash Tool from Espressif
- ESPtool (command-line tool)

See the [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/) for more information.

### Flashing the SD Card
The SD card can be setup automatically after the firmware got installed. See the [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#remote-setup-using-the-built-in-access-point) for details. For this to work, the SD card must be FAT formated (which is the default on a new SD card).
Alternatively, the SD card still can be set up manually, see the [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#3-sd-card) for details!

## Casing
Various 3D-printable housing can be found here:
  - https://www.thingiverse.com/thing:4573481 (Water Meter)
  - https://www.thingiverse.com/thing:5028229 (Power Meter)
  - https://www.thingiverse.com/thing:5224101 (Gas Meter)
  - https://www.thingiverse.com/thing:4571627 (ESP32-cam housing only)

## Donate
If you would like to support the developer with a cup of coffee, you can do that via [PayPal](https://www.paypal.com/donate?hosted_button_id=8TRSVYNYKDSWL).

<a href="https://www.paypal.com/donate?hosted_button_id=8TRSVYNYKDSWL"><img border="0" src="images/paypal.png" width="200px" target="_blank"></a>

## Support
If you have any technical problems please search the [discussions](https://github.com/jomjol/AI-on-the-edge-device/discussions). In case you find a bug or have a feature request, please open an [issue](https://github.com/jomjol/AI-on-the-edge-device/issues).

In other cases you can contact the developer via email: <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/mail.jpg" height="25">

## Changes and History 
See [Changelog](Changelog.md).

## Build It Yourself
See [Build Instructions](code/README.md).

## Tools
* Logfile downloader and combiner (Thx to [reserve85](https://github.com/reserve85))
  * Files see ['/tools/logfile-tool'](tbd), how-to see [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/outdated--Gasmeter-Log-Downloader/)

## Additional Ideas
There are some ideas and feature requests which are not currently being pursued – mainly due to capacity reasons on the part of the developers.
They features are collected in the [issues](https://github.com/jomjol/AI-on-the-edge-device/issues) and in [FeatureRequest.md](FeatureRequest.md).

## Our Contributors ❤️
<!-- Autogenerated table, do not manually update!
     It gets automatically updated on the next release.
     See https://github.com/marketplace/actions/contribute-list -->
<!-- readme: contributors -start -->
<!-- readme: contributors -end -->
