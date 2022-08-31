# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

### Details on **function**, **installation** and **configuration** can be found on the **[Wiki Page](https://github.com/jomjol/AI-on-the-edge-device/wiki)**

A 3d-printable housing can be found here:
  - https://www.thingiverse.com/thing:4573481 (Water Meter)
  - https://www.thingiverse.com/thing:5028229 (Power Meter)
  - https://www.thingiverse.com/thing:4571627 (ESP32-Cam housing only)

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter_all.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/main.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/size.png" width="200"> 

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter.jpg" width="600"> 

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/powermeter.jpg" width="600"> 




## Donate

------

If you would like to support the developer with a cup of coffee you can do that via [Paypal](https://www.paypal.com/donate?hosted_button_id=8TRSVYNYKDSWL).

<form action="https://www.paypal.com/donate" method="post" target="_top">
<input type="hidden" name="hosted_button_id" value="8TRSVYNYKDSWL" />
<input type="image" src="https://www.paypalobjects.com/en_US/DK/i/btn/btn_donateCC_LG.gif" border="0" name="submit" title="PayPal - The safer, easier way to pay online!" alt="Donate with PayPal button" />
<img alt="" border="0" src="https://www.paypal.com/en_DE/i/scr/pixel.gif" width="1" height="1" />
</form>
If you have any technical topics, you can file a issue in this repository. 

In other cases you can contact the developer via email: <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/mail.jpg" height="25"> 


------
## Change log
**General remark:** Besides the file `firmware.bin`, typically the content of `/html` will need to be updated!

------

##### Rolling (2022-08-31)

- WebUI improvements (many thanks to @**[caco3](https://github.com/caco3)**)

##### Rolling (2022-08-30)

- Added Backup Option, improved update page (many thanks to @**[caco3](https://github.com/caco3)**)

##### 11.2.0 - Intermediate Digits (2022-08-28)

- Updated Tensorflow / TFlite to newest tflite (version as of 2022-07-27)
- Updated analog neural network file (`ana-cont_11.3.0_s2.tflite` - default, `ana-class100_0120_s1_q.tflite`)
- Updated digital neural network file (`dig-cont_0570_s3.tflite` - default, `dig-class100_0120_s2_q.tflite`)

- Added automated filtering of tflite-file in the graphical configuration (thanks to @**[caco3](https://github.com/caco3)**)
- Updated consistency algorithm & test cases
- HTML: added favicon and system name, Improved reboot dialog  (thanks to @**[caco3](https://github.com/caco3)**)

##### 11.1.1 - Intermediate Digits (2022-08-22)

- New and improved consistency check (especially with analog and digital counters mixed)
- Bug Fix: digital counter algorithm

##### 11.0.1 - Intermediate Digits (2022-08-18)

- **NEW v11.0.1**: Bug Fix InfluxDB configuration (only update of html.zip necessary)

- Implementation of new CNN types to detect intermediate values of digits with rolling numbers

  - By default the old algo (0, 1, ..., 9, "N") is active (due to the limited types of digits trained so far)
  - Activation can be done by selection a tflite file with the new trained model in the 'config.ini'
  - **Details can be found in the [wiki](https://github.com/jomjol/AI-on-the-edge-device/wiki/Neural-Network-Types)** (different types, trained image types, naming convention)

- Updated  neural network files (and adaption to new naming convention)

- Published a tool to download and combine log files - **Thanks to **

  - Files see ['/tools/logfile-tool'](tbd), How-to see [wiki](https://github.com/jomjol/AI-on-the-edge-device/wiki/Gasmeter-Log-Downloader)

- Bug Fix: InfluxDB enabling in grahic configuration

  

## Tools

* Logfile downloader and combiner (Thx to [reserve85](https://github.com/reserve85))
  * Files see ['/tools/logfile-tool'](tbd), How-to see [wiki](https://github.com/jomjol/AI-on-the-edge-device/wiki/Gasmeter-Log-Downloader)



## Additional Ideas

There are some ideas and feature requests which are not followed currently - mainly due to capacity reasons on side of the developer. They are collected here: [FeatureRequest.md](FeatureRequest.md)



------

## History

##### 10.6.2 - Stability Increase (2022-07-24)

##### 9.2.0 - External Illumination (2021-12-02)

##### 8.5.0 Multi Meter Support (2021-10-07)

##### 7.1.2 MQTT-Update - (2021-06-17)

##### 6.7.2 Image Processing in Memory - (2021-05-01)

##### 5.0.0 Setup Modus - (2020-12-06)

##### 4.1.1 Configuration editor - (2020-12-02)

##### 4.0.0 Tflite Core - (2020-11-15)
##### 3.1.0 MQTT-Client - (2020-10-26)

##### 2.2.1 Version Control - (2020-09-27)


##### 2.1.0 Decimal Shift, Chrome & Edge - (2020-09-25)


##### 2.0.0 Layout update - (2020-09-12)

##### 1.1.3 Initial Version - (2020-09-09)


#### [Full Changelog](Changelog.md)

