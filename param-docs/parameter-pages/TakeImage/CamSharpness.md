# Parameter `CamSharpness`

**Image-Sharpness**

Range (`-2` .. `2`)

Default Value: `0`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

	After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    The OV2640 does not officially support sharpness, this is an experimental parameter!
	
!!! Note
    Positive values increase sharpness (more defined edges), negative values lower it (softer edges).

    This is used when **CamAutoSharpness** is off.

!!! Note
    This parameter can also be set on the Reference Image configuration page!
