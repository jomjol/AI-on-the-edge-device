# Parameter `SegmentThreshold`
Default Value: `0.4`

Only used if `SegmentDetection` is enabled.

A segment is regarded as lit if its contrast to the background is at least this fraction of the contrast of the strongest segment of the same digit (range `0.05` to `0.95`).

Lower the value if faint segments are missed (e.g. due to glare or a weak LCD), raise it if unlit "ghost" segments are detected as lit.
