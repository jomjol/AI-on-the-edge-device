# Parameter `CamAeLevel`

**Auto-Exposure-Level**

range on OV2640 (`-2` .. `2`)<br>
range on OV3660 and OV5640 (`-5` .. `5`)

Default Value: `0`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

	After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    The exposure offset for automatic exposure, lower values produce darker image.

!!! Note
    This parameter can also be set on the Reference Image configuration page!
