# Parameter `CamAutoSharpness`

**Auto-Sharpness**

- When **true**, the camera attempts to automatically adjusts the sharpness.
- When **false**, the **CamSharpness** setting is used instead.

Default Value: `false`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

	After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    The OV2640 does not officially support auto sharpness, this is an experimental parameter!

!!! Note
    This parameter can also be set on the Reference Image configuration page!
