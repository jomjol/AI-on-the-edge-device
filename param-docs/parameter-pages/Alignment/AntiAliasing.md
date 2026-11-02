# Parameter `AntiAliasing`
Default Value: `false`

!!! Warning
    This is an **Expert Parameter**! Only change it if you understand what it does!

Use bilinear interpolation instead of nearest-neighbor when rotating the image during
alignment. This produces a sharper, less blocky aligned image, which reduces recognition
scatter on analog dials - fewer borderline reads means fewer values held or corrected by
post-processing.

The nearest-neighbor rotation used when this is `false` is faster, but truncates each
rotated pixel to its nearest source pixel, which can visibly soften/blur fine details such
as pointer positions on analog dials.
