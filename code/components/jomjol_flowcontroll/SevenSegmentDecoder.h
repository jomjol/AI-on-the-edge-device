#pragma once

#ifndef SEVENSEGMENTDECODER_H
#define SEVENSEGMENTDECODER_H

#include <stdint.h>

/**
 * Seven segment digit decoder
 *
 * Alternative to the CNN digit classification for displays with very consistent digits
 * (LCD / LED seven segment displays). Each digit ROI is divided into seven sub-ROIs, one per
 * segment, plus two "hole" sub-ROIs inside the upper and lower loop of the digit that are never lit
 * and are used as background reference. The software measures which segments are lit and builds
 * the digit from that pattern.
 *
 * Segment naming (standard):
 *
 *       aaa
 *      f   b
 *      f   b
 *       ggg
 *      e   c
 *      e   c
 *       ddd
 *
 * This file has no ESP-IDF dependencies so it can be unit tested on a host machine.
 */

enum t_SegmentPolarity {
    SegmentPolarityAuto = 0,   // Detect automatically (lit segments differ from the background in the majority direction)
    SegmentPolarityBright = 1, // Lit segments are brighter than the background (e.g. LED display, backlit LCD)
    SegmentPolarityDark = -1   // Lit segments are darker than the background (e.g. classic LCD)
};

enum t_SegmentIndex {
    SEG_A = 0, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G,
    SEG_COUNT,
    SEG_HOLE_UPPER = SEG_COUNT, SEG_HOLE_LOWER,
    SEG_REGION_COUNT
};

/**
 * Default position of the sub-ROIs relative to the digit ROI (0.0 .. 1.0) as {x0, y0, x1, y1}.
 * Order: a, b, c, d, e, f, g, upper background, lower background.
 */
extern const float SegmentDefaultLayout[SEG_REGION_COUNT][4];

struct SegmentDecoderConfig {
    SegmentDecoderConfig();

    float layout[SEG_REGION_COUNT][4];   // Position of the sub-ROIs relative to the digit ROI (0.0 .. 1.0) as {x0, y0, x1, y1}
    t_SegmentPolarity polarity = SegmentPolarityAuto;
    float slantDeg = 0.0f;          // Italic slant of the digits in degrees (positive: top of the digit leans to the right)
    float threshold = 0.4f;         // A segment counts as lit if its contrast is >= threshold * contrast of the strongest segment
    int minContrast = 20;           // Minimum contrast (grey levels 0..255) of the strongest segment, otherwise the digit is unreadable
};

struct SegmentDecodeResult {
    int digit = -1;                 // 0..9, or -1 if the segment pattern is not a valid digit / unreadable
    uint8_t mask = 0;               // Bit n set = segment n lit (bit 0 = a ... bit 6 = g)
    float contrast[SEG_COUNT] = {0};// Contrast of every segment against the background (positive = lit direction)
    float background = 0;           // Mean grey value of the background reference regions
    int polarity = 0;               // Polarity which was used (1 bright, -1 dark)
};

struct SegmentRect {
    int x, y, dx, dy;               // Rectangle relative to the ROI, before slant is applied
    int slantOffset;                // Horizontal shift of the rectangle centre caused by the slant
};

/**
 * @brief Converts a segment bit mask into a digit
 * @return 0..9 or -1 if the pattern is not a digit
 */
int SegmentMaskToDigit(uint8_t mask);

/**
 * @brief Sets the position of one sub-ROI, given in percent of the digit ROI (same convention as a ROI: x y dx dy)
 * @return false if the values are out of range (layout is left unchanged)
 */
bool SetSegmentLayout(SegmentDecoderConfig &config, int region, float xPercent, float yPercent, float dxPercent, float dyPercent);

/**
 * @brief Returns the sub-ROI of one segment (or background region) for a ROI of the given size
 */
SegmentRect GetSegmentRect(int region, int roiWidth, int roiHeight, const SegmentDecoderConfig &config);

/**
 * @brief Decodes one digit from an image buffer
 *
 * @param image    pointer to the pixel data (row major, interleaved channels)
 * @param width    width of the image (= ROI width)
 * @param height   height of the image (= ROI height)
 * @param channels number of channels (1 = grey, 3 = RGB)
 * @param config   decoder settings
 */
SegmentDecodeResult DecodeSevenSegment(const uint8_t *image, int width, int height, int channels, const SegmentDecoderConfig &config);

#endif // SEVENSEGMENTDECODER_H
