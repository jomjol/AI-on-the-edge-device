# Parameter `CamWbMode`

**White-Balance-Mode**

Available options:

- `auto`
- `sunny`
- `cloudy`
- `office`
- `home`

Default Value: `auto`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    This is used when **CamAwb** is off.
	
!!! Note
    This parameter can also be set on the Reference Image configuration page!
