# Parameter `CamDcw`
Default Value: `true`
    
!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

    After changing this parameter you need to update your reference image and alignment markers!
	
	If **CamZoom** is used, this must be activated.

!!! Note
    When **CamDcw** is on, the image that you receive will be the size that you requested (VGA, QQVGA, etc).
    When **CamDcw** is off, the image that you receive will be one of UXGA, SVGA, or CIF. In other words, literally the actual image size as read from the sensor without any scaling.
    Note that if **CamDcw** is off, and you pick a different image size, this implicitly turns **CamDcw** back on again (although this isn't reflected in the options). 

**Downsize**

Enable/Disable camera image scaling.
