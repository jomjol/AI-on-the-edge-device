# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

### Details on **function**, **installation** and **configuration** can be found on the **[Wiki Page](https://github.com/jomjol/AI-on-the-edge-device/wiki)**

A 3d-printable housing can be found here: https://www.thingiverse.com/thing:4573481

respectively ESP32-Cam housing only: https://www.thingiverse.com/thing:4571627

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter_all.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/main.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/size.png" width="200"> 

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter.jpg" width="600"> 




## Donate

------

If you would like to support the developer with a cup of coffee you can do that via [Paypal](https://www.paypal.com/donate?hosted_button_id=8TRSVYNYKDSWL).

<form action="https://www.paypal.com/donate" method="post" target="_top">
<input type="hidden" name="hosted_button_id" value="8TRSVYNYKDSWL" />
<input type="image" src="https://www.paypalobjects.com/en_US/DK/i/btn/btn_donateCC_LG.gif" border="0" name="submit" title="PayPal - The safer, easier way to pay online!" alt="Donate with PayPal button" />
<img alt="" border="0" src="https://www.paypal.com/en_DE/i/scr/pixel.gif" width="1" height="1" />
</form>
If you have any technical topics, you can file a issue in this repository. 
In other cases you can contact the developer via email: <img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/mail.jpg" height="30"> 

## Change log

------

### Known Issues

* slow response of web server during picture analysis
* spontaneous reboots (mostly due to html access during image processing) - self recovery implemented

------

**General remark:** Beside the `firmware.bin`, typically also the content of `/html` needs to be updated!



##### 6.7.0 Image Processing in Memory - (2021-04-23)

* Upgrade digital CNN to v8.3.0  (added new type of digits)
* Internal update: TFlite (v2.5), esp32cam, startup sequence
* Rollback to espressif v2.1.0, as v3.2.0 shows unstable reboot

* Bugfix: WLan-passwords, reset of hostname

  

##### 6.6.1 Image Processing in Memory - (2021-04-05)

* NEW 6.6.1: failed SD card initialization indicated by fast blinking LED at startup
* Improved SD-card handling (increase compatibility with more type of cards)

##### 6.5.0 Image Processing in Memory - (2021-03-25)

* Upgrade digital CNN to v8.2.0  (added new type of digits)
* Supporting alignment structures in ROI definition
* Bug fixing: definition of  hostname in `config.ini`

##### 6.4.0 Image Processing in Memory - (2021-03-20)

* Additional alignment marks for settings the ROIs (analog and digit)
* Upgrade analog CNN to v7.0.0 (added new type of pointer)

##### 6.3.1 Image Processing in Memory - (2021-03-16)

* NEW: 6.3.1: bug fixing in initial edit reference image and `config.ini` (Spelling error in `InitialRotate`)
* Initial setup mode: bug fixing, error correction
* Bug-fixing

##### 6.2.2 Image Processing in Memory - (2021-03-10)

* NEW 6.2.2: bug fixing
* NEW 6.2.1: Changed brightness and contrast to default if not enabled (resolves to bright images)
* Determination of fixed illumination settings during startup - speed up of 5s in each run
* Update digital CNN to v8.1.1 (additional digital images trained)
* Extended error message in MQTT error message


* Image brightness is now adjustable 


* Bug fixing: minor topics 


##### 6.1.0 Image Processing in Memory - (2021-01-20)

* Disabling of analog / digital counters in configuration 
* Improved Alignment Algorithm (`AlignmentAlgo`  = `Default`,  `Accurate` , `Fast`)
* Analog counters: `ExtendedResolution` (last digit is extended by sub comma value of CNN)
* `config.ini`: additional parameter `hostname`  (additional to wlan.ini)
* Switching of GPIO12/13 via http-interface: `/GPIO?GPIO=12&Status=high/low`
* Bug fixing: html configuration page, wlan password ("=" now possible)

##### 6.0.0 Image Processing in Memory - (2021-01-02)

* **Major change**: image processing fully in memory - no need of SD card buffer anymore
  
  * Need to limit camera resolution to VGA (due to memory limits)
* MQTT: Last Will Testament (LWT) implemented: "connection lost" in case of connection lost to `TopicError`
* Disabled `CheckDigitIncreaseConsistency` in default configuration - must now be explicit enabled if needed
* Update digital CNN to v7.2.1 (additional digital images trained) 
* Setting of arbitrary time server in `config.ini`
* Option for fixed IP-, DNS-Settings in `wlan.ini`
* Increased stability (internal image and camera handling)
* Bug fixing: edit digits, handling PreValue, html-bugs




## Additional ideas

There are some ideas and feature request, which are not followed currently - mainly due to capacity reasons on side of the developer. They are collected here: [FeatureRequest.md](FeatureRequest.md)



------

## History

##### 5.0.0 Setup Modus - (2020-12-06)

##### 4.1.1 Configuration editor - (2020-12-02)

##### 4.0.0 Tflite Core - (2020-11-15)
##### 3.1.0 MQTT-Client - (2020-10-26)

##### 2.2.1 Version Control - (2020-09-27)


##### 2.1.0 Decimal Shift, Chrome & Edge - (2020-09-25)


##### 2.0.0 Layout update - (2020-09-12)

##### 1.1.3 Initial Version - (2020-09-09)


#### [Full Changelog](Changelog.md)



## Solved topics

* n.a.
