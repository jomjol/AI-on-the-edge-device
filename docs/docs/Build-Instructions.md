# New
See [README.md](https://github.com/jomjol/AI-on-the-edge-device/blob/master/code/README.md)

# Old

## Build the project yourself

- Download and install VS Code
  - https://code.visualstudio.com/Download
- Install the VS Code platform io plugin
  - <img src="https://raw.githubusercontent.com/jomjol/ai-on-the-edge-device/master/images/platformio_plugin.jpg" width="200" align="middle">
  - Check for error messages, maybe you need to manually add some python libraries
    - e.g. in my Ubuntu a python3-env was missing: `sudo apt-get install python3-venv`
- git clone this project
  - in Linux: `git clone https://github.com/jomjol/AI-on-the-edge-device.git`
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

