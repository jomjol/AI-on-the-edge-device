For Demo and Testing Purpose, the device can use pre-recorded images.

You need to enable it in the configuration (`TakeImage > Demo`) and also provide the needed files on the SD-Card.

One image per round gets used, starting with the first image for the first round.

For the reference image and the alignment also the first image gets used.

Once the last image got reached, it starts again with the first one.

## Example Demo
You can use the following demo or create your own one.
Just install it using the OTA Update functionality.

 - [demo.zip](https://github.com/jomjol/AI-on-the-edge-device/files/10320454/demo.zip) (this is just a zip of [this](https://github.com/jomjol/AI-on-the-edge-device/tree/master/code) folder in the repo)


## SD-Card Structure
```
demo/
├── 520.8983.jpg
├── 520.9086.jpg
├── 520.9351.jpg
├── ...
└── files.txt
```

- The jpg files can have any name
- The jpg files must be smaller than 30'000 bytes
- The `files.txt` must contains a list of those files, eg:
  ```
  520.8983.jpg
  520.9086.jpg
  520.9351.jpg
  ```

## Recoding
To record real images of a meter, you have to periodically fetch `http://<IP>/img_tmp/raw.jpg`.

To automate this, you can use the following shell script (Linux only):
```bash
#!/bin/bash

while [[ true ]]; do
    echo "fetching value..."
    wget -q http://192.168.1.151/value -O value.txt

    value=`cat value.txt`
    echo "Value: $value"
    
    diff=`diff value.txt value_previous.txt`
    changed=$?
    #echo "Diff: $diff"
    
    if [[ $changed -ne 0 ]]; then
        echo "Value changed:"
        echo $diff
        echo "fetching image..."
        wget -q http://192.168.1.151/img_tmp/raw.jpg -O $value.jpg
    else
        echo "Value did not change, skipping image fetching!"
    fi
    
    cp value.txt value_previous.txt
    
    echo "waiting 60s..."
    sleep 60
done
```

## How does it work
The Demo Mode tries to interfere as less as possible with the normal behavior. Whenever a Cam Framebuffer gets taken (`esp_camera_fb_get()`), it replaces the framebuffer with the image from the SD-Card.