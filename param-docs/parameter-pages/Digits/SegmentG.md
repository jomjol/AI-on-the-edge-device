# Parameter `SegmentG`
Default Value: `28 45 44 10`

Unit: Percent of the digit ROI (`x y dx dy`)

Only used if `SegmentDetection` is enabled.

Position of the sub-ROI used to sample segment **G** (middle bar), relative to the digit ROI. Same convention as a ROI:
`x` and `y` are the top left corner, `dx` and `dy` the width and height, all in percent of the digit ROI width/height (`0` to `100`).
The layout is used for all digits of the `[Digits]` section.

Segment layout:
```
     aaa
    f   b
    f   b
     ggg
    e   c
    e   c
     ddd
```

The sub-ROI should cover the middle part of the segment only, so that it does not touch neighbouring segments.
Check the position in the ROI overlay image (yellow = segments, green = background reference).
