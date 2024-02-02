# Parameter `RSSIThreshold`
Default Value: `0`

Possible values: `-100` .. `0` (`0` = disabled).

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!


This parameter activates a client triggered AP switching functionality (simplified roaming). 
If actual RSSI value is lower (more negative) than `RSSIThreshold`, all WIFI channels will be scanned for configured access point SSID. If an access point is in range which has better RSSI value (less negative) than actual RSSI value + 5 dBm, the device is trying to connect to this access point with the better RSSI value.


!!! Note
    The RSSI check only gets initiated at the end of each round to avoid any disturbance of processing.


!!! Note
    It gets automatically transferred to `/wlan.ini` on the SD-Card at next startup.
