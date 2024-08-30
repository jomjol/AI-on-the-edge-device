# Parameter `<NUMBER>.NegativeRateThreshold`
Default Value: `2`

Threshold parameter for negative rate detection.<br>
This parameter is intended to compensate for small reading fluctuations 
that occur when the counter does not change its value for a long time (e.g. at night).<br>
It is only applied to the last digit of the value read.<br>


Example:

    Value = 2
	
    Extended Resolution disabled:
    PreValue: 123.4567 >>> Threshold = +/- 0.0002
	Comparative value >>> max = 123.4569 and min = 123.4565
	
    Extended Resolution enabled:
    PreValue: 123.45678 >>> Threshold = +/- 0.00002
	Comparative value >>> max = 123.45680 and min = 123.45676
	
	If the value read is not greater than max and not less than min, 
	no further calculation is carried out and the Value/Prevalue 
	remain at the old value.

Range: `1` .. `9`.

![](img/NegativeRateThreshold.png)
