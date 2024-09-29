# Parameter `CamDcw`

**Image-Downsize**

- When **CamDcw** is on, the image that you receive will be the size that you requested (VGA, QQVGA, etc).
- When **CamDcw** is off, the image that you receive will be one of UXGA, SVGA, or CIF.

Default Value: `true`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

    After changing this parameter you need to update your reference image and alignment markers!
	
	If **CamZoom** is used, this must be activated.

!!! Note
    If **CamDcw** is off, and you pick a different image size, this implicitly turns **CamDcw** back on again. 
