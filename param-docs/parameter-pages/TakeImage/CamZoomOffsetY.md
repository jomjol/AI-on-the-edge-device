# Parameter `CamZoomOffsetY`

**Digital-Zoom-OffsetY**

range on OV2640 (`-360` .. `360`)<br>
range on OV3660 (`-528` .. `528`)<br>
range on OV5640 (`-720` .. `720`)

Default Value: `0`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

	After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    Y displacement of the image from the center.<br>
    Positive values ​​move the image up, negative values ​​move the image down.<br>
    The maximum possible offset depends on the value of the **CamZoomSize**.
	
!!! Note
    This parameter can also be set on the Reference Image configuration page!
