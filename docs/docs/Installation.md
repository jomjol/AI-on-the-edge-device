# Installation

The installation requires multiple steps:
1. Get the right hardware and wire it up
1. Flash the firmware onto the ESP32
1. Write the data to the SD-Card
1. Insert the SD-Card into the ESP32 board
1. Power/restart it. 

## Hardware
#### ESP32-CAM

* OV2640 camera module
*  SD-Card slot 
* 4 MB PSRAM. 

It can be easily found on the typical internet stores, searching for ESP32-CAM for less than 10 EUR.

#### USB->UART interface

For first time flashing the firmware a USB -> UART connector is needed. Later firmware upgrades than can be flashed via OTA.

#### Power supply

For power supply a 5V source is needed. Most easily this can be done via an USB power supply. The power supply should support minimum 500mA. For buffering current peaks some users reported to use a large elco condensator like a 2200uF between ground and VCC.

**Attention:** in several internet forums there are problems reported, in case the ESP32-CAM is only supplied with 3.3V.

#### Housing

A small 3D-printable example for a very small case can be found in Thingiverse here: https://www.thingiverse.com/thing:4571627

<img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/main.jpg" width="300"><img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/size.png" width="300"> 



**Attention**: the focus of the OV2640 needs to be adjusted, as it is normally set from ~40cm to infinity. In order to get an image that is big enough, it needs to be changed to about 10cm. Therefore the sealing glue on the objective ring needs to be removed with a scalpel or sharp knife. Afterwards the objective can be rotated clockwise until the image is sharp again.
<img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/focus_adjustment.jpg" width="200">



### Wiring

Beside the 5V power supply, only for the first flashing a connection to the USB-UART connector, including a short cut of GPIO0 to GND for bootloader start.

A example for wiring can be found here:

<img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/wiring.png" width="600"> 
[[/images/progammer_manual.jpg]]



It is also possible to use external LEDs for the illumination instead of the internal flash LED. This is described here: [[External-LED]]




## Firmware flashing
### Files
Grab the firmware from the
 - [Releases page](https://github.com/jomjol/AI-on-the-edge-device/releases) (Stable, tested versions), or the
 - [Automatically build development branch](https://github.com/jomjol/AI-on-the-edge-device/actions?query=branch%3Arolling) (experimental, untested versions). Please have a look on https://github.com/jomjol/AI-on-the-edge-device/wiki/Install-a-rolling-%28unstable%29-release first!

You need:
*   partitions.bin
*   bootloader.bin
*   firmware.bin
*   html.zip


### Flashing
There are several options to flash the firmware. Here three are described:

#### 1. Web Installer
There is a Web Installer available which will work right out of the web browser Edge and Chrome.
You can access it with the following link: https://jomjol.github.io/AI-on-the-edge-device

This is the preferred way for beginners as it also allows access to the USB Log:

[<img src=https://user-images.githubusercontent.com/1783586/200926652-293e9a1c-86ec-4b79-9cef-3e6f3c47ea4b.png height=200px>](https://user-images.githubusercontent.com/1783586/200926652-293e9a1c-86ec-4b79-9cef-3e6f3c47ea4b.png)


#### 2. Using the Flash Tool from Espressif

The flashing of the firmware can be done  with the "Flash Download Tool" from espressif, that can found [here](https://www.espressif.com/en/support/download/other-tools) 

Download and extract the Flash tool, after starting choose "Developer Mode", then "ESP32-DownloadTool" and you are in the setup of the flashing tool. Connect the ESP32-CAM with the USB-UART connection and identify the COM-Port. 

:bangbang: **Attention** :bangbang:  if you reflashing the code again, it is strongly recommended to erase the flash memory before flashing the firmware. Especially if you used OTA in between, which might cause remaining information on the flash, to still boot from an old image in the OTA-area, which is not erased by a normal flash.

But your ESP32 in bootloader mode and push start, then it will identify the board and you can configure the bin-configuration according to the following table:

| Filename       | Offset  |
|----------------|--------:|
| bootloader.bin | 0x1000  |
| partitions.bin | 0x8000  |
| firmware.bin   | 0x10000 |

<img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/Flash_Settings.png" width="400"> 

Alternatively it can be directly flashed from the development environment - here PlatformIO. But this is rather for experienced users, as the whole development chain needs to be installed for compilation.


#### 3. Using esptool in python directly

For this you need a python environment (e.g. Anaconda in Win10). 
Here you need to install the esptool:

```
pip install esptool
```
Then connect the ESP32 with the USB-UART connector to the system, put it in bootmode and with the following command you can erase the flash and flash bootloader, partitions and firmware in two steps:

```
esptool erase_flash
esptool write_flash 0x01000 bootloader.bin 0x08000 partitions.bin 0x10000 firmware.bin
```
- Maybe you need to specify the COM-port if it is not detected by default.
- If the erase command throws the error `A fatal error occurred: ESP32 ROM does not support function erase_flash.`, your `esptool` might be too old, see https://techoverflow.net/2022/02/08/how-to-fix-esp32-a-fatal-error-occurred-esp32-rom-does-not-support-function-erase_flash/

With some Python installations this may not work and you’ll receive an error, try `python -m pip install esptool` or `pip3 install esptool`

Further recommendations can be found on the [espressif webpage](https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html)

## SD-Card
The program expects a SD-Card installed with certain directory and file structure in order to work properly.
For the first setup take the `initial_esp32_setup_*.zip` from the [Release](https://github.com/jomjol/AI-on-the-edge-device/releases) page and extract the content of the contained `sd-card.zip` onto your SD-Card.

This must only be done once as further updates of the SD-Card are possible with the OTA Update.

### :bangbang: Attention :bangbang: 

- Due to the limited availability of GPIOs (OV2640, Flash-Light, PSRAM & SD-Card) the communication mode to the SD card is limited to 1-line SD-Mode. It showed up, that this results in problems with very large SD-Cards (64GB, sometimes 32 GB) and some no name low cost SD-cards.
- There must be no partition table on the SD-card (no GPT, but only MBR for the single partition)
- Following setting are necessary for formating the SD-card: **SINGLE PARTITION, MBR, FAT32 - 32K.  NOT exFAT**
- Some ESP32 devices share their SD-card and/or camera GPIOs with the pins for TX and RX. If you see errors like “Failed to connect” then your chip is probably not entering the bootloader properly. Remove the respective modules temporarily to free the GPIOs for flashing. You may find more information about troubleshooting on the [homepage of Espressif](https://docs.espressif.com/projects/esptool/en/latest/esp8266/troubleshooting.html).

**The ESP32 indicates problems with the SD card during startup with a fast not ending blinking.**
**In this case, please try another SD card.** 

## WLAN

The access to the WLAN is configured in the "wlan.ini" directly on the root directory of the sd-card. Just write the corresponding SSID and password before the startup of the ESP32. This file is hidden from external access (e.g. via Filemanager) to protect the password.

After power on the connection status is indicated by 3x blinking of the red on board LED.

WLAN-Status indication:

* **5 x** fast blinking (< 1 second): connection still pending
* **3 x** slow blinking (1 second on/off): WLAN connection established

It is normal that at first one or two times a pending connection is indicated.