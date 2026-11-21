# Parameter `SegmentDetection`
Default Value: `false`

Reads the digits by detecting the individual segments of a seven segment display (LCD / LED) instead of classifying the whole digit with the CNN model.

Each digit ROI is divided into seven sub-ROIs, one for every segment, plus two background reference areas inside the upper and lower loop of the digit. The firmware checks which segments are lit and builds the digit from that pattern. A pattern which is not a valid digit is reported as `N`.

This can be more reliable than the CNN for displays whose digits look very consistent, but are not well covered by the training data (e.g. a `9` being read as `4`, see issue #4061).

When enabled, the configured digit `Model` is not used.

!!! Note
    The default sub-ROI positions expect the digit ROI to be drawn **tightly** around the segments of the digit (the outer edges of the segments should touch the ROI border).
    The position of every sub-ROI can be adjusted with the parameters `SegmentA` ... `SegmentG`, `SegmentBgUpper` and `SegmentBgLower` (expert parameters).
    The segment sub-ROIs are drawn into the ROI overlay image (yellow = segments, green = background reference), so you can check that they sit on the segments.

!!! Note
    Only for seven segment displays. Do not enable it for mechanical counter wheels.
