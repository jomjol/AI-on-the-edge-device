# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

### Details on **function**, **installation** and **configuration** can be found on the **[Wiki Page](https://github.com/jomjol/AI-on-the-edge-device/wiki)**

A 3d-printable housing can be found here: https://www.thingiverse.com/thing:4571627

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


## Change log

------

### Known Issues

* slow response of web server during picture analysis
* spontaneous reboots (mostly due to html access during image processing) - self recovery implemented

------

**General remark:** Beside the `firmware.bin`, typically also the content of `/html` needs to be updated!



##### Rolling - (2021-01-19)

* Improvement for wlan password: additionally now "`=`" is possible in the password or wlan-name
  
* Reduction of log file info (removal of memory information)

2021-01-18

* New Feature `ExtendedResolution` uses the after comma digit of the last analog counter to increase precision by one digit. 
  This can only be activated in expert mode.

2021-01-17

* Disabling of digital counters implemented
* Attention: do not disable digital and analog!
* Bug fixing:

  * Configuration of analog counters on html-page failed (enable/disable, undefined ROIs)

2021-01-05

* Configuration: simple enabling / disabling of analog counters 

* Alignment Algorithm: implementation of 3 alignment options (`AlignmentAlgo`)

  * `Default` = know algo based on 1 channel SAD (square average difference) - using only the red channel
  * `Accurate` = using all 3 color channels (3x slower)
  * `Fast` = alignment only on first run, afterward only detection of shift of image by comparing SAD-values
  * Time comparison for reference images with 2 times ~2000 pixel²: 
    * Default ~40 s/run, Accurate: 1:40 min/run, **Fast: 7 s/run**
    * `Fast` Option allows must faster value updates (up to 2 per Minute) 

* Configuration of hostname in config.ini: new parameter `hostname` in section `System`

* Switching of GPIO12/13 via http-interface:

  * ```
    /GPIO?GPIO=12&Status=high
    /GPIO?GPIO=12&Status=low
    ```

* Bug fixing: html configuration page

* based on Master v6.0.0 (2021-01-02)

  

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

  

##### 5.0.0 Setup Modus - (2020-12-06)

* Implementation of initial setup modus for fresh installation

* Code restructuring (full compatibility between pure ESP-IDF and Platformio w/ espressif)
  
  

##### 4.1.1 Configuration editor - (2020-12-02)

* Bug fixing: internal improvement of file handling (reduce not responding)


##### 4.1.0 Configuration editor - (2020-11-30)

* Implementation of configuration editor (including basic and expert mode)

* Adjustable time zone to adjust to local time setting (incl. daylight saving time)

* MQTT: additional topic for error reporting

* standardized access to current logfile via `http://IP-ADRESS/logfileact`

* Update digital CNN to v7.2.0, analog CNN to 6.3.0

* Bug fixing: truncation error,  CheckDigitConsistency & PreValue implementation



##### 4.0.0 Tflite Core - (2020-11-15)
* Implementation of rolling log-files

* Update Tflite-Core to master@20201108 (v2.4)

* Bug-fixing for reducing reboots
  
  

##### 3.1.0 MQTT-Client - (2020-10-26)

* Update digital CNN to v6.5.0 and HTML (Info to hostname, IP, ssid)

* New implementation of "checkDigitConsistency" also for digits
* MQTT-Adapter: user and password for sign in MQTT-Broker

##### 3.0.0 MQTT-Client  (2020-10-14)

* Implementation of MQTT Client
* Improved Version Control
* bug-fixing



##### 2.2.1 Version Control  (2020-09-27)

* Bug-Fixing (hostname in wlan.ini and error handling inside flow)
  


##### 2.2.0 Version Control  (2020-09-27)

* Integrated automated versioning system (menu: SYSTEM --> INFO)
* Update Build-System to PlatformIO - Espressif 32 v2.0.0 (ESP-IDF 4.1)


##### 2.1.0 Decimal Shift, Chrome & Edge  (2020-09-25)

* Implementation of Decimal Shift

* Update default CNN for digits to v6.4.0

* Improvement HTML

* Support for Chrome and Edge

* Reduce logging to minimum - extended logging on demand

* Implementation of hostname in wlan.ini (`hostname = "HOSTNAME")`

* Bug fixing, code corrections


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
