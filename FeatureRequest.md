## Feature Requests

**There are a lot of ideas for further improvements, but only limited capacity on side of the developer. **Therefore I have created this page as a collection of ideas. 

1. Who ever has a new idea can put it here, so it that it is not forgotten. 

2. Who ever has time, capacity and passion to support, can take any of the ideas and implement them. 
   I will support and help where ever I can!
   
   

____

#### #3 Allow grouping of digits to multiple reading values

* https://github.com/jomjol/AI-on-the-edge-device/issues/123

Implementation of two different independent readouts in one setup

To do:

* Extend the configuration, setting and processing flow for two independend readouts


#### 

https://github.com/jomjol/AI-on-the-edge-device/issues/123



____

#### #2 MQTT-controll with callback 
* https://github.com/jomjol/AI-on-the-edge-device/issues/105

Extend the MQTT client to also enable callbacks for configuration setting

To do:

* implement callback for receiving information and override `config.ini` settings

* change configuration management to handle online updates (currently changes need a restart)

* think about the startup, as there the default config is loaded 

  

____

#### #1 Optional GPIO for external flash/lighting

* https://github.com/jomjol/AI-on-the-edge-device/issues/133

Implementation of an an extrnal flash / lightning through GPIOs.
* available GPIOs: 12 & 13 (currently in use for html switching)

To do:

* Implementation of a software module for external light source (e.g. WS8132 LED controller, ...)
* Update of the camera module to use the external light instead of the internal flash light
* Adopt the configuration algorithm with a configurable light source