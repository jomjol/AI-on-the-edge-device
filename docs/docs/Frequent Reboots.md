# Frequent reboots



There are several types of reboots. To get a deeper insight turn on the logging:

1. Internal logging (`config.ini`)
2. Serial log of the UART interface (same as for flashing the firmware)



There are two principle types of reboots

1. Random reboots (always different timing and situation)
2. Permanent Reboots always at the same time



______

### Random reboots

Random reboots have two reasons: overload during HTML access and unstable system

In general: there are several mechanisms in the firmware (like saving previous values), to have a "smooth" reboot without too many notable disturbance.

##### Overload during HTML access

If you frequently access the web server over HTML requests, the firmware tends to reboot. This especially happens during the first run and when the ESP32 is busy with the digitization flow. 

The reason for this are running out of memory during a flow, minor memory leakage in combination with missing error handling.

There is noting you can do about this kind of reboot, beside two thing:

1. Support the firmware development with improved and tested part of code
2. Be patient :-)

##### Unstable system

If your system is sometimes running smoothly over several runs and sometimes reboots obviously randomly, you have an partially unstable device. 

You can check this in the standard log file on the SD card:

```
2021-12-26T06:34:09: task_autodoFlow - round done
2021-12-26T06:34:09: CPU Temperature: 56.1
2021-12-26T06:38:00: task_autodoFlow - next round - Round #23
```

Here you see, that the round #23 is starting, so obviously there were no reboots in the last 22 rounds. There is hardware (ESP32CAM), where only 2-3 stable rounds are possible and others, where way more than 100 rounds without any reboots is possible.
There is noting you can do about it, beside testing different hardware.



______

### Permanent reboots

Permanent reboots at the same situation during the flow has a systematic problem either in the hardware or the configuration. It usually happens during the first run as there all needed parts of the firmware have been loaded for the first time.

To find the reason mostly the serial log of the UART interface from the startup until the reboots is very helpful. It can be stored using the USB / UART interface - the same as for flashing the firmware - and logging the serial output of the ESP32.

Possible problems:

* SD card

* PSRAM too low
* Configuration missing

##### SD card problems

The ESP32CAM is a little bit "picky" with the supported SD cards. Due to the limited availability of GPIOs the SD card can only be accessed via 1-wire mode. Therefore not all SD cards are supported. Several error cases can happen:

###### No SD card

Easy to detect: fast blinking red LED directly after startup, no reaction of the web server etc. at all

###### SD card not supported at all

Error message of no detectable SC card in the log file. **Normal looking** log for a 16GB SD card is like this:

```09:38:25.037 -> [0;32mI (4789) main: Using SDMMC peripheral[0m
09:38:25.037 -> [0;32mI (4789) main: Using SDMMC peripheral[0m
09:38:25.138 -> Name: SC16G
09:38:25.138 -> Type: SDHC/SDXC
09:38:25.138 -> Speed: 20 MHz
09:38:25.138 -> Size: 15193MB
```

Otherwise there is some error message.

###### SD card recognized but not supported

This is the most annoying error. The SD card is detected, but the files cannot be read. Most probably this results in a problem with the WLAN connection, as the first file needed is the `wlan.ini` in the root directory.



##### PSRAM too low

In order to work, there are 4 MB of PSRAM necessary. Normaly the ESP32CAM is equiped with 8 MB, whereof only 4 MB can be used effectively. 
Sometimes, there is hardware, where only 2 MB of PSRAM is present - **even if you have bought a 8 MB module**

You can identify the amount of PSRAM in the serial log file:

```
09:38:21.224 -> [0;32mI (881) psram: This chip is ESP32-D0WD[0m
09:38:21.224 -> [0;32mI (885) spiram: Found 64MBit SPI RAM device[0m
09:38:21.224 -> [0;32mI (890) spiram: SPI RAM mode: flash 40m sram 40m[0m
09:38:21.224 -> [0;32mI (895) spiram: PSRAM initialized, cache is in low/high (2-core) mode.[0m
```

Here you see 64MBit (= 8MByte) - which is okay. False reading would be: 16MBit

The error in the SD log file is typically related with the taking of the image (tbd) as the first time, the system is running out of memory is usually, when it tries to transfer an image from the camera to the PSRAM.

There is nothing to do, than to buy a new ESP32CAM with **really** 64MBit of PSRAM.



##### Configuration missing

There are several files needed during on run cycle. If one of this is missing, the firmware is missing information and tends to reboot due to missing error management:

* `/wlan.ini`

* `/config/config.ini`

* `/config/XXXXX.tflite` (1 time for analog and 1 time for digital)

  where XXXXX is the file name, that is written in the `config.ini`

