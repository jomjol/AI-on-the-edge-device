# Configuration Parameter Details

### [MakeImage]

```
[MakeImage]
LogImageLocation = /log/source
WaitBeforeTakingPicture = 5
LogfileRetentionInDays = 15
Brightness = -2
;Contrast = 0
;Saturation = 0
ImageQuality = 5
ImageSize = VGA
FixedExposure = true
```


| Parameter               | Meaning                                                      | Options/Examples |
| ----------------------- | ------------------------------------------------------------ | ---------------- |
| LogImageLocation        | location for storing a copy of the image                     |                  |
| WaitBeforeTakingPicture | waiting time between switch on the flash-light and taking the image in seconds |                  |
| LogfileRetentionInDays  | Number of days, for which the log files should be stored     | 0 = keep forever |
| Brightness              | Adjustment of the camera brightness (-2 ... 2)               |                  |
| Contrast                | NOT IMPLEMENTED                                              |                  |
| Saturation              | NOT IMPLEMENTED                                              |                  |
| ImageQuality            | Input image jpg-compression quality 0 (best) to 100 (lowest) | 5 = default      |
| ImageSize               | Input Image Size from Camera                                 | only VGA, QVGA   |
| FixedExposure           | If enabled, the exposure settings are fixed at the beginning and the waiting time after switching on the illumination will be skipped |                  |



### [Alignment]

```
[Alignment]
InitialRotate = 179
InitialMirror = false
SearchFieldX = 20
SearchFieldY = 20
AlignmentAlgo = Default
FlipImageSize = false
/config/ref0.jpg 104 271
/config/ref1.jpg 442 142
```

| Parameter                | Meaning                                                      | Options/Examples                      |
| ------------------------ | ------------------------------------------------------------ | ------------------------------------- |
| InitialMirror            | Option for initially mirroring the image on the original x-axis |                                       |
| InitialRotate            | Initial rotation of image before alignment in degree (1...359) |                                       |
| FlipImageSize            | Changes the aspect ratio after the image rotation to avoid cropping of the rotated imaged |                                       |
| /config/refx.jpg 98, 257 | Link to reference image and corresponding target coordinates | file link is relative to sd-card root |
| SearchFieldX/Y           | Search field size in X/Y for finding the reference images [pixel] |                                       |

Here two reference images are needed. Therefore rotation and shifting can be compensated. As the alignment is one of the most computing time using part, the search field needs to be limited. The calculation time goes quadratic with the search field size.

### [Digits]

```
[Digits]
Model=/config/digits.tfl
ModelInputSize 20, 32
LogImageLocation = /log/digit
LogfileRetentionInDays = 2
number1.digit1 292 120 37 67
number1.digit2 340 120 37 67
number1.digit3 389 120 37 67
number2.digit1 292 180 37 67
number2.digit2 340 180 37 67
```


| Parameter              | Meaning                                                      | Options/Examples                           |
| ---------------------- | ------------------------------------------------------------ | ------------------------------------------ |
| Model                  | Link to the CNN-tflite file used for AI-image recognition    |                                            |
| ModelInputSize         | Image input size for the CNN-Network [pixel]                 | needed to resize the ROI to the input size |
| LogImageLocation       | storage location for the recognized images, including the CNN-results in the file name/location |                                            |
| numberX.digitY         | ROI for the corresponding digit in the aligned image. <br />More than one number can be specified. Therefore the name consists of a naming of the number (`numberX`) and the region of interest (`digitY`) - separated by `.` |                                            |
| LogfileRetentionInDays | Number of days, for which the log files should be stored     | 0 = keep forever                           |
| LogImageLocation       | location for storing a copy of the image                     |                                            |


### [Analog]

```
[Analog]
Model=/config/analog.tfl
ModelInputSize 32, 32
LogImageLocation = /log/analog
LogfileRetentionInDays = 2
number1.analog1, 433, 207, 99, 99
number1.analog2, 378, 313, 99, 99
number1.analog3, 280, 356, 99, 99
number1.analog4, 149, 313, 99, 99
number2.analog1, 280, 456, 99, 99
number2.analog2, 149, 413, 99, 99
```

Same as for [digit], here only for the analog pointers

### [PostProcessing]

```
[PostProcessing]
number1.DecimalShift = 0
number2.DecimalShift = -1
PreValueUse = true
PreValueAgeStartup = 720
AllowNegativeRates = false
number1.MaxRateValue = 0.1
number2.MaxRateValue = 0.1
ErrorMessage = true
CheckDigitIncreaseConsistency = false
```

Here the post processing and consistency check for the readout can be adjusted

| Parameter                     | Meaning                                                      | Options/Examples                                             |
| ----------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| PreValueUse                   | Use the previous value for consistency check and substitution for NaN (True / False) |                                                              |
| PreValueAgeStartup            | Max age of PreValue after a reboot (downtime)                |                                                              |
| AllowNegativeRates            | Allow decrease of the readout value                          |                                                              |
| numberX.MaxRateValue          | Maximum chance rate from one to the next readout.<br />This can be specified for each number individual. |                                                              |
| ErrorMessage                  | Show error messages                                          |                                                              |
| numberX.DecimalShift          | Shifting of the decimal separator from the default position between digital and analog.<br />This can be specified for each number individual. | DecimalShift = 2: 123.456 --> 12345.6<br />DecimalShift = -1: 123.456 --> 12.3456<br/> |
| CheckDigitIncreaseConsistency | This parameter controls, if the digits are checked for a consistent change in comparison to the previous value. This only makes sense, if the last digit is changing very slowly and every single digit is visible (e.g. 4.7 --> 4.8 --> 4.9 --> 5.0 --> 5.1). If single digits are skipped, for example because the digits changes to fast, this should be disabled (e.g. 4.7 --> 5.0 --> 5.1). |                                                              |

### [MQTT]

```
[MQTT]
Uri = mqtt://IP-ADRESS:1883
MainTopic = wasserzaehler
ClientID = wasser
user = USERNAME
password = PASSWORD
```

Here the post processing and consistency check for the readout can be adjusted

| Parameter | Meaning                                                      | Options/Examples |
| --------- | ------------------------------------------------------------ | ---------------- |
| Uri       | URI to the MQTT broker including port e.g.: mqtt://IP-Address:Port |                  |
| MainTopic | MQTT main topic, under which the counters are published. <br />The single  value will be published with the following key:  `MainTopic/number/PARAMETER` where parameters are: value, rate, timestamp, error and json<br/>The general connection status can be found in `MainTopic/connection` |                  |
| ClientID  | ClientID to connect to the MQTT broker                       |                  |
| user      | user for MQTT authentication                                 | (optional)       |
| password  | password for MQTT authentication                             | (optional)       |


### [AutoTimer]

```
[AutoTimer]
AutoStart= true
Intervall = 4.85
```

This paragraph is used to automatically trigger the periodic automated readout.



| Parameter | Meaning                                         | Options/Examples                                             |
| --------- | ----------------------------------------------- | ------------------------------------------------------------ |
| AutoStart | Automatically trigger the readout after startup |                                                              |
| Intervall | Readout interval in minutes                     | Values smaller than 2 minutes do not make sense, as this is the time for one detection |

### [Debug]

```
[Debug]
Logfile = true
LogfileRetentionInDays = 2
```

This paragraph is used to switch on an extended logging. It is optional and by default only a minimum logging is enabled.
**Attention:** in case of extended logging the size of the log file (`/log.txt`, `/alignment.txt`) might increase rapidly, therefore manually deletion from time to time is recommended



| Parameter              | Meaning                                                  | Options/Examples               |
| ---------------------- | -------------------------------------------------------- | ------------------------------ |
| Logfile                | Turn on (true) or off (false) the extended logging       | parameter and section optional |
| LogfileRetentionInDays | Number of days, for which the log files should be stored | 0 = keep forever               |

### [System]

```
[System]
TimeZone = CET-1CEST,M3.5.0,M10.5.0/3
;TimeServer = TIMESERVER
;Hostname = undefined
SetupMode = false
```

This paragraph is used to switch on an extended logging. It is optional and by default only a minimum logging is enabled.
**Attention:** in case of extended logging the size of the log file (`/log.txt`, `/alignment.txt`) might increase rapidly, therefore manually deletion from time to time is recommended



| Parameter  | Meaning                                                      | Options/Examples                                             |
| ---------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| TimeZone   | TimeZone of the system can be specified                      | Central european, with summertime adjustement: `CET-1CEST,M3.5.0,M10.5.0/3` |
| TimeServer | An dedicated time server can be specified.                   | default = `pool.ntp.org`                                     |
| Hostname   | Additionally to the `wlan.ini` the hostname can be specified. It will be transferred to the `wlan.ini` and initiate a reboot |                                                              |
| SetupMode  | If enabled, the server starts in an initial setup mode. This is automatically disabled at the end of the setup |                                                              |


### [Ende]

No function, just to mark, that the config is done!