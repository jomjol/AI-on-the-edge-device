# Parameter `CamAgc`

**Auto-Gain-Control**

- When **true**, the camera attempts to automatically control the sensor gain, up to the value in the **CamGainceiling** property.
- When **false**, the **CamAgcGain** setting is used instead.

Default Value: `true`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

    After changing this parameter you need to update your reference image and alignment markers!
