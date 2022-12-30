# Migration from water-meter „old“ to water-meter “AI-on-the-edge-device”

 

There are only some few steps necessary to migrate your old system to the new one.

Please follow the following steps:

#### 1. Follow the installation guide to flash the ESP32CAM and prepare a SD-Card with the content of the master

#### 2. Save the following files from the old Docker system on your PC:

* Reference Points 1-3 (only 2 needed)
* `Config.ini`

#### 3. Copy Reference Points 1-3 onto the new water-meter system (Directory `/config`)

**Please note only two Reference Points are supported in the new system.**

#### 4. Open new `config.ini` File:

Insert from the old `Config.ini` file `[alignment]` and `[alignment.ref0]` and `[alignment.ref1]` section the two Ref x and y position and the `initial_rotation_angle= 123`into the new `Config.ini` File, e.g.:

###### Old:
```
[alignment.ref0]
image=./config/RB01_65x65.jpg
pos_x=28
pos_y=63

[alignment.ref1]
image=./config/RB02_50x35.jpg
pos_x=497
pos_y=127

[alignment]
initial_rotation_angle=180
```

###### New:

```
[Alignment]
InitalRotate=180
/config/RB01_65x65.jpg 28, 63
/config/RB02_50x35.jpg 497, 127
SearchFieldX = 20
SearchFieldY = 20
```


#### 5. Insert the old Digit Values into the new `Config.ini` File, e.g.:

###### Old:
```
[Digital_Digit.ziffer1]
pos_x=265
pos_y=117
dx=28
dy=51

[Digital_Digit.ziffer2]
pos_x=310
pos_y=117
dx=28
dy=51

[Digital_Digit.ziffer3]
pos_x=354
pos_y=117
dx=28
dy=51

[Digital_Digit.ziffer4]
pos_x=399
pos_y=117
dx=28
dy=51

[Digital_Digit.ziffer5]
pos_x=445
pos_y=115
dx=28
dy=51
```

###### New:
```
[Digits]
Model=/config/dig0630s3.tflite
;LogImageLocation = /log/digit
ModelInputSize 20, 32
digit1, 265, 117, 28, 51
digit2, 310, 117, 28, 51
digit3, 354, 117, 28, 51
digit4, 399, 117, 28, 51
digit5, 445, 115, 28, 51
```


#### 6. Make sure that you have the same quality and size settings as in your old `Config.ini` 

In the old configuration this was coded in the html-string for the image source:
###### Old:
```
URLImageSource=http://IP-ADRESS/capture_with_flashlight?quality=5&size=VGA
```

Default was Quality=5 and VGA.

###### New:

```
ImageQuality = 5
ImageSize = VGA
```



#### 7. Repeat the same for the analog section

#### 8. Insert your SSID and Password into the new wlan.ini File

#### 9. Compare and edit [ConsistencyCheck] Section with new [PostProcessing] Section

#### 10.  Save new config.ini File in the new System. 

#### 11. Restart the system.

#### 12. After the first start set manually the PreValue in the new system