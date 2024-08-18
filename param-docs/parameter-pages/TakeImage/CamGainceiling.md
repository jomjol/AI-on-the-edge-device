# Parameter `CamGainceiling`
Default Value: `x4`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

    After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    This is used when **CamAgc** is on.

**CamGainceiling**

The **Gain** is an analog multiplier applied to the raw sensor data.

The **Ceiling** is the maximum gain value that the sensor will use.

A higher gain means that the sensor has a greater response to light, but also makes sensor noise more visible.

Available options:

- `x2`
- `x4`
- `x8`
- `x16`
- `x32`
- `x64`
- `x128`
