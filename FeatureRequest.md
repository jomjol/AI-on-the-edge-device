## Feature Requests

**There are a lot of ideas for further improvements, but only limited capacity on side of the developer.** Therefore I have created this page as a collection of ideas. 

1. Whoever has a new idea can put it here, so that it is not forgotten. 

2. Whoever has the time, capacity and passion to support the project can take any of the ideas and implement them. I will provide support and help wherever I can!
   
   

____


#### #40 Trigger with cron like exact time slot

* https://github.com/jomjol/AI-on-the-edge-device/issues/2470



#### #39 upnp implementation to auto detect the device

* https://github.com/jomjol/AI-on-the-edge-device/issues/2481

  

#### #38 Energy Saving

* Deep sleep between recognition
* https://github.com/jomjol/AI-on-the-edge-device/issues/2486



#### #37 Auto init SD card

* Fully implement the SD card handling (including formatting) into the firmware
* https://github.com/jomjol/AI-on-the-edge-device/issues/2488Demo 

#### #36 Run demo without camera

Demo mode requires a working camera (if not, one receives a 'Cam bad' error). Would be nice to demo or play around on other ESP32 boards (or on ESP32-CAM boards when you broke the camera cable...).

#### #35 Use the same model, but provide the image from a Smartphone Camera
as reading the Electricity or Water meter every few minutues only delivers apparent accuracy (DE: "Scheingenauigkeit") you could just as well take a picture with your Smartphone evey so often (e.g. once a week when you are in the Basement anyway), then with some "semi clever" tricks pass this image to the model developed here, and the values then on to who ever needs them e.g. via MQTT.
IMO: It is not needed to have that many readings (datapoints) as our behaviour (Use of electricity or water) doesn't vary that much, say, over a weeks time. The interpolation between weekly readings will give sufficient information on the power and/or water usage. 


#### #34 implement state and Roi for water leak detection
for example see Roi on the next picture..
![grafik](https://user-images.githubusercontent.com/38385805/207858812-2a6ba41d-1a8c-4fa1-9b6a-53cdd113c106.png)
in case of position change between the measurments set this state to true, if there is no change set it back to false.
In a defined time window this movement can lead into an alarm state / water leak..
haveing this state in the mqtt broker can trigger functions like closing the ater pipe walve and so on...



#### #33 Implement MATTER protocoll

* see [#1404](https://github.com/jomjol/AI-on-the-edge-device/issues/1404)

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

#### ~~#29 Add favicon and use the hostname for the website~~- implemented v11.3.1

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/927~~

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

  

#### ~~#22 Direct hint to the different neural network files in the other repositories~~- implemented >v11.3.1

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/644~~

  

#### #21 Extended "CheckDigitalConsistency" Logik

* https://github.com/jomjol/AI-on-the-edge-device/issues/590

  

#### #20 Deep sleep and push mode

* Let the device be normally in deep sleep state, and wake it up periodically to collect data and push it via MQTT or HTTP post.
* Support ESP-NOW to reduce the overhead of connecting to wifi and mqtt 
* the above should enable battery powered applications

* An other way to set deep sleep would be to enable it in a specific period (at night).
  

#### #19 Extended log informations

* https://github.com/jomjol/AI-on-the-edge-device/issues/580

  

#### ~~#18 Document WLAN-strength in web page~~

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/563~~



#### ~~#17 Direct InfluxDB connection~~

* ~~Done in v10.6.0~~


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



#### ~~#14 Backup and restore option for configuration~~- implemented v11.3.1

* ~~https://github.com/jomjol/AI-on-the-edge-device/issues/459~~

* ~~Implement a zip file compression for store and restore~~

* ~~Update the html to handle it~~

  

#### #13 Manage non linear gauge without CNN re-training

* https://github.com/jomjol/AI-on-the-edge-device/issues/443

* Implement a look up table for non linear analog meters

  

#### ~~#12 Less reboots due to memory leakage~~

* ~~Issue: #414 & #425  #430~~

  

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
