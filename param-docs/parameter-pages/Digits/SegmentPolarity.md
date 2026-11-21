# Parameter `SegmentPolarity`
Default Value: `auto`

Only used if `SegmentDetection` is enabled.

Defines how lit segments look compared to the display background:

| Value    | Description |
| -------- | ----------- |
| `auto`   | Detected automatically for every digit |
| `dark`   | Lit segments are darker than the background (classic LCD) |
| `bright` | Lit segments are brighter than the background (LED displays, inverted / backlit LCD) |

Setting a fixed value makes the detection more robust against reflections.
