# Parameter `CamGainceiling`

**Gain-Ceiling**

Available options:

- `x2`
- `x4`
- `x8`
- `x16`
- `x32`
- `x64`
- `x128`

Default Value for ov2640: `x4`<br>
Default Value for ov5640: `x8`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

    After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    A higher gain means that the sensor has a greater response to light, but also makes sensor noise more visible.
	
    This is used when **CamAgc** is on.

!!! Note
    The **Gain** is an analog multiplier applied to the raw sensor data.<br>
    The **Ceiling** is the maximum gain value that the sensor will use.
