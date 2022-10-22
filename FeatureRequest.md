## Feature Requests

**There are a lot of ideas for further improvements, but only limited capacity on side of the developer.** Therefore I have created this page as a collection of ideas. 

1. Who ever has a new idea can put it here, so it that it is not forgotten. 

2. Who ever has time, capacity and passion to support, can take any of the ideas and implement them. 
   I will support and help where ever I can!
   
   

____

#### #32 Add feature to correct misinterpreted value

* If a value is misinterpreted, the user can manually correct the value.
* The misinterpreted ROIs would be saved in a "training data" -folder on the SD-card
* Stretch goal: make sending of saved training data as easy as pushing a button =)

#### #31 Implement InfluxDB v2.x interface

* Currently only InfluxDB v1.x is supportet, extend to v2.x
* Remark: interface has changed
* see [#1160](https://github.com/jomjol/AI-on-the-edge-device/issues/1160)

#### #30 Support meter clock over

* In case of meter clocking over, that is, reaching its max. value and starting over from 0,
  accept the new value and calculate correctly the difference.
  (see line 739 onwards in ClassFlowPostProcessing.cpp)


#### #28 Improved error handling for ROIs

* In case a ROI is out of the image, there is no error message, but a non sense image is used
* Implement a error message for wrong configuratioin of ROI

#### #27 Use Homie Spec for Mqtt binding

* Use the standardized Home Protocol for the Mqtt binding 
* https://homieiot.github.io/

#### #26 Changes behaviour for "N" replacement

* in case the higher digits has already increased by minium 1 - don't set the "N" to the last value, but to "0"
* https://github.com/jomjol/AI-on-the-edge-device/issues/792


#### #25 Trigger Measurement via MQTT

* https://github.com/jomjol/AI-on-the-edge-device/issues/727


#### #24 Show Mqtt state directly in Webserver

* Show MQTT log in Web page. E.g. connection established or failed to connect...

  


#### #23 CPU Temp and Mqtt values

* Show the CPU Temp directly in Webpage. Also add the value to MQTT sending

    

#### #21 Extended "CheckDigitalConsistency" Logik

* https://github.com/jomjol/AI-on-the-edge-device/issues/590

  


#### #16 Serial Communication

* https://github.com/jomjol/AI-on-the-edge-device/issues/512

* Send the readout value via RX/TX interface with a dedicated TAG

* Make dedicated communication FlowModule

* Modification of RX/TX communication

* Configuration interfache

  


#### #15 Calibration for FishEye image

* https://github.com/jomjol/AI-on-the-edge-device/issues/507

1.  The development of such a correction algorithm with the libraries, that are available for the ESP32 environment.
2. New module for integration of the flow into the image processing flow.
3. Extension of the configuration (config.ini) and html-pages
4. Parameter adjustment and testing for every different fish-eye module
5. Maintenance for further updates / modules, ...



#### #13 Manage non linear gauge without CNN re-training

* https://github.com/jomjol/AI-on-the-edge-device/issues/443

* Implement a look up table for non linear analog meters

  

#### #11 MQTT - configurable payload

* https://github.com/jomjol/AI-on-the-edge-device/issues/344

  

#### #10 Improve and bug fix logging of images

* https://github.com/jomjol/AI-on-the-edge-device/issues/307

  

#### #8 MQTT configurable readout intervall

Make the readout intervall configurable via MQTT.

* Change the mqtt part to receive and process input and not only sending



#### #7 Extended Error Handling

Check different types of error (e.g. tflite not availabe) and generate an error on the html page.

To do:

* Make a list of "important" errors
* Implement a checking algo
* Extend the firmware and html page for the error handling



#### #5 Configurable decimal separator (point or comma) 

Decimal separator configurable for different systems

To do:

* Implementation of decimal point into postprocessing module
* Extension of configuration
* Adaption of the html configuration to implement shifting



#### #2 MQTT-controll with callback 
* https://github.com/jomjol/AI-on-the-edge-device/issues/105

Extend the MQTT client to also enable callbacks for configuration setting

To do:

* implement callback for receiving information and override `config.ini` settings

* change configuration management to handle online updates (currently changes need a restart)

* think about the startup, as there the default config is loaded 

  

____



# Refused

#### #9 Basic auth for the UI 

* https://github.com/jomjol/AI-on-the-edge-device/issues/283

* Implementation of an authentication mechanism.

This will not be part of the free version



#### #20 Deep sleep and push mode

* Let the device be normally in deep sleep state, and wake it up periodically to collect data and push it via MQTT or HTTP post.
* Support ESP-NOW to reduce the overhead of connecting to wifi and mqtt 
* the above should enable battery powered applications

* An other way to set deep sleep would be to enable it in a specific period (at night).

Technically not possible, as the limiting factor for the power consumption is not the esp32, but the electrical curcuit on the ESP32CAM board, which consumes a lot already in deep sleep mode.



# Closed


#### ~~#29 Add favicon and use the hostname for the website~~- implemented v11.3.1



#### ~~#19 Extended log informations~~

~~* https://github.com/jomjol/AI-on-the-edge-device/issues/580~~

  

#### ~~#29 Add favicon and use the hostname for the website~~- implemented v11.3.1

~~* https://github.com/jomjol/AI-on-the-edge-device/issues/927~~



#### ~~#22 Direct hint to the different neural network files in the other repositories~~- implemented >v11.3.1

~~* https://github.com/jomjol/AI-on-the-edge-device/issues/644~~



#### ~~#18 Document WLAN-strength in web page~~

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/563~~



#### ~~#17 Direct InfluxDB connection~~

* ~~Done in v10.6.0~~



#### ~~#14 Backup and restore option for configuration~~- implemented v11.3.1

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/459~~

* ~~Implement a zip file compression for store and restore~~

* ~~Update the html to handle it~~

  

#### ~~#12 Less reboots due to memory leakage~~

* ~~Issue: #414 & #425  #430~~



#### ~~#6 Check for double ROI names~~ - implemented v8.0.0

~~Check during configuration, that ROI names are unique.~~

~~To do:~~

* ~~Implementation of ROI name checking in html code before saving analog or digital ROIs~~

  

#### ~~#4 Initial Shifting and Rotation~~ - implemented v7.0.0

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/123~~

~~Implementation of a shifting additional to the initial rotation of the raw camera input~~

~~To do:~~

* ~~Implementation of shifting~~
* ~~Extension of configuration~~
* ~~Adaption of the html configuration to implement shifting~~



#### ~~#3 Allow grouping of digits to multiple reading values~~ - implemented v8.0.0

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/123~~

~~Implementation of two different independent readouts in one setup~~

~~To do:~~

* ~~Extend the configuration, setting and processing flow for two independend readouts~~



#### ~~#1 Optional GPIO for external flash/lighting~~ - implemented (v8.0.0)

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/133~~

~~Implementation of an an extrnal flash / lightning through GPIOs.~~

* ~~available GPIOs: 12 & 13 (currently in use for html switching)~~

~~To do:~~

* ~~Implementation of a software module for external light source (e.g. WS8132 LED controller, ...)~~
* ~~Update of the camera module to use the external light instead of the internal flash light~~
* ~~Adopt the configuration algorithm with a configurable light source~~



