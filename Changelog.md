# Versions





##### 7.1.2 MQTT-Update - (2021-06-17)

* NEW: 7.1.2: bug fix setting hostname, Flash-LED not off during reboot

* NEW: 7.1.1: bug fix wlan password with "="  (again)

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

##### 6.7.2 Image Processing in Memory - (2021-05-01)

* NEW 6.7.2: Updated html for setup modus - remove reboot on edit configuration)

* NEW 6.7.1: Improved stability of camera (back to v6.6.1) - remove black strips and areas

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