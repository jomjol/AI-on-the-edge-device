# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

### Details on **function**, **installation** and **configuration** can be found on the **[Wiki Page](https://github.com/jomjol/AI-on-the-edge-device/wiki)**

A 3d-printable housing can be found here: https://www.thingiverse.com/thing:4571627

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter_all.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/main.jpg" width="200"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/size.png" width="200"> 

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/watermeter.jpg" width="600"> 
<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/edit_reference.jpg" width="600"> 



## Change log

------

### Known Issues

* Parts of the web page only works correctly in **Firefox** and Chrome!
  With **Edge** not all parts (especially the configuration) are **not full functional**.
* spontaneous reboot, especially in case of intensive web server access (improved since v2.0.0)

------

**General remark:** Beside the `firmware.bin`, typically also the content of `/html` needs to be updated!



##### Rolling - (2020-09-21)

* Temperature Logging, Code Corrections

2020-09-20

* Update HTML-Backround (thanks to michaeljoos72)

2020-09-16

* Impovements in hostname

2020-09-14

* Implementation of hostname in wlan.ini (`hostname = "HOSTNAME")` - Parameter is optional

* Bug correction DecimalShift

2020-09-13

* Bug fixing DecimalShift (digits after comma)

* Implementation of decimal shift (New Parameter "DecimalShift = 1" in [PostProcessing])
DecimalShift = 2 --> Result: 123.456 --> 12345.6
  DecimalShift = -1 --> Result: 123.456 --> 12.3456
  The shift is done at the very first step of merging digital and analog, so all following number already will work on the shifted number.
  
* based on v2.0.0 (2020-09-12)

  

##### 2.0.0 Layout update  (2020-09-12)

  * Update to **new and modern layout**
  * Support for Chrome improved
  * Improved robustness: improved error handling in auto flow reduces spontaneous reboots
  * File server: Option for "DELETE ALL"
  * WLan: support of spaces in SSID and password
  * Reference Image: Option for mirror image, option for image update on the fly
  * additional parameter in `wasserzaehler.html?noerror=true`  to suppress an potential error message
  * bug fixing



##### 1.1.3 (2020-09-09)

* **Bug in configuration of analog ROIs corrected** - correction in v.1.0.2 did not work properly
* Improved update page for the web server (`/html` can be updated via a zip-file, which is provided in `/firmware/html.zip`)
* Improved Chrome support

##### 1.1.0 (2020-09-06)

* Implementation of "delete complete directory"
  **Attention: beside the `firmware.bin`, also the content of `/html` needs to be updated!**



##### 1.0.2 (2020-09-06)

* Bug in configuration of analog ROIs corrected
* minor bug correction

##### 1.0.1 (2020-09-05)

* preValue.ini Bug corrected
* minor bug correction

##### 1.0.0 (2020-09-04)

* **First usable version** - compatible to previous project (https://github.com/jomjol/water-meter-system-complete)
* NEW: 
  * no docker container for CNN calculation necessary
  * web based configuration editor on board

##### 0.1.0 (2020-08-07)

* Initial Version


#### [Full Changelog](Changelog.md)



## Solved topics

* n.a.