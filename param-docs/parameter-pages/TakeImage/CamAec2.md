# Parameter `CamAec2`

**Auto-Exposure-Control2**

- When **true**, the sensor’s "night mode" is enabled, extending the range of automatic gain control.
- When **false**, the sensor’s "night mode" is disabled.

Default Value: `true`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

	After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    This may resolve some over-exposure and under-exposure issues.
	
!!! Note
    This parameter can also be set on the Reference Image configuration page!
