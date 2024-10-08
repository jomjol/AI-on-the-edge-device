# AI on the Edge Device: Digitizing Your non-digital meters with an ESP32-CAM
<p align="center">
  <img src="images/icon/watermeter.svg" width="150px">
</p>

Artificial intelligence is everywhere, from speech to image recognition. While most AI systems rely on powerful processors or cloud computing, **edge computing** brings AI closer to the end user by utilizing the capabilities of modern processors.  
This project demonstrates edge computing using the **ESP32**, a low-cost, AI-capable device, to digitize your analog meters—whether water, gas, or electricity. With affordable hardware and simple instructions, you can turn any standard meter into a smart device.

Let's explore how to make **AI on the Edge** a reality! 🌟

All you need is an [ESP32 board with a supported camera](https://jomjol.github.io/AI-on-the-edge-device-docs/Hardware-Compatibility/) and some practical skills. 🛠️

---

## Key Features 🚀
- 🔗 **Tensorflow Lite (TFLite) integration** – including an easy-to-use wrapper.
- 📸 **Inline image processing** (feature detection, alignment, ROI extraction).
- 💡 **Small** and **affordable** device (3 x 4.5 x 2 cm³, less than 10 EUR).
- 📷 Integrated camera and illumination.
- 🌐 Web interface for administration and control.
- 🔄 OTA interface for updating directly via the web interface.
- 🏠 Full integration with Home Assistant.
- 📊 Support for **Influx DB 1** and **2**.
- 📡 **MQTT protocol** support.
- 📥 **REST API** available for data access.

## Workflow 🔧
The device captures a photo of your meter at set intervals. It then extracts the Regions of Interest (ROIs) from the image and runs them through artificial intelligence. As a result, you get the digitized value of your meter.

There are several options for what to do with that value:
- 📤 Send it to a **MQTT broker**.
- 📝 Write it to an **InfluxDb**.
- 🔗 Provide access via a **REST API**.

<p align="center">
  <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/idea.jpg" width="600"> 
</p>

---

## Impressions 📷

### AI-on-the-edge-device on a Water Meter 💧
<p align="center">
  <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter_all.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/main.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/size.png" width="200"> 
</p>

### Web Interface (Water Meter) 💻
<p align="center">
  <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter.jpg" width="600"> 
</p>

### AI-on-the-edge-device on an Electrical Power Meter ⚡
<p align="center">
  <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/powermeter.jpg" width="600"> 
</p>

---

## Setup 🛠️
There is growing [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/) which provides you with a lot of information. Head there to get started, set it up, and configure it.

There are also articles in the German Heise magazine "make:" about the setup and technical background (behind a paywall): [DIY - Setup](https://www.heise.de/select/make/2021/2/2103513300897420296) 📰

A lot of people have created useful YouTube videos that might help you get started:
- 🎥 [youtube.com/watch?v=HKBofb1cnNc](https://www.youtube.com/watch?v=HKBofb1cnNc)
- 🎥 [youtube.com/watch?v=yyf0ORNLCk4](https://www.youtube.com/watch?v=yyf0ORNLCk4)
- 🎥 [youtube.com/watch?v=XxmTubGek6M](https://www.youtube.com/watch?v=XxmTubGek6M)
- 🎥 [youtube.com/watch?v=mDIJEyElkAU](https://www.youtube.com/watch?v=mDIJEyElkAU)
- 🎥 [youtube.com/watch?v=SssiPkyKVVs](https://www.youtube.com/watch?v=SssiPkyKVVs)
- 🎥 [youtube.com/watch?v=MAHE_QyHZFQ](https://www.youtube.com/watch?v=MAHE_QyHZFQ)
- 🎥 [youtube.com/watch?v=Uap_6bwtILQ](https://www.youtube.com/watch?v=Uap_6bwtILQ)

For further background information, head to:
- [Neural Networks](https://www.heise.de/select/make/2021/6/2126410443385102621)
- [Training Neural Networks](https://www.heise.de/select/make/2022/1/2134114065999161585)
- [Programming on the ESP32](https://www.heise.de/select/make/2022/2/2204010051597422030)

---

## Download 🔽
The latest available version can be found on the [Releases page](https://github.com/jomjol/AI-on-the-edge-device/releases).

---

## Flashing the ESP32 💾
Initially, you will have to flash the ESP32 via a USB connection. Later updates are possible directly over the air (OTA using Wi-Fi).

There are different ways to flash your ESP32:
- The preferred way is the [Web Installer and Console](https://jomjol.github.io/AI-on-the-edge-device/index.html), a browser-based tool to flash the ESP32 and extract the log over USB:  
  ![](images/web-installer.png)
- Flash Tool from Espressif
- ESPtool (command-line tool)

See the [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/) for more information.

---

## Flashing the SD Card 💾
The SD card can be set up automatically after the firmware is installed. See the [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#remote-setup-using-the-built-in-access-point) for details. For this to work, the SD card must be FAT formatted (which is the default on a new SD card).

Alternatively, the SD card can still be set up manually. See the [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#3-sd-card) for details.

---

## Casing 🛠️
Various 3D-printable housings can be found here:
- 💧 [Water Meter](https://www.thingiverse.com/thing:4573481)
- ⚡ [Power Meter](https://www.thingiverse.com/thing:5028229)
- 🔥 [Gas Meter](https://www.thingiverse.com/thing:5224101)
- 📷 [ESP32-cam housing only](https://www.thingiverse.com/thing:4571627)

---

## Donate ☕
If you'd like to support the developer with a cup of coffee, you can do so via [PayPal](https://www.paypal.com/donate?hosted_button_id=8TRSVYNYKDSWL).

<p align="center">
  <a href="https://www.paypal.com/donate?hosted_button_id=8TRSVYNYKDSWL"><img border="0" src="images/paypal.png" width="200px" target="_blank"></a>
</p>

---

## Support 💬
If you have any technical problems, please search the [discussions](https://github.com/jomjol/AI-on-the-edge-device/discussions). In case you find a bug or have a feature request, please open an [issue](https://github.com/jomjol/AI-on-the-edge-device/issues).

For any other issues, you can contact the developer via email:  
<p align="center">
  <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/mail.jpg" height="25">
</p>

---

## Changes and History 📜
See the [Changelog](Changelog.md) for detailed information.

---

## Build It Yourself 🔨
See the [Build Instructions](code/README.md) for step-by-step guidance.

---

## Tools 🛠️
* Logfile downloader and combiner (Thanks to [reserve85](https://github.com/reserve85))  
  * Files can be found at ['/tools/logfile-tool'](tbd), and how-to instructions are in the [documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/outdated--Gasmeter-Log-Downloader/).

---

## Additional Ideas 💡
There are some ideas and feature requests which are not currently being pursued—mainly due to capacity constraints on the part of the developers. These features are collected in the [issues](https://github.com/jomjol/AI-on-the-edge-device/issues) and in [FeatureRequest.md](FeatureRequest.md).

---

## Our Contributors ❤️
<!-- Do not manually edit this section! It should get updated using the Github action "Manually update contributors list" -->
<!-- readme: contributors -start -->
<!-- readme: contributors -end -->
