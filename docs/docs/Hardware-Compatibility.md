# Hardware Compatibility

See also https://github.com/jomjol/AI-on-the-edge-device/discussions/1732

General Remark: similar "looking" Board can have major differences:

- Processor
- Ram (Size! & Type) -> this Project needs at least 4MB RAM!
- Flashrom
- Camera Modules
- Onboard/External Antenna
- Quality of Components
- Manufacture Quality of the PCB and soldering
- Different Components
- "Clone" Components -> ESPxx
- etc.

This can cause different Power Consumption, Power Requirements, compatibility issues, etc.

Most manufacturers and sellers buy what's cheap today on the Asian markets. In the end, it looks like it is sometimes a trial and error approach which ESP32-CAM Module works reliable.

Below you find some remarks and experiences from the community:

# ESP32 core itself

| Chip Version              | Image | Status   |
| ------------------------- | ----- | -------- |
| ESP32-D0WDQ6 (revision 1) |       | **okay** |

# PSRAM

| Labeling on PSRAM module                       | Image | Status                    |
| ---------------------------------------------- | ----- | ------------------------- |
| IPUS<br/>IPS640LS0<br/>1815XBGN                |       | **okay**                  |
| AP MEMORY<br/>6404L-3SOR<br/>1040H<br/>110089G |       | **okay**                  |
| AP MEMORY<br/>6404L-3SQR<br/>12205<br/>150047G |       | **okay**<br />8MB              |
| AP MEMORY<br/>6404L-350R<br/>1120A<br/>130027G |       | **NOT OK**<br />PSRAM not accessible|
| AP MEMORY<br/>6404L-3SQR<br/>13100<br/>180026G|       | **NOT OK**<br />PSRAM not accessible|
| AP MEMORY<br/>6404L-3SQR<br/>11207<br/>130024G|       | **NOT OK**<br />PSRAM not accessible|
| ESP PSRAM64H 462021<br/>1B00286                |       | **okay**                  |
| ESP PSRAM16M 302020<br/>                       |       | **NOT OK**<br />2MB only! |

# OV2640 - Camera

The experience with the camera only is based on single modules. It is well possible, that this module had a damage overall and other modules of the same type will work. Give it a try and report to me!

| Labeling on Flex-Connector | Image | Status                            |
| -------------------------- | ----- | --------------------------------- |
| TY-OV2<br/>640-V2.0        |       | **okay**                          |
| DCX-OV2<br/>640-V2         |       | **okay**                          |
| DC-26<br/>40-V3            |       | **okay**: 3x<br/>**NOT OKAY:** 1x |



# ESP32 Modules

| Module                                                       | Image | Status                         |
| ------------------------------------------------------------ | ----- | ------------------------------ |
| ESP32CAM<br/>Different versions on the market! Especially the PSRAM is sometimes labeled wrong (Label: 4MB, Real: only 2 MB --> will not work!) |       | **okay**<br />with >=4 MB PSRAM! |
| ESP32-S3-EYE<br />No Flash LED, pins different used (e.g. LCD diskplay) |       | **NOT OKAY**                   |



# SD-Cards

Due to the limited free available gpios (due to all the extensions needed like: camera, sd-card, LED-flash, ...) the sd card is connected in 1-wire mode. There are some cards, that are compatible with the esp32cam module for unknown reasons.
It is observed, that smaller cards (up to 4 GB) tend to be more stable and larger cards have more problems. But quite some exceptions in the forums (4 GB cards not working, 16 G cards working like a charm).


# Devices known to work

Please add links to stores of which you know they work:
 - https://arduino-projekte.info/produkt/esp32-cam-v2-integriertem-ch340-mit-ov2640-kamera-modul/ ? See https://github.com/jomjol/AI-on-the-edge-device/discussions/1041
- https://www.amazon.de/-/en/gp/product/B0B51CQ13R
- https://www.reichelt.de/entwicklerboards-esp32-kamera-2mp-25--debo-cam-esp32-p266036.html?PROVID=2788&gclid=CjwKCAiAqaWdBhAvEiwAGAQlttJnV4azXWDYeaFUuNioMICh-jvxKp6Cifmcep9vvtoT2JRCDqBczRoC7Q0QAvD_BwE (27.12.2022)
 - ...
 Sandisk 2GB Micro SD Class 2 [Sandisk 2GB](https://www.amazon.co.uk/gp/product/B000N3LL02/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1)
AITRIP ESP32 and CAM [ESP-32/CAM](https://www.amazon.co.uk/gp/product/B08X49P8P3/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1)
- [Amazon US - Aideepen ESP32-CAM W BT Board ESP32-CAM-MB Micro USB to Serial Port CH-340G with OV2640 2MP Camera Module Dual Mode](https://www.amazon.com/gp/product/B0948ZFTQZ) with [Amazon US - Cloudisk 5Pack 4GB Micro SD Card 4 GB MicroSD Memory Card Class6](https://www.amazon.com/gp/product/B07QYTP4VN)

# Weak Wifi
The ESP32-CAM supports an external antenna. It requires some soldering skills but can improve the connection quality. See https://randomnerdtutorials.com/esp32-cam-connect-external-antenna/