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

* Parts of the web page only works correctly in **Firefox**!
  With **Chrome** or **Edge** not all parts (especially the configuration) is **not full functional**.
* spontaneous reboot, especially in case of intensive web server access

------

**General remark:** beside the `firmware.bin`, typically also the content of `/html` needs to be updated!



##### Rolling - (2020-09-11)

* Improved handling of PreValue
* Improved error handling for automated processflow (reduce spontaneous reboot - see Issues)
* Support of spaces in WLan SSID or password

2020-09-10

* Optimization of "DELETE ALL" - Autoreload of directory after delete, protection of wlan.ini

* Internal Optimization (removal of unnessary error messages, restructure CTfLiteClass)

* additional parameter in `wasserzahler.html?noerror=true`  to suppress an potential error message in case of consitency check (is equal to `ErrorMessage` = False in `config.ini`)

* update ROI-configurator, in case of no ROIs defined

2020-09-09

* Update to new and modern layout (thanks to michaeljoos in iobroker-thread)
* incorporated "Take Picture" to make new reference image
* based on v1.1.3 (2020-09-09)



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