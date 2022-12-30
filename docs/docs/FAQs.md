# Frequently Asked Questions

#### My device is reboot frequently. What can I do?

There are several reasons for the reboot:

* Frequent HTML requests
* Wrong configuration, missing configuration files
* Unstable hardware - see [[Hardware Compatibility]]

There is a dedicated Wiki page about this: [[Frequent Reboots]]


#### How accurate are the detections?

It is hard to give a specific accuracy number. It depends on many factors, e.g.

* How in-focus is your camera?
* How sturdy is the camera mount? Does it slightly move over extended periods of time?
* What type of meter are you reading? Is the meter already in the training data set?
* Are you trying to read digits, an analog dial, or both?
* etc.

Anecdotally, the authors of this wiki have great success with the meter. While the AI algorithm itself is not perfect and sometimes returns `NaN` or incorrect values, other post-processing / prevalue / sanity checks help ensure such invalid values are filtered out. With the correct settings, one author has been running this device for 1 month without any incorrect values reported. 

See the FAQs below for more details and configuration hints.


#### My numbers are not corrected detected. What can I do?

* There is a dedicated Wiki page about the correct setting [[ROI Configuration]]
* This page also includes the instructions for gathering new images for the training.

#### How can I ensure invalid numbers are never reported?

As mentioned above, the AI algorithm is not perfect. Sometimes it may read an incorrect value.

We can tune the software to _almost_ never report an incorrect value. There is a tradeoff though: the software may report _stale_ values - i.e. it will drop incorrect values for a potentially long period of time, resulting in the meter reading being outdated by hours. If never receiving an incorrect value is important to you, consider tolerating this tradeoff.

You can change the following settings to reduce incorrect readings (but potentially increase staleness of data):
* Set a prevalue via the UI, then change `PostProcessing` configuration option `PreValueAgeStartup` to a much larger number (e.g. `43200` = 30 days).
* Change `PostProcessing` configuration option `MaxRateType` to be time based instead of absolute. Set `MaxRateValue` to something realistic (e.g. `5` gal/min). You can often find the max flow rate your meter supports directly on the cover.
* Reduce `AutoTimer` configuration option `Intervall` to the lowest it can be (e.g. `3` min). The more often you take readings, the less likely for data staleness to occur.

#### Even after I have setup everything perfect there is a false reading - especially around the zero crossing (roll over to next number)
* The roll over behavior is different for the different meters. E.g.:
  * Rolling over start with different previous position (e.g. at 7, 8 or 9)
  * The neutral position (no rolling) is not perfectly at zero, but rather at something like 7.9 or 8.1, even if it should be exactly 8

* The "PostProcessingAlgo" is trying to judge out of the individual readings, what number it should be. 
  * For example if the previous number is a "1", but the next number seems to be a "8.9", mos probably there was a "zero crossing" and the number is a "9" and not still an "8"

* Currently the setting of the algorithm is set to fit most of the meters and cases. But the parameters do not fit perfectly for all situations. Therefore there might be intermediate states, where the reading is false. 
  This is especially the case, at the positions, where the roll over (zero crossing) is just starting.
* To prevent a sending of false parameters, there is the possibility to limit the maximum allowed change (MaxRateChange).
  Usually after some time and movement of the counters a bit further, the reading is getting back to a stable reading.
* To handle this, a parametrized setting would be needed. This is rather complicated to implement as subtle changes make a relevant difference. Currently this is not implemented. 
  So please be a bit patient with your meter :-)





