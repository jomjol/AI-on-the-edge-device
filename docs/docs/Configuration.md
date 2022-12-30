Most of the settings can be modified with the help of a web based [graphical user interface](Graphical-configuration). This is hosted through the web server on the ESP32.

More configuration parameters can be edited by hand in the `config.ini` and corresponding files in the `/config` directory on the SD-card. 



If you where using the Version 1 of the watermeter you can easily transfer the configuration to the new system by following the steps in this [migration description](MigrateOldConfigToNew.md)



## Processing / Config.ini principle

The principle is very simple and can most easily be described as a flow of processing steps. Each step has a dedicated parameter description in the ``config.ini``, which is indicated by brackets ```[name_of_step]```. The steps are processed in the order written in the config file. That means, that you first have to describe the image taking, then the aligning and cutting and only after that you can start to config a neural network. The last step is the post processing.

###  Processing steps - Overview

In the following you get a short overview over the available steps. This order is also the suggested order for the processing flow. Single steps can be left out, if not needed (e.g. omit the analog part, if only digits are present)

#### 1. ``[MakeImage]``

* This steps parametrises the taking of the image by the ESP32-CAM. Size, quality and storage for logging and debugging can be set.

#### 2. ``[Alignment]``
* Image preprocessing, including image alignment with reference images

#### 3. ``[Digits]``

* Neural network evaluation of an image for digits. The neural network is defined by a tflite formatted file and the output is a number between 0 .. 9 or NaN (if image is not unique enough)

#### 4. ``[Analog]``
- Neural network evaluation of analog counter. The neural network is defined by a tflite formatted file and the output is a number between 0.0 .. 9.9, representing the position of the pointer.


#### 5. ``[PostProcessing]``
- Summarized the individually converted pictures to the overall result. It also implements some error corrections and consistency checks to filter wrong reading.

#### 6. ``[MQTT]``

  - Transfer of the readings to a MQTT server.


#### 7. ``[AutoTimer]``
- Configuration of the automated flow start at the start up of the ESP32. 

#### 8. ``[Debug]``
- Configuration for debugging details

#### 9. ``[Ende]``
- No meaning, just an additional indication, that the configuration is finished.

  

**A detailed parameter description can be found here: [[Configuration Parameter Details]].**



## Graphical configuration interface

It is recommended to do the configuration of the alignment structures and ROIs through the graphical user interface. A step by step instruction can be found here: [[Graphical Configuration]]



## Background for Image Alignment

Details on the image recognition flow can be found in the other project here: https://github.com/jomjol/water-meter-system-complete/blob/master/images/Alignment_procedure_draft.pdf

The ```config.ini``` here has the same functionality and options, but a slightly different syntax due to a own written ini-parser is used. Migration see [here](MigrateOldConfigToNew.md).



### Integration into Home Assistant

Thanks to the help of the user @deadly667 here are some hints for the integration into the home assistant: [[Integration-Home-Assistant]]

