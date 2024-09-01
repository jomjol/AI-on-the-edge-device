# Parameter `CamRawGma`

**Raw-Gamma**

- Enable/Disable raw gamma mode.

Default Value: `true`

See [here](../datasheets/Camera.ov2640_ds_1.8_.pdf) for the ov2640 camera datasheet.<br>
See [here](../datasheets/OV5640_datasheet.pdf) for the ov5640 camera datasheet.

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

    After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    The main purpose of the Gamma (GMA) function is to compensate for the non-linear characteristics of the sensor. 
	GMA converts the pixel values according to the Gamma curve to compensate the sensor output under different light strengths.
    The non-linear gamma curve is approximately constructed with different linear functions. Raw gamma compensates the
    image in the RAW domain.
