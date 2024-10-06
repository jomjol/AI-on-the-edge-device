# Welcome to the AI-on-the-edge-device
<img src="images/icon/watermeter.svg" width="100px">

Artificial intelligence based systems have become established in our everyday lives. Just think of speech or image recognition. Most of the systems rely on either powerful processors or a direct connection to the cloud for doing the calculations there. With the increasing power of modern processors, the AI systems are coming closer to the end user – which is usually called **edge computing**.
In this project, edge computing is demonstrated through a practical example, where an AI network is implemented on an ESP32 device, hence: **AI on the edge**.

This project allows you to digitize your **analog** water, gas, power and other meters using cheap and easily available hardware.

All you need is an [ESP32 board with a supported camera](https://jomjol.github.io/AI-on-the-edge-device-docs/Hardware-Compatibility/) and some practical skills.

<img src="images/esp32-cam.png" width="200px">

## Key features
- Tensorflow Lite (TFlite) integration – including easy-to-use wrapper
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

A lot of people created useful Youtube videos which might help you getting started.
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
Initially you will have to flash the ESP32 via a USB connection. Later updates are possible directly over the air (OTA using WIFI).

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
If you have any technical problems please search the [discussions](https://github.com/jomjol/AI-on-the-edge-device/discussions). In case you found a bug or have a feature request, please open an [issue](https://github.com/jomjol/AI-on-the-edge-device/issues).

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

<!-- readme: contributors -start -->
<table>
	<tbody>
		<tr>
            <td align="center">
                <a href="https://github.com/jomjol">
                    <img src="https://avatars.githubusercontent.com/u/30766535?v=4" width="75;" alt="jomjol"/>
                    <br />
                    <sub><b>jomjol</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/caco3">
                    <img src="https://avatars.githubusercontent.com/u/1783586?v=4" width="75;" alt="caco3"/>
                    <br />
                    <sub><b>caco3</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/haverland">
                    <img src="https://avatars.githubusercontent.com/u/412645?v=4" width="75;" alt="haverland"/>
                    <br />
                    <sub><b>haverland</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/Slider0007">
                    <img src="https://avatars.githubusercontent.com/u/115730895?v=4" width="75;" alt="Slider0007"/>
                    <br />
                    <sub><b>Slider0007</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/SybexX">
                    <img src="https://avatars.githubusercontent.com/u/587201?v=4" width="75;" alt="SybexX"/>
                    <br />
                    <sub><b>SybexX</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/nliaudat">
                    <img src="https://avatars.githubusercontent.com/u/6782613?v=4" width="75;" alt="nliaudat"/>
                    <br />
                    <sub><b>nliaudat</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/Zwer2k">
                    <img src="https://avatars.githubusercontent.com/u/10438794?v=4" width="75;" alt="Zwer2k"/>
                    <br />
                    <sub><b>Zwer2k</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/phlupp">
                    <img src="https://avatars.githubusercontent.com/u/6304863?v=4" width="75;" alt="phlupp"/>
                    <br />
                    <sub><b>phlupp</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/jasaw">
                    <img src="https://avatars.githubusercontent.com/u/721280?v=4" width="75;" alt="jasaw"/>
                    <br />
                    <sub><b>jasaw</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/dockSquadron">
                    <img src="https://avatars.githubusercontent.com/u/11964767?v=4" width="75;" alt="dockSquadron"/>
                    <br />
                    <sub><b>dockSquadron</b></sub>
                </a>
            </td>
		</tr>
		<tr>
            <td align="center">
                <a href="https://github.com/rdmueller">
                    <img src="https://avatars.githubusercontent.com/u/1856308?v=4" width="75;" alt="rdmueller"/>
                    <br />
                    <sub><b>rdmueller</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/cristianmitran">
                    <img src="https://avatars.githubusercontent.com/u/36613624?v=4" width="75;" alt="cristianmitran"/>
                    <br />
                    <sub><b>cristianmitran</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/michaeljoos72">
                    <img src="https://avatars.githubusercontent.com/u/20517474?v=4" width="75;" alt="michaeljoos72"/>
                    <br />
                    <sub><b>michaeljoos72</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/Ayushjhawar8">
                    <img src="https://avatars.githubusercontent.com/u/111112495?v=4" width="75;" alt="Ayushjhawar8"/>
                    <br />
                    <sub><b>Ayushjhawar8</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/henrythasler">
                    <img src="https://avatars.githubusercontent.com/u/6277203?v=4" width="75;" alt="henrythasler"/>
                    <br />
                    <sub><b>henrythasler</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/pixeldoc2000">
                    <img src="https://avatars.githubusercontent.com/u/376715?v=4" width="75;" alt="pixeldoc2000"/>
                    <br />
                    <sub><b>pixeldoc2000</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/mad2xlc">
                    <img src="https://avatars.githubusercontent.com/u/37449746?v=4" width="75;" alt="mad2xlc"/>
                    <br />
                    <sub><b>mad2xlc</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/jochenchrist">
                    <img src="https://avatars.githubusercontent.com/u/2930448?v=4" width="75;" alt="jochenchrist"/>
                    <br />
                    <sub><b>jochenchrist</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/parhedberg">
                    <img src="https://avatars.githubusercontent.com/u/13777521?v=4" width="75;" alt="parhedberg"/>
                    <br />
                    <sub><b>parhedberg</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/slovdahl">
                    <img src="https://avatars.githubusercontent.com/u/1417619?v=4" width="75;" alt="slovdahl"/>
                    <br />
                    <sub><b>slovdahl</b></sub>
                </a>
            </td>
		</tr>
		<tr>
            <td align="center">
                <a href="https://github.com/RaHehl">
                    <img src="https://avatars.githubusercontent.com/u/7577984?v=4" width="75;" alt="RaHehl"/>
                    <br />
                    <sub><b>RaHehl</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/muggenhor">
                    <img src="https://avatars.githubusercontent.com/u/484066?v=4" width="75;" alt="muggenhor"/>
                    <br />
                    <sub><b>muggenhor</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/LordGuilly">
                    <img src="https://avatars.githubusercontent.com/u/13271835?v=4" width="75;" alt="LordGuilly"/>
                    <br />
                    <sub><b>LordGuilly</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/amantyagiprojects">
                    <img src="https://avatars.githubusercontent.com/u/174239452?v=4" width="75;" alt="amantyagiprojects"/>
                    <br />
                    <sub><b>amantyagiprojects</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/ppisljar">
                    <img src="https://avatars.githubusercontent.com/u/13629809?v=4" width="75;" alt="ppisljar"/>
                    <br />
                    <sub><b>ppisljar</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/ralf1307">
                    <img src="https://avatars.githubusercontent.com/u/46164027?v=4" width="75;" alt="ralf1307"/>
                    <br />
                    <sub><b>ralf1307</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/SkylightXD">
                    <img src="https://avatars.githubusercontent.com/u/16561545?v=4" width="75;" alt="SkylightXD"/>
                    <br />
                    <sub><b>SkylightXD</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/ottk3">
                    <img src="https://avatars.githubusercontent.com/u/5236802?v=4" width="75;" alt="ottk3"/>
                    <br />
                    <sub><b>ottk3</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/Turbo87">
                    <img src="https://avatars.githubusercontent.com/u/141300?v=4" width="75;" alt="Turbo87"/>
                    <br />
                    <sub><b>Turbo87</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/yonz2">
                    <img src="https://avatars.githubusercontent.com/u/13886257?v=4" width="75;" alt="yonz2"/>
                    <br />
                    <sub><b>yonz2</b></sub>
                </a>
            </td>
		</tr>
		<tr>
            <td align="center">
                <a href="https://github.com/Yveaux">
                    <img src="https://avatars.githubusercontent.com/u/7716005?v=4" width="75;" alt="Yveaux"/>
                    <br />
                    <sub><b>Yveaux</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/flooxo">
                    <img src="https://avatars.githubusercontent.com/u/93255373?v=4" width="75;" alt="flooxo"/>
                    <br />
                    <sub><b>flooxo</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/kalwados">
                    <img src="https://avatars.githubusercontent.com/u/11840444?v=4" width="75;" alt="kalwados"/>
                    <br />
                    <sub><b>kalwados</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/kub3let">
                    <img src="https://avatars.githubusercontent.com/u/95883234?v=4" width="75;" alt="kub3let"/>
                    <br />
                    <sub><b>kub3let</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/pfeifferch">
                    <img src="https://avatars.githubusercontent.com/u/73090220?v=4" width="75;" alt="pfeifferch"/>
                    <br />
                    <sub><b>pfeifferch</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/rstephan">
                    <img src="https://avatars.githubusercontent.com/u/8532364?v=4" width="75;" alt="rstephan"/>
                    <br />
                    <sub><b>rstephan</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/smartboart">
                    <img src="https://avatars.githubusercontent.com/u/38385805?v=4" width="75;" alt="smartboart"/>
                    <br />
                    <sub><b>smartboart</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/toolsfactory">
                    <img src="https://avatars.githubusercontent.com/u/7744975?v=4" width="75;" alt="toolsfactory"/>
                    <br />
                    <sub><b>toolsfactory</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/mkelley88">
                    <img src="https://avatars.githubusercontent.com/u/5567324?v=4" width="75;" alt="mkelley88"/>
                    <br />
                    <sub><b>mkelley88</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/rainman110">
                    <img src="https://avatars.githubusercontent.com/u/3213107?v=4" width="75;" alt="rainman110"/>
                    <br />
                    <sub><b>rainman110</b></sub>
                </a>
            </td>
		</tr>
		<tr>
            <td align="center">
                <a href="https://github.com/myxor">
                    <img src="https://avatars.githubusercontent.com/u/1397377?v=4" width="75;" alt="myxor"/>
                    <br />
                    <sub><b>myxor</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/joergrosenkranz">
                    <img src="https://avatars.githubusercontent.com/u/310438?v=4" width="75;" alt="joergrosenkranz"/>
                    <br />
                    <sub><b>joergrosenkranz</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/queeek">
                    <img src="https://avatars.githubusercontent.com/u/9533371?v=4" width="75;" alt="queeek"/>
                    <br />
                    <sub><b>queeek</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/eltociear">
                    <img src="https://avatars.githubusercontent.com/u/22633385?v=4" width="75;" alt="eltociear"/>
                    <br />
                    <sub><b>eltociear</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/040medien">
                    <img src="https://avatars.githubusercontent.com/u/115072?v=4" width="75;" alt="040medien"/>
                    <br />
                    <sub><b>040medien</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/hex7c0">
                    <img src="https://avatars.githubusercontent.com/u/4419146?v=4" width="75;" alt="hex7c0"/>
                    <br />
                    <sub><b>hex7c0</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/dkneisz">
                    <img src="https://avatars.githubusercontent.com/u/43378003?v=4" width="75;" alt="dkneisz"/>
                    <br />
                    <sub><b>dkneisz</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/CFenner">
                    <img src="https://avatars.githubusercontent.com/u/9592452?v=4" width="75;" alt="CFenner"/>
                    <br />
                    <sub><b>CFenner</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/PLCHome">
                    <img src="https://avatars.githubusercontent.com/u/29116097?v=4" width="75;" alt="PLCHome"/>
                    <br />
                    <sub><b>PLCHome</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/adarazs">
                    <img src="https://avatars.githubusercontent.com/u/6269603?v=4" width="75;" alt="adarazs"/>
                    <br />
                    <sub><b>adarazs</b></sub>
                </a>
            </td>
		</tr>
		<tr>
            <td align="center">
                <a href="https://github.com/wetneb">
                    <img src="https://avatars.githubusercontent.com/u/309908?v=4" width="75;" alt="wetneb"/>
                    <br />
                    <sub><b>wetneb</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/AngryApostrophe">
                    <img src="https://avatars.githubusercontent.com/u/89547888?v=4" width="75;" alt="AngryApostrophe"/>
                    <br />
                    <sub><b>AngryApostrophe</b></sub>
                </a>
            </td>
		</tr>
	<tbody>
</table>
<!-- readme: contributors -end -->
