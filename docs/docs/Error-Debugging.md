# Error Debugging

## Rebooting

##### General Remark 

1. Due to the rather complex code with a lot of external libraries and the limited availability of memory a reboot of the device from time to time is "normal". Background are memory leakages and therefore running out of free memory. 

2. The hardware of the ESP32CAM has a varying quality. I have one and the same hardware with a reboot range from every 5 detection runs to up to 250 detection runs. 

##### Getting deeper inside

Have a look into the log file (``/log/message/...``). 

* If the log file is very short you need to enable a enhanced logging in the ``config.ini`` (Debug --> ``logfile = true``) . 



Analyze the debugging output of the serial interface

* Connect a serial to USB interface (like for flashing) and make a logging of the serial communication
* There are a lot more intermediate information and the lines before the reboot tell you, where the firmware fails



**If you make an issue about this, please post these two information additionally** 

**Don't forget to remove your WLAN password in the serial log**



## Often observed problems

### Hardware failure
  * Camera not working --> check the interface, test another module
  * Low cost module with only 2MB of PSRAM instead of 4MB --> image taking will fail first. This will never work due to too low memory

### ROI misaligned

<img src="https://user-images.githubusercontent.com/108122193/188264361-0f5038ce-d827-4096-93fb-5907d3b072b4.png" width=30% height=30%>

This typically happens if you have suboptimal "Alignement Marks". A very simple and working solution is to put put higly contrasted stickers on your meter and put "Alignement Marks" on it (see picture below)

<img src="https://user-images.githubusercontent.com/108122193/188264752-c0f2a2be-0c22-40de-afaf-fd55b2eb4182.png" width=30% height=30%>

If after those adjustement you still have some issues, you can try to adjust your aligmenet settings in expert mode:
<img src="https://user-images.githubusercontent.com/108122193/188382213-68c4a015-6582-4911-81bc-cdce8ef60ed2.png" width=75% height=75%>


### My Analog Meter are recognized as Digital Counter or vice versa 

<img src="https://user-images.githubusercontent.com/108122193/188265470-001a392f-d1f4-46a3-b1e8-f29ec41c8621.png" width=40% height=40%>


1. First, check that your ROI are correctly defined (yey!)
2. Second, verify that the name of your ROI analog and digital ROIs are different 

### Recognition is working well, but number aren't sorted correctly

You have to sort your ROI correctly (Bigger to smaller). Select your ROI and click either "move next" or "move previous". Repeat until your ROI are correctly sorted

<img src="https://user-images.githubusercontent.com/108122193/188264916-03befff1-4e61-4370-bd5a-9168a88c57f2.png" width=50% height=50%>
