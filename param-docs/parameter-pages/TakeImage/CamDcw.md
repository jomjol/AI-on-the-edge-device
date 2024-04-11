# Parameter `CamDcw`
Default Value: `true`
    
!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

!!! Note
    After changing this parameter you need to update your reference image and alignment markers!

!!! Note
    When DCW is on, the image that you receive will be the size that you requested (VGA, QQVGA, etc).
    When DCW is off, the image that you receive will be one of UXGA, SVGA, or CIF. In other words, literally the actual image size as read from the sensor without any scaling.
    Note that if DCW is off, and you pick a different image size, this implicitly turns DCW back on again (although this isn't reflected in the options). 
