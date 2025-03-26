# Build

## Preparations
```
git clone https://github.com/jomjol/AI-on-the-edge-device.git
cd AI-on-the-edge-device
git checkout main
git submodule update --init
```

## Update Submodules
```
cd /components/submodule-name (e.g. tflite-micro-example)
git checkout VERSION (e.g. HASH of latest tflite-micro-example build)
cd ../../ (at the code level)
git submodule update --init
```
You may need to manually delete some directories in the 'components' folder beforehand, as they were not deleted during checkout (before update -- init)

## Build and Flash within terminal
See further down to build it within an IDE.
### Compile
```
cd code
platformio run --environment esp32cam
```

### Upload
```
pio run --target upload --upload-port /dev/ttyUSB0
```

Alternatively you also can set the UART device in `platformio.ini`, eg. `upload_port = /dev/ttyUSB0`

### Monitor UART Log
```
pio device monitor -p /dev/ttyUSB0
```

## Build and Flash with Visual Code IDE

- Download and install VS Code
  - https://code.visualstudio.com/Download
- Install the VS Code platform io plugin
  - <img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/platformio_plugin.jpg" width="200" align="middle">
  - Check for error messages, maybe you need to manually add some python libraries
    - e.g. in my Ubuntu a python3-env was missing: `sudo apt-get install python3-venv`
- git clone this project
  - in Linux: 

    ```
    git clone https://github.com/jomjol/AI-on-the-edge-device.git
    cd AI-on-the-edge-device
    git checkout main
    git submodule update --init
    ```

- in VS code, open the `AI-on-the-edge-device/code` 
	- from terminal: `cd AI-on-the-edge-device/code && code .`
- open a pio terminal (click on the terminal sign in the bottom menu bar)
- make sure you are in the `code` directory
- To build, type  `platformio run --environment esp32cam`
  - or use the graphical interface:
    <img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/platformio_build.jpg" width="200" align="middle">
  - the build artifacts are stored in  `code/.pio/build/esp32cam/`
- Connect the device and type `pio device monitor`. There you will see your device and can copy the name to the next instruction
- Add `upload_port = you_device_port` to the `platformio.ini` file
- make sure an sd card with the contents of the `sd_card` folder is inserted and you have changed the wifi details
- `pio run --target erase` to erase the flash
- `pio run --target upload` this will upload the `bootloader.bin, partitions.bin,firmware.bin` from the `code/.pio/build/esp32cam/` folder. 
- `pio device monitor` to observe the logs via uart

# Update Parameters
If you create or rename a parameter, make sure to update its documentation in `../param-docs/parameter-pages`! Check the `../param-docs/README.md` for more information.
