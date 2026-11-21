#include <unity.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "SevenSegmentDecoder.h"

// Segments per digit (a, b, c, d, e, f, g)
static const char *SegmentsOfDigit[10] = {
    "abcdef", "bc", "abdeg", "abcdg", "bcfg", "acdfg", "acdefg", "abc", "abcdefg", "abcdfg"
};

/**
 * Renders a synthetic seven segment digit into a RGB buffer.
 * The segment geometry deliberately differs from the decoder layout to mimic a real display.
 */
static std::vector<uint8_t> renderDigit(const char *segments, int w, int h, uint8_t fg, uint8_t bg,
                                        float slantDeg = 0, int noise = 0, int weakSegment = -1, uint8_t weakFg = 0) {
    std::vector<uint8_t> img(w * h * 3, bg);
    float t = 0.17f * w;   // thickness of vertical segments
    float th = 0.10f * h;  // thickness of horizontal segments
    float slant = tanf(slantDeg * (float) M_PI / 180.0f);

    for (int y = 0; y < h; ++y) {
        int shift = (int) roundf((h / 2.0f - (y + 0.5f)) * slant);

        for (int x0 = 0; x0 < w; ++x0) {
            int x = x0 - shift;     // coordinates in the upright digit
            bool on[7] = {false};
            on[0] = (y < th) && (x >= t * 0.5f) && (x < w - t * 0.5f);
            on[1] = (x >= w - t) && (y >= th * 0.5f) && (y < h / 2.0f);
            on[2] = (x >= w - t) && (y >= h / 2.0f) && (y < h - th * 0.5f);
            on[3] = (y >= h - th) && (x >= t * 0.5f) && (x < w - t * 0.5f);
            on[4] = (x < t) && (y >= h / 2.0f) && (y < h - th * 0.5f);
            on[5] = (x < t) && (y >= th * 0.5f) && (y < h / 2.0f);
            on[6] = (fabsf(y - h / 2.0f) < th / 2.0f) && (x >= t * 0.5f) && (x < w - t * 0.5f);

            int value = bg;
            for (int s = 0; s < 7; ++s) {
                if (on[s] && strchr(segments, 'a' + s)) {
                    value = (s == weakSegment) ? weakFg : fg;
                }
            }

            if (noise > 0) {
                value += (rand() % (2 * noise + 1)) - noise;
                value = value < 0 ? 0 : (value > 255 ? 255 : value);
            }

            for (int ch = 0; ch < 3; ++ch) {
                img[(y * w + x0) * 3 + ch] = (uint8_t) value;
            }
        }
    }

    return img;
}

static void checkAllDigits(uint8_t fg, uint8_t bg, t_SegmentPolarity polarity, float slantDeg, int noise) {
    SegmentDecoderConfig config;
    config.polarity = polarity;
    config.slantDeg = slantDeg;

    for (int digit = 0; digit < 10; ++digit) {
        std::vector<uint8_t> img = renderDigit(SegmentsOfDigit[digit], 30, 54, fg, bg, slantDeg, noise);
        SegmentDecodeResult r = DecodeSevenSegment(img.data(), 30, 54, 3, config);
        TEST_ASSERT_EQUAL_INT_MESSAGE(digit, r.digit, SegmentsOfDigit[digit]);
    }
}

void test_SegmentMaskToDigit() {
    TEST_ASSERT_EQUAL_INT(8, SegmentMaskToDigit(0x7F));
    TEST_ASSERT_EQUAL_INT(1, SegmentMaskToDigit(0x06));
    TEST_ASSERT_EQUAL_INT(-1, SegmentMaskToDigit(0x00));
    TEST_ASSERT_EQUAL_INT(-1, SegmentMaskToDigit(0x01));    // only top segment
}

void test_SevenSegment_DarkOnLight() {
    checkAllDigits(30, 200, SegmentPolarityDark, 0, 0);
    checkAllDigits(30, 200, SegmentPolarityAuto, 0, 0);
}

void test_SevenSegment_BrightOnDark() {
    checkAllDigits(230, 20, SegmentPolarityBright, 0, 0);
    checkAllDigits(230, 20, SegmentPolarityAuto, 0, 0);
}

void test_SevenSegment_Noise() {
    srand(42);
    checkAllDigits(40, 180, SegmentPolarityAuto, 0, 25);
}

void test_SevenSegment_Slant() {
    checkAllDigits(30, 200, SegmentPolarityAuto, 8, 0);
}

/**
 * Issue #4061: a 9 with a weak (faint) segment must not be read as 4.
 */
void test_SevenSegment_NineWithWeakSegment() {
    SegmentDecoderConfig config;

    // Bottom segment (d) only half as dark as the others
    std::vector<uint8_t> img = renderDigit(SegmentsOfDigit[9], 30, 54, 30, 200, 0, 0, SEG_D, 115);
    TEST_ASSERT_EQUAL_INT(9, DecodeSevenSegment(img.data(), 30, 54, 3, config).digit);

    // Top segment (a) faint -> still 9 thanks to threshold
    img = renderDigit(SegmentsOfDigit[9], 30, 54, 30, 200, 0, 0, SEG_A, 115);
    TEST_ASSERT_EQUAL_INT(9, DecodeSevenSegment(img.data(), 30, 54, 3, config).digit);

    // And a real 4 stays a 4
    img = renderDigit(SegmentsOfDigit[4], 30, 54, 30, 200);
    TEST_ASSERT_EQUAL_INT(4, DecodeSevenSegment(img.data(), 30, 54, 3, config).digit);
}

void test_SevenSegment_Blank() {
    SegmentDecoderConfig config;
    std::vector<uint8_t> img = renderDigit("", 30, 54, 30, 200);
    TEST_ASSERT_EQUAL_INT(-1, DecodeSevenSegment(img.data(), 30, 54, 3, config).digit);
}

void test_SevenSegment_Variants() {
    SegmentDecoderConfig config;
    std::vector<uint8_t> img = renderDigit("abcfg", 30, 54, 30, 200);   // 9 without bottom bar
    TEST_ASSERT_EQUAL_INT(9, DecodeSevenSegment(img.data(), 30, 54, 3, config).digit);
    img = renderDigit("cdefg", 30, 54, 30, 200);                         // 6 without top bar
    TEST_ASSERT_EQUAL_INT(6, DecodeSevenSegment(img.data(), 30, 54, 3, config).digit);
    img = renderDigit("abcf", 30, 54, 30, 200);                          // 7 with left bar
    TEST_ASSERT_EQUAL_INT(7, DecodeSevenSegment(img.data(), 30, 54, 3, config).digit);
}

void test_SevenSegment_SetLayout() {
    SegmentDecoderConfig config;
    TEST_ASSERT_EQUAL_INT(1, SetSegmentLayout(config, SEG_A, 20, 0, 60, 10));
    TEST_ASSERT_EQUAL_INT(1, config.layout[SEG_A][0] == 0.2f);
    TEST_ASSERT_EQUAL_INT(1, config.layout[SEG_A][2] == 0.8f);

    // Invalid values are rejected and the layout is left unchanged
    TEST_ASSERT_EQUAL_INT(0, SetSegmentLayout(config, SEG_B, 90, 0, 20, 10));    // exceeds the ROI
    TEST_ASSERT_EQUAL_INT(0, SetSegmentLayout(config, SEG_B, 10, 10, 0, 10));    // zero width
    TEST_ASSERT_EQUAL_INT(0, SetSegmentLayout(config, SEG_REGION_COUNT, 0, 0, 10, 10));
    TEST_ASSERT_EQUAL_INT(1, config.layout[SEG_B][0] == SegmentDefaultLayout[SEG_B][0]);
}

/**
 * ROI drawn with a margin around the digit: the sub-ROIs are moved to the segments via the layout.
 */
void test_SevenSegment_CustomLayout() {
    const int w = 30, h = 54;               // ROI
    const int ox = 4, oy = 6, dw = 22, dh = 42;  // digit inside the ROI

    SegmentDecoderConfig config;
    for (int r = 0; r < SEG_REGION_COUNT; ++r) {
        float x0 = (ox + SegmentDefaultLayout[r][0] * dw) * 100.0f / w;
        float y0 = (oy + SegmentDefaultLayout[r][1] * dh) * 100.0f / h;
        float x1 = (ox + SegmentDefaultLayout[r][2] * dw) * 100.0f / w;
        float y1 = (oy + SegmentDefaultLayout[r][3] * dh) * 100.0f / h;
        TEST_ASSERT_EQUAL_INT(1, SetSegmentLayout(config, r, x0, y0, x1 - x0, y1 - y0));
    }

    for (int digit = 0; digit < 10; ++digit) {
        std::vector<uint8_t> small = renderDigit(SegmentsOfDigit[digit], dw, dh, 30, 200);
        std::vector<uint8_t> img(w * h * 3, 200);
        for (int y = 0; y < dh; ++y) {
            memcpy(&img[((y + oy) * w + ox) * 3], &small[y * dw * 3], dw * 3);
        }
        TEST_ASSERT_EQUAL_INT_MESSAGE(digit, DecodeSevenSegment(img.data(), w, h, 3, config).digit, SegmentsOfDigit[digit]);
    }
}
