# Over-The-Air (OTA) Update

You can do an OTA (over-the-air) update via the graphical user interface.
Grab the firmware from the

 *  [Releases page](https://github.com/jomjol/AI-on-the-edge-device/releases) (Stable, tested versions), or the
 *  [Automatically build development branch](https://github.com/jomjol/AI-on-the-edge-device/actions?query=branch%3Arolling) (experimental, untested versions). Please have a look on https://github.com/jomjol/AI-on-the-edge-device/wiki/Install-a-rolling-%28unstable%29-release first!

You need:
* firmware.bin
* html.zip

### **General remark:** 

- It is always recommended to upload both files, as they are coupled to each other
- If you make a major update, it might be needed to modify the `config.ini` as it's syntax or context has changed
- It is recommended to make a **backup** of the `/config`  directory, minimum of the `config.ini`.



### Access to the update page:

The graphical OTA update can be accessed in the menue "System":

* <img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/ota-update-menue.jpg" width="600" align="middle">


### Update

* <img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/ota-update-details.jpg" width="600" align="middle">

Just follow the steps 1 to 5 to perform the update:

1. Select (a) and upload (b) the file `firmware.bin`
2. Flash the firmware
3. Select (a) and upload (b) the file `html.zip`
4. Update the html-files
5. Reboot



**After the reboot with a major change it is recommended to check the configuration settings and save them again**