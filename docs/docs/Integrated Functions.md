## wasserzaehler

```http://IP-ESP32/wasserzaehler.html```

This is the main purpose of this device. It returns the converted image as a number with different option. The output can be modified either by the configuration parameters or by HTML parameters.

Details can be found here:  tbd



## Picture Server

```http://IP-ESP32/capture```

```http://IP-ESP32/capture_with_flashlight```

This is a implementation of the camera interface of https://github.com/jomjol/water-meter-picture-provider

It is fully compatible including the parameters (```quality```=..., ``size=...`` ) . This allows to use this ESP32 system in parallel to the corresponding docker system: https://github.com/jomjol/water-meter-system-complete, from which this project is basically the successor.



## File server

Access: ```http://IP-ESP32/fileserver/```

Simple file server, that allows viewing, upload, download and deleting of single files of the SD-card content.

The usage is self explaining. The file path or file can directly be accessed by the URL after file server.

Example for ```config.ini``` :  ```http://IP-ESP/fileserver/config/config.ini```



## OTA-Update

```http://IP-ESP32/ota?file=firmware.bin```

Here an over the air update can be triggered. The firmware file is expected to be located in the sub directory ```/firmware/``` and can be uploaded with the file server. By the parameter ```file``` the name of the firmware file needs to be given.



## Reboot

```http://IP-ESP32/reboot```

A reboot with a delay of 5 seconds is initiated, e.g. after firmware update.

**ATTENTION**: currently this is not working properly - hardware power off is needed instead. **Work in progress!**



##  Simple Web Server

If none of the above URLs are fitting, a very simple web server checks, if there is a fitting file from the sub directory ```/html``` 
This can be used for a very simple web server for information or simple web pages.