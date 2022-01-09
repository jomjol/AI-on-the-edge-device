# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

### Details on **function**, **installation** and **configuration** can be found on the **[Wiki Page](https://github.com/jomjol/AI-on-the-edge-device/wiki)**

A 3d-printable housing can be found here: https://www.thingiverse.com/thing:4573481

or here https://www.thingiverse.com/thing:5028229

respectively ESP32-Cam housing only: https://www.thingiverse.com/thing:4571627

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
## Coming next

* Automated update of the neural network file (tflite) to make the learing of additional pictures much easier and automated (GitHub action)
* New "hyprid" neural network for digital numbers --> allowing the detection of intermediate states ("ring between two numbers") as a subdigit


------
## Change log
### Known Issues

* slow response of web server during picture analysis
* spontaneous reboots (mostly due to html access during image processing) - self recovery implemented

**General remark:** Beside the `firmware.bin`, typically also the content of `/html` needs to be updated!

------



##### 10.1.0 - Stability Increase (2022-01-09)

- Reduce ESP32 frequency to 160MHz

- Update tflite (new source: https://github.com/espressif/tflite-micro-esp-examples)

- Update analog neural network (ana-s3-q-20220105.tflite)

- Update digital neural network (dig-s1-q-20220102.tflite)

- Increased web-server buffers
- bug fix: compiler compatibility

##### 10.0.2 - Stability Increase (2022-01-01)

- NEW v10.0.2: Corrected JSON error

- Updated compiler toolchain to ESP-IDF 4.3

- Removal of memory leak

- Improved error handling during startup (check PSRAM and camera with remark in logfile)

- MQTT: implemented raw value additionally, removal of regex contrain

- Normalized Parameter ``MaxRateValue``  to "change per minute" 

- HTML: improved input handling

- Corrected error handling: in case of error the old value, rate, timestamp are not transmitted any more

  

##### 9.2.0 - External Illumination (2021-12-02)

- Direct JSON access: ``http://IP-ADRESS/json`` 
- Error message in log file in case camera error during startup
- Upgrade analog CNN to v9.1.0
- Upgrade digital CNN to v13.3.0 (added new images)
- html: support of different ports

##### 9.1.1 - External Illumination (2021-11-16)

- NEW 9.1.1 bug fix: LED implemenetation
- External LEDs: change control mode (resolve bug with more than 2 LEDs)
- Additional info into log file
- Bug fix: decimal shift, html, log file

##### 9.0.0 - External Illumination (2021-10-23)

* Implementation of external illumination to adjust positioning, brightness and color of the illumination now individually
  * Technical details can be found in the wiki: https://github.com/jomjol/AI-on-the-edge-device/wiki/External-LED
  <img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/intern_vs_external.jpg" width="500">
* New housing published for external LEDs and small clearing: https://www.thingiverse.com/thing:5028229






## Additional ideas

There are some ideas and feature request, which are not followed currently - mainly due to capacity reasons on side of the developer. They are collected here: [FeatureRequest.md](FeatureRequest.md)



------

## History

##### 8.5.0 - Multi Meter Support (2021-10-07)

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

