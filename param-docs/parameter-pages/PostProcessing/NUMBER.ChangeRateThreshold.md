# Parameter `<NUMBER>.ChangeRateThreshold`
Default Value: `2`

Range: `1` .. `9`.

Threshold parameter for change rate detection.<br>
This parameter is intended to compensate for small reading fluctuations that occur when the meter does not change its value for a long time (e.g. at night) or slightly turns backwards. This can eg. happen on watermeters.

It is only applied to the last digit of the read value (See example below).
If the read value is within PreValue +/- Threshold, no further calculation is carried out and the Value/Prevalue remains at the old value.

Example:

    Smallest ROI provides value for 0.000x
    ChangeRateThreshold = 2
  
    Extended Resolution disabled:
    PreValue: 123.456'7 >>> Threshold = +/- 0.000'2
	Comparative value >>> max = 123.456'9 and min = 123.456'5
	
    Extended Resolution enabled:
    PreValue: 123.456'78 >>> Threshold = +/- 0.000'02
	Comparative value >>> max = 123.456'80 and min = 123.456'76

![](img/ChangeRateThreshold.png)
