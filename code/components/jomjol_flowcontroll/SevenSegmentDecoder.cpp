#include "SevenSegmentDecoder.h"

#include <math.h>

/**
 * Position of the sub-ROIs, relative to the digit ROI (0.0 .. 1.0), as {x0, y0, x1, y1}.
 * The ROI should be drawn tightly around the segments of the digit.
 * The regions only cover the middle part of every segment so that neighbouring segments
 * (and small ROI misalignments) do not influence each other.
 */
static const float SegmentLayout[SEG_REGION_COUNT][4] = {
    {0.28f, 0.02f, 0.72f, 0.12f},   // a - top
    {0.80f, 0.16f, 0.98f, 0.40f},   // b - top right
    {0.80f, 0.60f, 0.98f, 0.84f},   // c - bottom right
    {0.28f, 0.88f, 0.72f, 0.98f},   // d - bottom
    {0.02f, 0.60f, 0.20f, 0.84f},   // e - bottom left
    {0.02f, 0.16f, 0.20f, 0.40f},   // f - top left
    {0.28f, 0.45f, 0.72f, 0.55f},   // g - middle
    {0.32f, 0.20f, 0.68f, 0.38f},   // upper hole (background reference)
    {0.32f, 0.62f, 0.68f, 0.80f},   // lower hole (background reference)
};

#define SEGMASK(a, b, c, d, e, f, g) ((a) | ((b) << 1) | ((c) << 2) | ((d) << 3) | ((e) << 4) | ((f) << 5) | ((g) << 6))

struct SegmentPattern {
    uint8_t mask;
    int digit;
};

// Valid patterns, including common display variants (6 without top bar, 7 with left bar, 9 without bottom bar)
static const SegmentPattern SegmentPatterns[] = {
    //         a  b  c  d  e  f  g
    {SEGMASK(1, 1, 1, 1, 1, 1, 0), 0},
    {SEGMASK(0, 1, 1, 0, 0, 0, 0), 1},
    {SEGMASK(1, 1, 0, 1, 1, 0, 1), 2},
    {SEGMASK(1, 1, 1, 1, 0, 0, 1), 3},
    {SEGMASK(0, 1, 1, 0, 0, 1, 1), 4},
    {SEGMASK(1, 0, 1, 1, 0, 1, 1), 5},
    {SEGMASK(1, 0, 1, 1, 1, 1, 1), 6},
    {SEGMASK(0, 0, 1, 1, 1, 1, 1), 6},
    {SEGMASK(1, 1, 1, 0, 0, 0, 0), 7},
    {SEGMASK(1, 1, 1, 0, 0, 1, 0), 7},
    {SEGMASK(1, 1, 1, 1, 1, 1, 1), 8},
    {SEGMASK(1, 1, 1, 1, 0, 1, 1), 9},
    {SEGMASK(1, 1, 1, 0, 0, 1, 1), 9},
};

int SegmentMaskToDigit(uint8_t mask) {
    for (unsigned int i = 0; i < sizeof(SegmentPatterns) / sizeof(SegmentPatterns[0]); ++i) {
        if (SegmentPatterns[i].mask == mask) {
            return SegmentPatterns[i].digit;
        }
    }
    return -1;
}

static int clampInt(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

SegmentRect GetSegmentRect(int region, int roiWidth, int roiHeight, float slantDeg) {
    SegmentRect rect = {0, 0, 0, 0, 0};

    if ((region < 0) || (region >= SEG_REGION_COUNT) || (roiWidth <= 0) || (roiHeight <= 0)) {
        return rect;
    }

    int x0 = (int) floorf(SegmentLayout[region][0] * roiWidth);
    int y0 = (int) floorf(SegmentLayout[region][1] * roiHeight);
    int x1 = (int) ceilf(SegmentLayout[region][2] * roiWidth);
    int y1 = (int) ceilf(SegmentLayout[region][3] * roiHeight);

    x0 = clampInt(x0, 0, roiWidth - 1);
    y0 = clampInt(y0, 0, roiHeight - 1);
    x1 = clampInt(x1, x0 + 1, roiWidth);
    y1 = clampInt(y1, y0 + 1, roiHeight);

    rect.x = x0;
    rect.y = y0;
    rect.dx = x1 - x0;
    rect.dy = y1 - y0;

    float centerY = (y0 + y1) / 2.0f;
    rect.slantOffset = (int) roundf((roiHeight / 2.0f - centerY) * tanf(slantDeg * (float) M_PI / 180.0f));

    return rect;
}

/**
 * Mean grey value of one region. The slant is applied row by row so that italic digits are sampled correctly.
 */
static float regionMean(const uint8_t *image, int width, int height, int channels, int region, float slantDeg) {
    SegmentRect rect = GetSegmentRect(region, width, height, slantDeg);
    float slant = tanf(slantDeg * (float) M_PI / 180.0f);
    unsigned long sum = 0;
    unsigned long count = 0;

    for (int y = rect.y; y < rect.y + rect.dy; ++y) {
        int shift = (int) roundf((height / 2.0f - (y + 0.5f)) * slant);

        for (int x = rect.x + shift; x < rect.x + rect.dx + shift; ++x) {
            if ((x < 0) || (x >= width)) {
                continue;
            }

            const uint8_t *p = image + ((long) y * width + x) * channels;
            unsigned int grey = 0;

            for (int ch = 0; ch < channels; ++ch) {
                grey += p[ch];
            }

            sum += grey / channels;
            ++count;
        }
    }

    if (count == 0) {
        return 0;
    }

    return (float) sum / count;
}

SegmentDecodeResult DecodeSevenSegment(const uint8_t *image, int width, int height, int channels, const SegmentDecoderConfig &config) {
    SegmentDecodeResult result;

    if ((image == NULL) || (width < 7) || (height < 7) || (channels < 1)) {
        return result;
    }

    float mean[SEG_REGION_COUNT];

    for (int i = 0; i < SEG_REGION_COUNT; ++i) {
        mean[i] = regionMean(image, width, height, channels, i, config.slantDeg);
    }

    result.background = (mean[SEG_HOLE_UPPER] + mean[SEG_HOLE_LOWER]) / 2.0f;

    int polarity = (int) config.polarity;

    if (polarity == SegmentPolarityAuto) {
        // Unlit segments are close to the background, so the sum of all differences
        // points in the direction of the lit segments (every digit has at least two lit segments).
        float sumDiff = 0;

        for (int i = 0; i < SEG_COUNT; ++i) {
            sumDiff += mean[i] - result.background;
        }

        polarity = (sumDiff >= 0) ? SegmentPolarityBright : SegmentPolarityDark;
    }

    result.polarity = polarity;

    float maxContrast = 0;

    for (int i = 0; i < SEG_COUNT; ++i) {
        result.contrast[i] = polarity * (mean[i] - result.background);

        if (result.contrast[i] > maxContrast) {
            maxContrast = result.contrast[i];
        }
    }

    if (maxContrast < config.minContrast) {
        // Nothing lit / no usable contrast -> unreadable
        return result;
    }

    float threshold = config.threshold * maxContrast;

    for (int i = 0; i < SEG_COUNT; ++i) {
        if (result.contrast[i] >= threshold) {
            result.mask |= (1 << i);
        }
    }

    result.digit = SegmentMaskToDigit(result.mask);

    return result;
}
