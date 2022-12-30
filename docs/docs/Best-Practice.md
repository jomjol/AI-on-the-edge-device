This page shows some best practices.
# Camera Placement 
* Move the Camera as close as possible(~4cm), this will help get rid of reflections.
  -> focus can be adjusted by turning the outer black ring of the camera.
* If the LED reflections are too strong, put tape over the LED to defuse the light
* Change the ImageSize to QVGA under "Expert mode" configuration when close enough, this will be faster and is often good enough for digital recognition.

# Reflections 

  * Try to get ride of the reflections by rotating the camera, so that the reflections are at positions, where no number is.
  * By using the external LED option, you can place WS2812 LEDs freely away from the main axis.
  * Users report, that a handy cover foil could also help

# Post-processing
* Filter out the Number "9", as "3" will often be misread for a "9" and void every number between 3 and 9 due to it being negative flow.
* Split the readings into two, while the decimal numbers might move to fast to be recognized, at least the slower moving part will produce a correct reading.
-> keep in mind that the offset needs to be adjusted, a.e if you have a comma reading of "3", it needs to become "0.3". This can be done wherever the data ends up being sent, like home assistant using sensor templates.
* If you are using a low resolution and only digital mode, processing can often be done in <1 minute. Check the logs to confirm how fast it is and then set the interval accordingly under "Expert mode" in configuration, as the normal mode will lock you to 3+ minutes.

***

* [ ] Todo condense from various discussions, eg. ~~https://github.com/jomjol/AI-on-the-edge-device/issues/765~~ and https://github.com/jomjol/AI-on-the-edge-device/discussions/984
* [ ] Todo add images and more in-depth explanation 
