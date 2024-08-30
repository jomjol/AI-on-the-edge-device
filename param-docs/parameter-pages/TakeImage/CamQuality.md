# Parameter `CamQuality`

**Image-Quality**

Range (`8` .. `63`)

Default Value: `10`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

    After changing this parameter you need to update your reference image and alignment markers!

    Value below 10 could result in system instabilities!

!!! Note
    Quality index for pictures: `8` (highest quality) ... `63` (lowest quality)

    This is similar to the quality setting when exporting a jpeg image from photo editing software.
