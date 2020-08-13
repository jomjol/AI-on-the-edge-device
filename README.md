# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

Details can be found on the Wiki pages.

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/index.png" width="600"> 


## Changelog - lastest version

##### Rolling (2020-08-13)

* Implementation of Digital Consistency Check: check if Digit shows next number earlier, than previous number has gone through zero - Turn on/off - see updated config.ini `CheckDigitIncreaseConsistency = True`)
  Not fully tested!
  
  

##### 0.1.0 (2020-08-07)

* Initial Version



## Known Issues

* spontaneous reboot, especially in case of intensive web server access
* stopping automated tflite flow - Error teared down to alignment procedure (results in restart)
* Camera initialization stuck not often, but randomly 5s after reboot - Hard reboot (power on/off) necessary :-(
  --> Next try: adopted reboot sequence with additional and extended camera reset



## Solved topics

* stuck in reboot - Solved by modified startup sequence