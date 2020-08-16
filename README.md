# AI-on-the-edge-device

This is an example of Artificial Intelligence (AI) calculations on a very cheap hardware.

Details on **function**, **installation** and **configuration** can be found on the **[Wiki Page](https://github.com/jomjol/AI-on-the-edge-device/wiki)**

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/main.jpg" width="300"><img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/size.png" width="300"> 

<img src="https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/master/images/index.png" width="600"> 


## Change log - latest version

##### Rolling (2020-08-15)

* Nightly updates (bug fixing)
  
* Implementation of Digital Consistency Check: check if Digit shows next number earlier, than previous number has gone through zero - Turn on/off - see updated config.ini `CheckDigitIncreaseConsistency = True`)
  Not fully tested!
  
  

##### 0.1.0 (2020-08-07)

* Initial Version



## Known Issues

* spontaneous reboot, especially in case of intensive web server access
* stopping automated tflite flow - Error teared down to alignment procedure (results in restart) - Image load fails



## Solved topics

* stuck in reboot - Solved by modified startup sequence
* Camera initialization stuck not often, but randomly 5s after reboot - Hard reboot (power on/off) necessary :-(
  --> Next try: adopted reboot sequence with additional and extended camera reset --> seems working!