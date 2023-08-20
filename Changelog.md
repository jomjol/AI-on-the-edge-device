## [unreleased] - 2023-08-20

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/rolling...v15.3.0)

#### Changed

 - Updates submodules (esp-nn, tflite-micro-example, esp-camera)

 - Explicitly included needed tflite network layers (instead of all) , resulting in much smaller firmware size

   

## [15.3.0] - 2023-07-22

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/v15.3.0...v15.2.4)

#### Changed

 - Updated PlatformIO to `6.3.2`
 - Updated analog tflite files
   - ana-cont_1207_s2_q.tflite
   - dig-cont_0620_s3_q.tflite




## [15.2.4] - 2023-05-02

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/v15.2.1...v15.2.4)

#### Changed
 - Updated PlatformIO to `6.2.0`
 - [#2376](https://github.com/jomjol/AI-on-the-edge-device/pull/2376) Improve logging if Autostart is not enabled

#### Fixed
 - [#2373](https://github.com/jomjol/AI-on-the-edge-device/pull/2373) Allow the Alignment Mark step while status is "Initializing" or "Initialization (delayed)" or while in setup mode
 - [#2381](https://github.com/jomjol/AI-on-the-edge-device/pull/2381) Fix broken sysinfo REST API


## [15.2.1] - 2023-04-27

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/v15.2.0...v15.2.1)

#### Fixed
 - [#2357](https://github.com/jomjol/AI-on-the-edge-device/pull/2357) Fix Alignment Mark issue


## [15.2.0] - 2023-04-23

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/v15.1.1...v15.2.0)

#### Added

-  [#2286](https://github.com/jomjol/AI-on-the-edge-device/pull/2286) Implement a camera livestream handler
-  [#2252](https://github.com/jomjol/AI-on-the-edge-device/pull/2252) Set prevalue using MQTT + set prevalue to RAW value (REST+MQTT)
-  [#2319](https://github.com/jomjol/AI-on-the-edge-device/pull/2319) Extend InfluxDBv1 with individual topic names

#### Changed

-  [#2285](https://github.com/jomjol/AI-on-the-edge-device/pull/2285) Re-implemented PSRAM usage
-  [#2325](https://github.com/jomjol/AI-on-the-edge-device/pull/2325) Keep MainFlowTask alive to handle reboot
-  [#2233](https://github.com/jomjol/AI-on-the-edge-device/pull/2233) Remove trailing slash in influxDBv1
-  [#2305](https://github.com/jomjol/AI-on-the-edge-device/pull/2305) Migration of PlatformIO `5.2.0` to `6.1.0` (resp. ESP IDF from `4.4.2` to `5.0.1`)
-  Various cleanup and refactoring

#### Fixed

-  [#2326](https://github.com/jomjol/AI-on-the-edge-device/pull/2326) Activate save button after Analogue ROI creationSet prevalue using MQTT + set prevalue to RAW value (REST+MQTT)
-  [#2283](https://github.com/jomjol/AI-on-the-edge-device/pull/2283) Fix Timezone issues on InfluxDB
-  Various minor fixes

#### Removed

-   n.a.


## [15.1.1] - 2023-03-23

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/v15.1.0...v15.1.1)

#### Added

- [#2206](https://github.com/jomjol/AI-on-the-edge-device/pull/2206) Log PSRAM usage
- [#2216](https://github.com/jomjol/AI-on-the-edge-device/pull/2216) Log MQTT connection refused reasons

#### Changed

- n.a.

#### Fixed

-  [#2224](https://github.com/jomjol/AI-on-the-edge-device/pull/2224), [#2213](https://github.com/jomjol/AI-on-the-edge-device/pull/2213) Reverted some of the PSRAM usage changes due to negative sideffects 
-  [#2203](https://github.com/jomjol/AI-on-the-edge-device/issues/2203) Correct API for pure InfluxDB v1
-  [#2180](https://github.com/jomjol/AI-on-the-edge-device/pull/2180) Fixed links in Parameter Documentation
-  Various minor fixes

#### Removed

-   n.a.

## [15.1.0] - 2023-03-12

### Update Procedure

Update Procedure see [online documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#update-ota-over-the-air)

:bangbang: Afterwards you should force-reload the Web Interface (usually Ctrl-F5 will do it)!

:bangbang: Afterwards you should check your configuration for errors!

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/v15.0.3...v15.1.0)

#### Added
- The Configuration page has now tooltips with enhanced documentation
- MQTT:
    - Added `GJ` (`gigajoule`) as an energy meter unit
    - Removed State Class and unit from `raw` topic
    - Various Improvements (Only send Homeassistant Discovery the first time we connect, ...) (https://github.com/jomjol/AI-on-the-edge-device/pull/2091
- Added Expert Parameter to change CPU Clock from `160` to `240 Mhz`
- SD card basic read/write check and a folder/file presence check at boot to indicate SD card issues or missing folders / files ([#2085](https://github.com/jomjol/AI-on-the-edge-device/pull/2085))
- Simplified "WIFI roaming" by client triggered channel scan (AP switching at low RSSI) -> using expert parameter "RSSIThreshold" ([#2120](https://github.com/jomjol/AI-on-the-edge-device/pull/2120))
- Log WLAN disconnect reason codes (see [WLAN disconnect reasons](https://jomjol.github.io/AI-on-the-edge-device-docs/WLAN-disconnect-reason))
- Support of InfluxDB v2 ([#2004](https://github.com/jomjol/AI-on-the-edge-device/pull/2004))


#### Changed
- Updated models (tflite files), removed old versions (https://github.com/jomjol/AI-on-the-edge-device/pull/2089, https://github.com/jomjol/AI-on-the-edge-device/pull/2133)
  :bangbang: **Attention:** Update your configuration!
    -   Hybrid CNN network to `dig-cont_0611_s3` 
    -   Analog CNN network to `ana-cont-11.0.5` and `ana-clas100-1.5.7`
    -   Digital CNN network to `dig-class100-1.6.0`
-   Various Web interface Improvements/Enhancements:
    - Restructured Menu (Needs cache clearing to be applied)
    - Enhanced `Previous Value` page
    - Improved/faster Graph page
    - Various minor improvements
    - ROI config pages improvements
    - Improved Backup Functionality
- Added log file logs for Firmware Update
- Improved memory management (moved various stuff to external PSRAM, https://github.com/jomjol/AI-on-the-edge-device/pull/2117)
- Camera driver update: Support of contrast and saturation ([#2048](https://github.com/jomjol/AI-on-the-edge-device/pull/2048))   
  :bangbang:  **Attention**: This could have impact to old configurations. Please check your configuration and potentially adapt parametrization, if detection is negativly affected.
- Improved error handling and provide more verbose output in error cases during boot phase ([#2020](https://github.com/jomjol/AI-on-the-edge-device/pull/2020))
- Red board LED is indicating more different errors and states (see [Status LED Blink Codes](https://jomjol.github.io/AI-on-the-edge-device-docs/StatusLED-BlinkCodes))
- Logfile: Print start indication block after time is synced to indicate start in logfile after a cold boot
- `Image Quality Index`: Limit lower input range to 8 to avoid system instabilities

#### Fixed
- Various minor fixes
- Added State Class "measurement" to rate_per_time_unit
- GPIO: Avoid MQTT publishing to empty topic when "MQTT enable" flag is not set
- Fix timezone config parser
- Remote Setup truncated long passwords (https://github.com/jomjol/AI-on-the-edge-device/issues/2167)
-  Problem with timestamp in InfluxDB interface

#### Removed
-   n.a.


## [15.0.3] - 2023-02-28

**Name: Parameter Migration**

### Update Procedure

Update Procedure see [online documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#update-ota-over-the-air)

:bangbang: Afterwards you should force-reload the Web Interface (usually Ctrl-F5 will do it).

### Changes

This release only migrates some parameters, see #2023 for details and a list of all parameter changes.
The parameter migration happens automatically on the next startup. No user interaction is required.
A backup of the config is stored on the SD-card as `config.bak`.

Beside of the parameter change and the bugfix listed below, no changes are contained in this release!

If you want to revert back to `v14` or earlier, you will have to revert the migration changes in `config.ini` manually!

#### Added

-   n.a.

#### Changed

-   [#2023](https://github.com/jomjol/AI-on-the-edge-device/pull/2023) Migrated Parameters
-   Removed old `Topic` parameter, it is not used anymore

#### Fixed

-   [#2036](https://github.com/jomjol/AI-on-the-edge-device/issues/2036) Fix wrong url-encoding
-   **NEW v15.0.2:**  [#1933](https://github.com/jomjol/AI-on-the-edge-device/issues/1933) Bugfix InfluxDB Timestamp
-   **NEW v15.0.3:**  Re-added lost dropdownbox filling for Postprocessing Individual Parameters

#### Removed

-   n.a.


## [14.0.3] -2023-02-05

**Name: Stabilization and Improved User Experience**

Thanks to over 80 Pull Requests from 6 contributors, we can anounce another great release with many many improvements and new features:

### Update Procedure

Update Procedure see [online documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#update-ota-over-the-air)

### Changes

For a full list of changes see [Full list of changes](https://github.com/jomjol/AI-on-the-edge-device/compare/v13.0.8...v14.0.0)

#### Added

-   [1877](https://github.com/jomjol/AI-on-the-edge-device/pull/1877) Show WIFI signal text labels / Log RSSI value to logfile
-   [1671](https://github.com/jomjol/AI-on-the-edge-device/pull/1671) Added experimental support for WLAN 802.11k und 802.11v (Mesh-Support)
-   Web UI caching of static files
-   Added various debug tools
-   [1798](https://github.com/jomjol/AI-on-the-edge-device/pull/1798) Add error handling for memory intensive tasks
-   [1784](https://github.com/jomjol/AI-on-the-edge-device/pull/1784) Add option to disable brownout detector
-   Added full web browser based installation mode (including initial setup of SD-card) - see [WebInstaller](https://jomjol.github.io/AI-on-the-edge-device/index.html)
-   Added [Demo Mode](https://jomjol.github.io/AI-on-the-edge-device-docs/Demo-Mode)
-   [1648](https://github.com/jomjol/AI-on-the-edge-device/pull/1648) Added trigger to start a flow by [REST](https://jomjol.github.io/AI-on-the-edge-device-docs/REST-API) API or [MQTT](https://jomjol.github.io/AI-on-the-edge-device-docs/MQTT-API/)
-   Show special images during steps `Initializing` and `Take Image` as the current camera image might be incomplete or outdated

#### Changed

-   Migrated documentation (Wiki) to <https://jomjol.github.io/AI-on-the-edge-device-docs>. Please help us to make it even better.
-   New OTA Update page with progress indication
-   Various memory optimizations
-   Cleanup code/Web UI
-   Updated models
-   [1809](https://github.com/jomjol/AI-on-the-edge-device/pull/1809) Store preprocessed image with ROI to RAM
-   Better log messages on some errors/issues
-   [1742](https://github.com/jomjol/AI-on-the-edge-device/pull/1742) Replace alert boxes with overlay info boxes
-   Improve log message when web UI is installed incomplete
-   [1676](https://github.com/jomjol/AI-on-the-edge-device/pull/1676) Improve NTP handling
-   HTML: improved user informations (info boxes, error hints, ...)
-   [1904](https://github.com/jomjol/AI-on-the-edge-device/pull/1904) Removed newlines in JSON and replaced all whitespaces where there was more than one

#### Fixed

-   Fixed many many things
-   [1509](https://github.com/jomjol/AI-on-the-edge-device/pull/1509) Protect `wifi.ini` from beeing deleted.
-   [1530](https://github.com/jomjol/AI-on-the-edge-device/pull/1530) Homeassistant `Problem Sensor`
-   [1518](https://github.com/jomjol/AI-on-the-edge-device/pull/1518) JSON Strings
-   [1817](https://github.com/jomjol/AI-on-the-edge-device/pull/1817) DataGraph: datafiles sorted -> newest on top

#### Removed

-   n.a.

## [13.0.8] - 2022-12-19

**Name: Home Assistant MQTT Discovery Support**

### Update Procedure see [online documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Installation/#update-ota-over-the-air)

### Added

-   Implementation of [Home Assistant MQTT Discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery)
-   Improved ROIs configuration: locked ROI geometry, equidistant delta x
-   Improved OTA Update mechanism (only working after installation for next update)
-   Added data logging in `/log/data` - One day per file and each measurement is on one line
    -   Format: csv - comma separated
    -   Content: `time`, `name-of-number`, `raw-value`, `return-value`, `pre-value`, `change-rate`, `change-absolute`, `error-text`, `cnn-digital`, `cnn-analog`
-   Show graph of values direct in the user interface (thanks to [@rdmueller](https://github.com/rdmueller))

    -   Using new data logging (see above)
    -   Possibility to choose different values and switch between different numbers (if present)

    Note: You need to activate data logging for this feature to work, see above!
-   PreValue is now contained in `/json` ([#1154](https://github.com/jomjol/AI-on-the-edge-device/issues/1154))
-   SD card info into the `System>Info` menu (thanks to [@Slider007](https://github.com/Slider0007))
-   Version check (Firmware vs. Web UI)
-   Various minor new features

### Changed

-   Updated tflite (`dig-cont_0600_s3.tflite`)
-   Updated OTA functionality (more robust, but not fully bullet prove yet)
-   Updated Espressif library to `espressif32@v5.2.0`
-   [#1176](https://github.com/jomjol/AI-on-the-edge-device/discussions/1176) accept minor negative values (-0.2) if extended resolution is enabled
-   [#1143](https://github.com/jomjol/AI-on-the-edge-device/issues/1143) added config parameter `AnalogDigitalTransitionStart`. It can setup very early and very late digit transition starts.
-   New version of `dig-class100` (v1.4.0): added images of heliowatt powermeter 
-   NEW v13.0.2: Update Tool "Logfile downloader and combiner" to handle the new csv file format.
-   NEW v13.0.2: MQTT: Added MQTT topic `status` (Digitalization Status), Timezone to MQTT topic `timestamp`.#
-   NEW v13.0.2: Logging: Disable heap logs by default, cleanup
-   NEW v13.0.7:
    -   log NTP server name
    -   Improved log messages
    -   Various preparations for next release
-   **NEW v13.0.8**: 
    -   Continue booting on PSRAM issues, Web UI will show an error
    -   Updated models
    -   Various UI enhancements
    -   Various internal improvements
    -   Show uptime in log
    -   Show uptime and round on overview page

### Fixed

-   [#1116](https://github.com/jomjol/AI-on-the-edge-device/issues/1116) precision problem at setting prevalue
-   [#1119](https://github.com/jomjol/AI-on-the-edge-device/issues/1119) renamed `firmware.bin` not working in OTA
-   [#1143](https://github.com/jomjol/AI-on-the-edge-device/issues/1143) changed postprocess for `analog->digit` (lowest digit processing)
-   [#1280](https://github.com/jomjol/AI-on-the-edge-device/issues/1280) check ROIs name for unsupported characters
-   [#983](https://github.com/jomjol/AI-on-the-edge-device/issues/983) old log files did not get deleted 
-   Failed NTP time sync during startup gets now retried every round if needed
-   Whitespaces and `=` in MQTT and InfluxDB passwords
-   Various minor fixes and improvements
-   NEW v13.0.2: Corrected Version comparison between firmware and Web UI.
-   NEW v13.0.3: Re-updated build environment to v5.2.0 (from accidental downgrad to v4.4.0)
-   NEW v13.0.4: Fix for reboot in case of MQTT not used
-   NEW v13.0.5: No reboot in case of missing NTP-connection
-   NEW v13.0.7:
    -   Prevent autoreboot on cam framebuffer init error
    -   Properly protect `wlan.ini` against deletion
    -   Fixed various MQTT topic content issues
    -   Fix Digit detected as 10 (<https://github.com/jomjol/AI-on-the-edge-device/pull/1525>)
    -   Fix frozen time in datafile on error
    -   Various minor fixes
-   **NEW v13.0.8**: 
    -   Fix Rate Problem ([#1578](https://github.com/jomjol/AI-on-the-edge-device/issues/1578), [#1572](https://github.com/jomjol/AI-on-the-edge-device/issues/1572))
    -   Stabilized MQTT
    -   Fixed redundant calls in OTA
    -   Block REST API calls till resource is ready
    -   Fixed number renaming ([#1635](https://github.com/jomjol/AI-on-the-edge-device/issues/1635))

### Removed

-   n.a.

## [12.0.1] 2022-09-29

Name: Improve **u**ser e**x**perience 

:bangbang: The release breaks a few things in ota update :bangbang:

**Make sure to read the instructions below carfully!**.

1.  Backup your configuration (use the `System > Backup/Restore` page)!
2.  You should update to `11.3.1` before you update to this release. All other migrations are not tested. 
    Rolling newer than `11.3.1` can also be used, but no guaranty.
3.  Upload and update the `firmware.bin` file from this release. **but do not reboot**
4.  Upload the `html-from-11.3.1.zip` in html upload and update the web interface.
5.  Now you can reboot.

If anything breaks you can try to
1\. Call `http://<IP>/ota?task=update&file=firmware.bin` resp. `http://<IP>/ota?task=update&file=html.zip` if the upload successed but the extraction failed.
1\. Use the initial_esp32_setup.zip ( <https://github.com/jomjol/AI-on-the-edge-device/wiki/Installation> ) as alternative.

### Added

-   Automatic release creation
-   Newest firmware of rolling branch now automatically build and provided in [Github Actions Output](https://github.com/jomjol/AI-on-the-edge-device/actions) (developers only)
-   [#1068](https://github.com/jomjol/AI-on-the-edge-device/issues/1068) New update mechanism: 
    -   Handling of all files (`zip`, `tfl`, `tflite`, `bin`) within in one common update interface
    -   Using the `update.zip` from the [Release page](https://github.com/jomjol/AI-on-the-edge-device/releases)
    -   Status (`upload`, `processing`, ...) displayed on Web Interface
    -   Automatical detection and suggestion for reboot where needed (Web Interface uupdates only need a page refresh)
    -   :bangbang: Best for OTA use Firefox. Chrome works with warnings. Safari stuck in upload.

### Changed

-   Integrated version info better shown on the Info page and in the log
-   Updated menu
-   Update used libraries (`tflite`, `esp32-cam`, `esp-nn`, as of 20220924) 

### Fixed

-   [#1092](https://github.com/jomjol/AI-on-the-edge-device/issues/1092) censor passwords in log outputs 
-   [#1029](https://github.com/jomjol/AI-on-the-edge-device/issues/1029) wrong change of `checkDigitConsistency` now working like releases before `11.3.1` 
-   Spelling corrections (**[cristianmitran](https://github.com/cristianmitran)**) 

### Removed

-   Remove the folder `/firmware` from GitHub repository. 
    If you want to get the latest `firmware.bin` and `html.zip` files, please download from the automated [build action](https://github.com/jomjol/AI-on-the-edge-device/actions) or [release page](https://github.com/jomjol/AI-on-the-edge-device/releases)

## [11.3.1](https://github.com/jomjol/AI-on-the-edge-device/releases/tag/v11.3.1), 2022-09-17

Intermediate Digits

-   **ATTENTION**: 

    -   first update the `firmware.bin` and ensure that the new version is running

    -   Only afterwards update the `html.zip`

    -   Otherwise the downwards compatibility of the new counter clockwise feature is not given and you end in a reboot loop, that needs manual flashing!


-   **NEW v11.3.1**: corrected corrupted asset `firmware.bin`
-   Increased precision (more than 6-7 digits)
-   Implements Counter Clockwise Analog Pointers
-   Improved post processing algorithm
-   Debugging: intensive use of testcases
-   MQTT: improved handling, extended logging, automated reconnect
-   HTML: Backup Option for Configuration
-   HTML: Improved Reboot
-   HTML: Update WebUI (Reboot, Infos, CPU Temp, RSSI)
-   This version is largely also based on the work of **[caco3](https://github.com/caco3)**,  **[adellafave](https://github.com/adellafave)**,  **[haverland](https://github.com/haverland)**,  **[stefanbode](https://github.com/stefanbode)**, **[PLCHome](https://github.com/PLCHome)**

## [11.2.0](https://github.com/jomjol/AI-on-the-edge-device/releases/tag/v11.2.0), 2022-08-28

Intermediate Digits

-   Updated Tensorflow / TFlite to newest tflite (version as of 2022-07-27)

-   Updated analog neural network file (`ana-cont_11.3.0_s2.tflite` - default, `ana-class100_0120_s1_q.tflite`)

-   Updated digital neural network file (`dig-cont_0570_s3.tflite` - default, `dig-class100_0120_s2_q.tflite`)

-   Added automated filtering of tflite-file in the graphical configuration (thanks to @**[caco3](https://github.com/caco3)**)

-   Updated consistency algorithm & test cases

-   HTML: added favicon and system name, Improved reboot dialog  (thanks to @**[caco3](https://github.com/caco3)**)

## [11.1.1](https://github.com/jomjol/AI-on-the-edge-device/releases/tag/v11.1.1), 2022-08-22

Intermediate Digits

-   New and improved consistency check (especially with analog and digital counters mixed)
-   Bug Fix: digital counter algorithm

## [11.0.1](https://github.com/jomjol/AI-on-the-edge-device/releases/tag/v11.0.1), 2022-08-18

Intermediate Digits

-   **NEW v11.0.1**: Bug Fix InfluxDB configuration (only update of html.zip necessary)

-   Implementation of new CNN types to detect intermediate values of digits with rolling numbers

    -   By default the old algo (0, 1, ..., 9, "N") is active (due to the limited types of digits trained so far)
    -   Activation can be done by selection a tflite file with the new trained model in the 'config.ini'
    -   **Details can be found in the [wiki](https://github.com/jomjol/AI-on-the-edge-device/wiki/Neural-Network-Types)** (different types, trained image types, naming convention)

-   Updated  neural network files (and adaption to new naming convention)

-   Published a tool to download and combine log files - **Thanks to **

    -   Files see ['/tools/logfile-tool'](tbd), How-to see [wiki](https://github.com/jomjol/AI-on-the-edge-device/wiki/Gasmeter-Log-Downloader)

-   Bug Fix: InfluxDB enabling in grahic configuration

## [10.6.2](https://github.com/jomjol/AI-on-the-edge-device/releases/tag/v10.6.2), 2022-07-24

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

## [10.5.2](https://github.com/jomjol/AI-on-the-edge-device/releases/tag/v10.5.2), 2022-02-22

Stability Increase

### Changed

-   NEW 10.5.2: Bug Fix: wrong `firmware.bin` (no rate update)
-   NEW 10.5.1: Bug Fix: wrong return value, rate value & PreValue status, HTML: SSID & IP were not displayed
-   MQTT: changed wifi naming to "wifiRSSI"
-   HTML: check selectable values for consistency
-   Refactoring of check postprocessing consistency (e.g. max rate, negative rate, ...)
-   Bug Fix: corrected error in "Check Consistency Increase"

## [10.4.0](https://github.com/jomjol/AI-on-the-edge-device/releases/tag/v10.4.0), 2022-02-12

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


[15.2.4]: https://github.com/jomjol/AI-on-the-edge-device/compare/v15.2.1...v15.2.4
[15.2.1]: https://github.com/jomjol/AI-on-the-edge-device/compare/v15.2.0...v15.2.1
[15.2.0]: https://github.com/jomjol/AI-on-the-edge-device/compare/v15.1.1...v15.2.0
[15.1.1]: https://github.com/jomjol/AI-on-the-edge-device/compare/v15.1.0...v15.1.1
[15.1.0]: https://github.com/jomjol/AI-on-the-edge-device/compare/v15.0.3...v15.1.0
[15.0.3]: https://github.com/jomjol/AI-on-the-edge-device/compare/v14.0.3...v15.0.3
[14.0.3]: https://github.com/jomjol/AI-on-the-edge-device/compare/v13.0.8...v14.0.3
[13.0.8]: https://github.com/jomjol/AI-on-the-edge-device/compare/v12.0.1...v13.0.8
[13.0.7]: https://github.com/jomjol/AI-on-the-edge-device/compare/v12.0.1...v13.0.7
[13.0.5]: https://github.com/jomjol/AI-on-the-edge-device/compare/v12.0.1...v13.0.5
[13.0.4]: https://github.com/jomjol/AI-on-the-edge-device/compare/v12.0.1...v13.0.4
[13.0.1]: https://github.com/jomjol/AI-on-the-edge-device/compare/v12.0.1...v13.0.1
[12.0.1]: https://github.com/jomjol/AI-on-the-edge-device/compare/v11.3.1...v12.0.1
[11.4.3]: https://github.com/haverland/AI-on-the-edge-device/compare/v10.6.2...v11.4.3
[11.4.2]: https://github.com/haverland/AI-on-the-edge-device/compare/v10.6.2...v11.4.2
[11.3.9]: https://github.com/haverland/AI-on-the-edge-device/compare/v10.6.2...v11.3.9
