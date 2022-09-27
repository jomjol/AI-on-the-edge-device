# Changelog

## [Unreleased]

:bangbang: The release breaks a few things in ota update :bangbang:

**Make sure to read the instructions below carfully!**.

1.) Backup your configuration!
2.) You should update to 11.3.1 before you update to this release. All other are not tested. 
    Rolling junger than 11.3.1 can also be used, but no guaranty.
3.) Upload and update the firmware.bin file from this release. **but do not reboot**
4.) Upload the html-from-11.3.1.zip in html upload and update the web interface.
5.) Now you can reboot.

If anything bricks you can use the initial_esp32_setup.zip ( <https://github.com/jomjol/AI-on-the-edge-device/wiki/Installation> ) as alternative.

### Added

- Automatic release creation
- Newest firmware of rolling branch now automatically build and provided in [Github Actions Output](https://github.com/jomjol/AI-on-the-edge-device/actions) (developers only)
- [\#1068](https://github.com/jomjol/AI-on-the-edge-device/issues/1068) New update mechanism: 
  - Handling of all files (`zip`, `tfl`, `tflite`, `bin`) within in one common update interface
  - Using the `update.zip` from the [Release page](https://github.com/jomjol/AI-on-the-edge-device/releases)
  - Status (`upload`, `processing`, ...) displayed on Web Interface
  - Automatical detection and suggestion for reboot where needed (Web Interface uupdates only need a page refresh)
  - :bangbang: Best for OTA use Firefox. Chrome works with warnings. Safari stuck in upload.

### Changed
- Integrated version info better shown on the Info page and in the log
- Updated menu
- Update used libraries (`tflite`, `esp32-cam`, `esp-nn`, as of 20220924) 
### Fixed

- [\#1029](https://github.com/jomjol/AI-on-the-edge-device/issues/1029) wrong change of `checkDigitConsistency` now working like releases before `11.3.1` 
- Spelling corrections (**[cristianmitran](https://github.com/cristianmitran)**) 


### Removed
- Remove the folder `/firmware` from GitHub repository. 
  If you want to get the latest `firmware.bin` and `html.zip` files, please download from the automated [build action](https://github.com/jomjol/AI-on-the-edge-device/actions) or [release page](https://github.com/jomjol/AI-on-the-edge-device/releases)

## [11.3.1] - (2022-09-17)
Intermediate Digits

- **ATTENTION**: 
  - first update the `firmware.bin` and ensure that the new version is running

  - Only afterwards update the `html.zip`
  
  - Otherwise the downwards compatibility of the new counter clockwise feature is not given and you end in a reboot loop, that needs manual flashing!
  


- **NEW v11.3.1**: corrected corrupted asset `firmware.bin`
- Increased precision (more than 6-7 digits)
- Implements Counter Clockwise Analog Pointers
- Improved post processing algorithm
- Debugging: intensive use of testcases
- MQTT: improved handling, extended logging, automated reconnect
- HTML: Backup Option for Configuration
- HTML: Improved Reboot
- HTML: Update WebUI (Reboot, Infos, CPU Temp, RSSI)
- This version is largely also based on the work of **[caco3](https://github.com/caco3)**,  **[adellafave](https://github.com/adellafave)**,  **[haverland](https://github.com/haverland)**,  **[stefanbode](https://github.com/stefanbode)**, **[PLCHome](https://github.com/PLCHome)**

## [11.2.0] -  (2022-08-28)
Intermediate Digits

- Updated Tensorflow / TFlite to newest tflite (version as of 2022-07-27)
- Updated analog neural network file (`ana-cont_11.3.0_s2.tflite` - default, `ana-class100_0120_s1_q.tflite`)
- Updated digital neural network file (`dig-cont_0570_s3.tflite` - default, `dig-class100_0120_s2_q.tflite`)

- Added automated filtering of tflite-file in the graphical configuration (thanks to @**[caco3](https://github.com/caco3)**)
- Updated consistency algorithm & test cases
- HTML: added favicon and system name, Improved reboot dialog  (thanks to @**[caco3](https://github.com/caco3)**)

## [11.1.1] -  (2022-08-22)
Intermediate Digits

- New and improved consistency check (especially with analog and digital counters mixed)
- Bug Fix: digital counter algorithm

## [11.0.1] - (2022-08-18)
Intermediate Digits

- **NEW v11.0.1**: Bug Fix InfluxDB configuration (only update of html.zip necessary)

- Implementation of new CNN types to detect intermediate values of digits with rolling numbers

  - By default the old algo (0, 1, ..., 9, "N") is active (due to the limited types of digits trained so far)
  - Activation can be done by selection a tflite file with the new trained model in the 'config.ini'
  - **Details can be found in the [wiki](https://github.com/jomjol/AI-on-the-edge-device/wiki/Neural-Network-Types)** (different types, trained image types, naming convention)

- Updated  neural network files (and adaption to new naming convention)

- Published a tool to download and combine log files - **Thanks to **

  - Files see ['/tools/logfile-tool'](tbd), How-to see [wiki](https://github.com/jomjol/AI-on-the-edge-device/wiki/Gasmeter-Log-Downloader)

- Bug Fix: InfluxDB enabling in grahic configuration


## [10.6.2] - (2022-07-24)

Stability Increase

### Added

-   **NEW 10.6.2**: ignore hidden files in model selection (configuration page)

-   **NEW 10.6.1**: Revoke esp32cam & tflite update

-   **NEW 10.6.1**: Bug Fix: tflite-filename with ".", HTML spelling error

-   IndluxDB: direct injection into InfluxDB - thanks to **[wetneb](https://github.com/wetneb)**

-   MQTT: implemented "Retain Flag" and extend with absolute Change (in addition to rate)

-   `config.ini`: removal of modelsize (readout from tflite)

-   Updated analog neural network file (`ana1000s2.tflite`) & digital neural network file (`dig1400s2q.tflite`)

-   TFMicro/Lite: Update (espressif Version 20220716)

-   Updated esp32cam (v20220716)

-   ESP-IDF: Update to 4.4

-   Internal update (CNN algorithm optimizations, reparation for new neural network type)

-   Bug Fix: no time with fixed IP, Postprocessing, MQTT

## [10.5.2] - (2022-02-22)

Stability Increase

### Changed

-   NEW 10.5.2: Bug Fix: wrong `firmware.bin` (no rate update)
-   NEW 10.5.1: Bug Fix: wrong return value, rate value & PreValue status, HTML: SSID & IP were not displayed
-   MQTT: changed wifi naming to "wifiRSSI"
-   HTML: check selectable values for consistency
-   Refactoring of check postprocessing consistency (e.g. max rate, negative rate, ...)
-   Bug Fix: corrected error in "Check Consistency Increase"

## [10.4.0] - (2022-02-12)

Stability Increase

### Changed

-   Graphical configuration: select available neural network files (_.tfl,_.tflite) from drop down menu
-   OTA-update: add option to upload tfl / tflite files to the correct location (`/config/`)
    -   In the future the new files will also be copied to the `firmware` directory of the repository
-   Added Wifi RSSI to MQTT information
-   Updated analog neural network file (`ana-s3-q-20220105.tflite`)
-   Updated digital neural network file (`dig-s1-q-20220102.tflite`)
-   Updated build environment to `Espressif 3.5.0`

## [10.3.0] - (2022-01-29)

Stability Increase

### Changed

-   Implemented LED flash dimming (`LEDIntensity`).
    Remark: as auto illumination in the camera is used, this is rather for energy saving. It will not help reducing reflections
-   Additional camera parameters: saturation, contrast (although not too much impact yet)
-   Some readings will have removable "N"s that can not be removed automatically and are handled with an "error" --> no return value in the field "value" anymore (still reported back via field "raw value")
-   Updated esp32 camera hardware driver
-   Bug fix: MQTT, HTML improvements

**ATTENTION:  The new ESP32 camera hardware driver is much more stable on newer OV2640 versions (no or much less reboots) but seems to be not fully compatible with older versions.**

If you have problem with stalled systems you can try the following

-   Update the parameter `ImageQuality` to `12` instead of current value `5` (manually in the `config.ini`)

-   If this is not helping, you might need to update your hardware or stay with version 9.2

## [10.2.0] - (2022-01-14)

Stability Increase

### Changed

-   Due to the updated camera driver, the image looks different and a new setup might be needed

    -   Update reference image
    -   Update Alignment marks

-   Reduce reboot due to camera problems

-   Update esp32-camera to new version (master as of 2022-01-09)

## [10.1.1] - (2022-01-12)

 Stability Increase

### Changed

-   Bug Fix MQTT problem
-   Issue:
    -   Changing from v9.x to 10.x the MQTT-parameter "Topic" was renamed into "MainTopic" to address multiple number meters. This renaming should have been done automatically in the background within the graphical configuration, but was not working. Instead the parameter "Topic" was deleted and "MainTopic" was set to disabled and "undefined".
-   ToDo
    -   Update the `html.zip`
    -   If old `config.ini` available: copy it to `/config`, open the graphical configuration and save it again.
    -   If old `config.ini` not available: reset the parameter "MainTopic" within the `config.ini` manually
    -   Reboot

## [10.1.0] -  (2022-01-09)

Stability Increase

### Changed

-   Reduce ESP32 frequency to 160MHz

-   Update tflite (new source: <https://github.com/espressif/tflite-micro-esp-examples>)

-   Update analog neural network (ana-s3-q-20220105.tflite)

-   Update digital neural network (dig-s1-q-20220102.tflite)

-   Increased web-server buffers

-   bug fix: compiler compatibility

## [10.0.2] - (2022-01-01)

Stability Increase

### Changed

-   NEW v10.0.2: Corrected JSON error

-   Updated compiler toolchain to ESP-IDF 4.3

-   Removal of memory leak

-   Improved error handling during startup (check PSRAM and camera with remark in logfile)

-   MQTT: implemented raw value additionally, removal of regex contrain

-   Normalized Parameter `MaxRateValue`  to "change per minute"

-   HTML: improved input handling

-   Corrected error handling: in case of error the old value, rate, timestamp are not transmitted any more

## [9.2.0] -  (2021-12-02)

External Illumination

### Changed

-   Direct JSON access: `http://IP-ADRESS/json`
-   Error message in log file in case camera error during startup
-   Upgrade analog CNN to v9.1.0
-   Upgrade digital CNN to v13.3.0 (added new images)
-   html: support of different ports

## [9.1.1] - External Illumination (2021-11-16)

### Changed

-   NEW 9.1.1 bug fix: LED implemenetation
-   External LEDs: change control mode (resolve bug with more than 2 LEDs)
-   Additional info into log file
-   Bug fix: decimal shift, html, log file

## [9.0.0] - External Illumination (2021-10-23)

### Changed

-   Implementation of external illumination to adjust positioning, brightness and color of the illumination now set individually
    -   Technical details can be found in the wiki: <https://github.com/jomjol/AI-on-the-edge-device/wiki/External-LED>
        <img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/intern_vs_external.jpg" width="500">
-   New housing published for external LEDs and small clearing: <https://www.thingiverse.com/thing:5028229>

## [8.5.0] - Multi Meter Support (2021-10-07)

### Changed

-   Upgrade digital CNN to v13.1.0 (added new images)
-   bug fix: wlan password with space, double digit output

## [8.4.0] - Multi Meter Support (2021-09-25)

### Changed

-   License change (remove MIT license, remark see below)

-   html: show hostname in title and main page

-   configuration:

    -   moved setting `ExtendedResolution` to individual number settings
    -   New parameter `IgnoreLeadingNaN` (delete leading NaN's specifically)
    -   **ATTENTION**: update of the `config.ini` needed (open, adjust `ExtendedResolution`, save)

-   Bug fixing (html, images of recognized numbers)

    **ATTENTION: LICENSE CHANGE - removal of MIT License.**

-   Currently no licence published - copyright belongs to author

-   If you are interested in a commercial usage or dedicated versions please contact the developer
    -   no limits to private usage

## [8.3.0] - Multi Meter Support (2021-09-12)

### Changed

-   Upgrade digital CNN to v12.1.0 (added new images)
-   Dedicated NaN handling, internal refactoring (CNN-Handling)
-   HTML: confirmation after config.ini update
-   Bug fixing

## [8.2.0] - Multi Meter Support (2021-08-24)

### Changed

-   Improve server responsiveness


-   Flow status and prevalue status in overview
-   Improved prevalue handling

## [8.1.0] - Multi Meter Support (2021-08-12)

### Changed

-   GPIO: using the general mqtt main topic for GPIO


-   Upgrade digital CNN to v12.0.0  (added new images)
-   Update tfmicro to new master (2021-08-07)
-   Bug fix: remove text in mqtt value, remove connect limit in wlan reconnet

## [8.0.5] - Multi Meter Support (2021-08-01)

### Changed

-   NEW 8.0.5: bug fix: saving prevalue


-   NEW 8.0.4: bug fix: load config.ini after upgrade
-   NEW 8.0.3: bug fix: reboot during `config.ini` handling, html error
-   NEW 8.0.2: saving roundes prevalue, bug fix html server
-   NEW 8.0.1: bug fix: html handling of parameter `FixedExposure` and `ImageSize`
-   Dual / multi meter support (more than 1 number to be recognized)
    This is implemented with the feature "number" on the ROI definition as well as selected options
-   MQTT: standardization of the naming - including new topics (`json`,  `freeMem`, `uptime`)c
-   Preparation for extended GPIO support (thanks to Zwerk2k) - not tested and fully functional yet
-   Bug fixing: html server, memory leak, MQTT connect, hostname, turn of flash LED

<span style="color: red;">**ATTENTION: the configuration and prevalue files are modified automatically and will not be backward compatible!**</span>

## [7.1.2] MQTT-Update - (2021-06-17)

### Changed

-   NEW: 7.1.2: bug fix setting hostname, Flash-LED not off during reboot


-   NEW: 7.1.1: bug fix wlan password with "="  (again)

-   MQTT error message: changes "no error", send retain flag

-   Update wlan handling to esp-idf 4.1

-   Upgrade digital CNN to v8.7.0  (added new images)

-   Bug fix: MQTT, WLAN, LED-Controll, GPIO usage, fixed IP, calculation flow rate

## [7.0.1] MQTT-Update - (2021-05-13)

### Changed

-   NEW: 7.0.1: bug fix wlan password with "="


-   Upgrade digital CNN to v8.5.0  (added new images)

-   New MQTT topics: flow rate (units/minute), time stamp (last correct read readout)

-   Update MQTT/Error topic to " " in case no error (instead of empty string)

-   Portrait or landscape image orientation in rotated image (avoid cropping)

## [6.7.2] Image Processing in Memory - (2021-05-01)

### Changed

-   NEW 6.7.2: Updated html for setup modus - remove reboot on edit configuration)


-   NEW 6.7.1: Improved stability of camera (back to v6.6.1) - remove black strips and areas

-   Upgrade digital CNN to v8.3.0  (added new type of digits)

-   Internal update: TFlite (v2.5), esp32cam, startup sequence

-   Rollback to espressif v2.1.0, as v3.2.0 shows unstable reboot

-   Bugfix: WLan-passwords, reset of hostname

## [6.6.1] Image Processing in Memory - (2021-04-05)

### Changed

-   NEW 6.6.1: failed SD card initialization indicated by fast blinking LED at startup


-   Improved SD-card handling (increase compatibility with more type of cards)

## [6.5.0] Image Processing in Memory - (2021-03-25)

### Changed

-   Upgrade digital CNN to v8.2.0  (added new type of digits)


-   Supporting alignment structures in ROI definition
-   Bug fixing: definition of  hostname in `config.ini`

## [6.4.0] Image Processing in Memory - (2021-03-20)

### Changed

-   Additional alignment marks for settings the ROIs (analog and digit)


-   Upgrade analog CNN to v7.0.0 (added new type of pointer)

## [6.3.1] Image Processing in Memory - (2021-03-16)

### Changed

-   NEW: 6.3.1: bug fixing in initial edit reference image and `config.ini` (Spelling error in `InitialRotate`)


-   Initial setup mode: bug fixing, error correction
-   Bug-fixing

## [6.2.2] Image Processing in Memory - (2021-03-10)

### Changed

-   NEW 6.2.2: bug fixing


-   NEW 6.2.1: Changed brightness and contrast to default if not enabled (resolves to bright images)

-   Determination of fixed illumination settings during startup - speed up of 5s in each run

-   Update digital CNN to v8.1.1 (additional digital images trained)

-   Extended error message in MQTT error message

-   Image brightness is now adjustable

-   Bug fixing: minor topics

## [6.1.0] Image Processing in Memory - (2021-01-20)

### Changed

-   Disabling of analog / digital counters in configuration


-   Improved Alignment Algorithm (`AlignmentAlgo`  = `Default`,  `Accurate` , `Fast`)
-   Analog counters: `ExtendedResolution` (last digit is extended by sub comma value of CNN)
-   `config.ini`: additional parameter `hostname`  (additional to wlan.ini)
-   Switching of GPIO12/13 via http-interface: `/GPIO?GPIO=12&Status=high/low`
-   Bug fixing: html configuration page, wlan password ("=" now possible)

## [6.0.0] Image Processing in Memory - (2021-01-02)

### Changed

-   **Major change**: image processing fully in memory - no need of SD card buffer anymore

    -   Need to limit camera resolution to VGA (due to memory limits)


-   MQTT: Last Will Testament (LWT) implemented: "connection lost" in case of connection lost to `TopicError`
-   Disabled `CheckDigitIncreaseConsistency` in default configuration - must now be explicit enabled if needed
-   Update digital CNN to v7.2.1 (additional digital images trained)
-   Setting of arbitrary time server in `config.ini`
-   Option for fixed IP-, DNS-Settings in `wlan.ini`
-   Increased stability (internal image and camera handling)
-   Bug fixing: edit digits, handling PreValue, html-bugs

## [5.0.0] Setup Modus - (2020-12-06)

### Changed

-   Implementation of initial setup modus for fresh installation


-   Code restructuring (full compatibility between pure ESP-IDF and Platformio w/ espressif)

## [4.1.1] Configuration editor - (2020-12-02)

### Changed

-   Bug fixing: internal improvement of file handling (reduce not responding)

## [4.1.0] Configuration editor - (2020-11-30)

### Changed

-   Implementation of configuration editor (including basic and expert mode)


-   Adjustable time zone to adjust to local time setting (incl. daylight saving time)

-   MQTT: additional topic for error reporting

-   standardized access to current logfile via `http://IP-ADRESS/logfileact`

-   Update digital CNN to v7.2.0, analog CNN to 6.3.0

-   Bug fixing: truncation error,  CheckDigitConsistency & PreValue implementation

## [4.0.0] Tflite Core - (2020-11-15)

### Changed

-   Implementation of rolling log-files


-   Update Tflite-Core to master@20201108 (v2.4)

-   Bug-fixing for reducing reboots

## [3.1.0] MQTT-Client - (2020-10-26)

### Changed

-   Update digital CNN to v6.5.0 and HTML (Info to hostname, IP, ssid)

-   New implementation of "checkDigitConsistency" also for digits

-   MQTT-Adapter: user and password for sign in MQTT-Broker

## [3.0.0] MQTT-Client  (2020-10-14)

### Changed

-   Implementation of MQTT Client


-   Improved Version Control
-   bug-fixing

## [2.2.1] Version Control  (2020-09-27)

### Changed

-   Bug-Fixing (hostname in wlan.ini and error handling inside flow)

## \[2.2.0| Version Control  (2020-09-27)

### Changed

-   Integrated automated versioning system (menu: SYSTEM --> INFO)


-   Update Build-System to PlatformIO - Espressif 32 v2.0.0 (ESP-IDF 4.1)

## [2.1.0] Decimal Shift, Chrome & Edge  (2020-09-25)

### Changed

-   Implementation of Decimal Shift


-   Update default CNN for digits to v6.4.0

-   Improvement HTML

-   Support for Chrome and Edge

-   Reduce logging to minimum - extended logging on demand

-   Implementation of hostname in wlan.ini (`hostname = "HOSTNAME")`

-   Bug fixing, code corrections

## [2.0.0] Layout update  (2020-09-12)

### Changed

-   Update to **new and modern layout**
-   Support for Chrome improved
-   Improved robustness: improved error handling in auto flow reduces spontaneous reboots
-   File server: Option for "DELETE ALL"
-   WLan: support of spaces in SSID and password
-   Reference Image: Option for mirror image, option for image update on the fly
-   additional parameter in `wasserzaehler.html?noerror=true`  to suppress an potential error message
-   bug fixing

## [1.1.3](2020-09-09)

### Changed

-   **Bug in configuration of analog ROIs corrected** - correction in v.1.0.2 did not work properly


-   Improved update page for the web server (`/html` can be updated via a zip-file, which is provided in `/firmware/html.zip`)
-   Improved Chrome support

## [1.1.0](2020-09-06)

### Changed

-   Implementation of "delete complete directory"
    **Attention: beside the `firmware.bin`, also the content of `/html` needs to be updated!**

## [1.0.2](2020-09-06)

### Changed

-   Bug in configuration of analog ROIs corrected


-   minor bug correction

## [1.0.1](2020-09-05)

### Changed

-   preValue.ini Bug corrected


-   minor bug correction

## [1.0.0](2020-09-04)

### Changed

-   **First usable version** - compatible to previous project (<https://github.com/jomjol/water-meter-system-complete>)


-   NEW:
    -   no docker container for CNN calculation necessary
    -   web based configuration editor on board

## [0.1.0](2020-08-07)

### Changed

-   Initial Version

[Unreleased]: https://github.com/haverland/AI-on-the-edge-device/compare/11.4.3...HEAD

[11.4.3]: https://github.com/haverland/AI-on-the-edge-device/compare/10.6.2...11.4.3

[11.4.2]: https://github.com/haverland/AI-on-the-edge-device/compare/10.6.2...11.4.2

[11.3.9]: https://github.com/haverland/AI-on-the-edge-device/compare/10.6.2...11.3.9
