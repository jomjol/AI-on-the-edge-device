# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

### Details on **function**, **installation** and **configuration** can be found on the **[Wiki Page](https://github.com/jomjol/AI-on-the-edge-device/wiki)**

A 3d-printable housing can be found here: https://www.thingiverse.com/thing:4573481

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

## Change log

------

### Known Issues

* slow response of web server during picture analysis
* spontaneous reboots (mostly due to html access during image processing) - self recovery implemented

------

**General remark:** Beside the `firmware.bin`, typically also the content of `/html` needs to be updated!



##### 8.0.2 - Multi Meter Support (2021-07-23

* NEW 8.0.2: saving roundes prevalue, bug fix html server
* NEW 8.0.1: bug fix html handling of parameter `FixedExposure` and `ImageSize`
* Dual / multi meter support (more than 1 number to be recognized)
  This is implemented with the feature "number" on the ROI definition as well as selected options
* MQTT: standardization of the naming - including new topics (`json`,  `freeMem `, `uptime`)c
* Preparation for extended GPIO support (thanks to Zwerk2k) - not tested and fully functional yet
* Bug fixing: html server, memory leak, MQTT connect, hostname, turn of flash LED

<span style="color: red;">**ATTENTION: the configuration and prevalue files are modified automatically and will not be backward compatible!**</span> 



##### 7.1.2 MQTT-Update - (2021-06-17)

* NEW: 7.1.2: bug fix setting hostname, Flash-LED not off during rebootNEW: 7.1.1: bug fix wlan password with "="  (again)
* MQTT error message: changes "no error", send retain flag
* Update wlan handling to esp-idf 4.1
* Upgrade digital CNN to v8.7.0  (added new images)
* Bug fix: MQTT, WLAN, LED-Controll, GPIO usage, fixed IP, calculation flow rate

  

##### 7.0.1 MQTT-Update - (2021-05-13)

* NEW: 7.0.1: bug fix wlan password with "=" 

* Upgrade digital CNN to v8.5.0  (added new images)

* New MQTT topics: flow rate (units/minute), time stamp (last correct read readout)

* Update MQTT/Error topic to " " in case no error (instead of empty string)

* Portrait or landscape image orientation in rotated image (avoid cropping)

  


## Additional ideas

There are some ideas and feature request, which are not followed currently - mainly due to capacity reasons on side of the developer. They are collected here: [FeatureRequest.md](FeatureRequest.md)



------

## History

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



## Solved topics

* n.a.
