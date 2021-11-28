## Feature Requests

**There are a lot of ideas for further improvements, but only limited capacity on side of the developer.** Therefore I have created this page as a collection of ideas. 

1. Who ever has a new idea can put it here, so it that it is not forgotten. 

2. Who ever has time, capacity and passion to support, can take any of the ideas and implement them. 
   I will support and help where ever I can!
   
   

____

#### #12 Less reboots due to memory leakage

* Issue: #414 & #425  #430

  

#### #11 MQTT - configurable payload

* https://github.com/jomjol/AI-on-the-edge-device/issues/344

  

#### #10 Improve and bug fix logging of images

* https://github.com/jomjol/AI-on-the-edge-device/issues/307

  

#### #9 Basic auth for the UI

* https://github.com/jomjol/AI-on-the-edge-device/issues/283

* Implementation of an authentication mechanism.

#### #8 MQTT configurable readout intervall

Make the readout intervall configurable via MQTT.

* Change the mqtt part to receive and process input and not only sending

#### #7 Extended Error Handling

Check different types of error (e.g. tflite not availabe) and generate an error on the html page.

To do:

* Make a list of "important" errors
* Implement a checking algo
* Extend the firmware and html page for the error handling

#### ~~#6 Check for double ROI names~~ - implemented v8.0.0

~~Check during configuration, that ROI names are unique.~~

~~To do:~~

* ~~Implementation of ROI name checking in html code before saving analog or digital ROIs~~

  

#### #5 Configurable decimal separator (point or comma) 

Decimal separator configurable for different systems

To do:

* Implementation of decimal point into postprocessing module
* Extension of configuration
* Adaption of the html configuration to implement shifting



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





____

#### #2 MQTT-controll with callback 
* https://github.com/jomjol/AI-on-the-edge-device/issues/105

Extend the MQTT client to also enable callbacks for configuration setting

To do:

* implement callback for receiving information and override `config.ini` settings

* change configuration management to handle online updates (currently changes need a restart)

* think about the startup, as there the default config is loaded 

  

____

#### ~~#1 Optional GPIO for external flash/lighting~~ - implemented (v8.0.0)

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/133~~

~~Implementation of an an extrnal flash / lightning through GPIOs.~~

* ~~available GPIOs: 12 & 13 (currently in use for html switching)~~

~~To do:~~

* ~~Implementation of a software module for external light source (e.g. WS8132 LED controller, ...)~~
* ~~Update of the camera module to use the external light instead of the internal flash light~~
* ~~Adopt the configuration algorithm with a configurable light source~~