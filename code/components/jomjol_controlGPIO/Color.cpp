/********************************************************************************
 * https://github.com/RoboticsBrno/SmartLeds
 *
 * MIT License
 * 
 * Copyright (c) 2017 RoboticsBrno (RobotikaBrno)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#include "Color.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace {

// Int -> fixed point
int up(int x) { return x * 255; }

} // namespace

int iRgbSqrt(int num) {
    // https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Binary_numeral_system_.28base_2.29
    assert("sqrt input should be non-negative" && num >= 0);
    assert("sqrt input should no exceed 16 bits" && num <= 0xFFFF);
    int res = 0;
    int bit = 1 << 16;
    while (bit > num)
        bit >>= 2;
    while (bit != 0) {
        if (num >= res + bit) {
            num -= res + bit;
            res = (res >> 1) + bit;
        } else
            res >>= 1;
        bit >>= 2;
    }
    return res;
}

Rgb::Rgb(const Hsv& y) {
    // https://stackoverflow.com/questions/24152553/hsv-to-rgb-and-back-without-floating-point-math-in-python
    // greyscale
    if (y.s == 0) {
        r = g = b = y.v;
        return;
    }

    const int region = y.h / 43;
    const int remainder = (y.h - (region * 43)) * 6;

    const int p = (y.v * (255 - y.s)) >> 8;
    const int q = (y.v * (255 - ((y.s * remainder) >> 8))) >> 8;
    const int t = (y.v * (255 - ((y.s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
    case 0:
        r = y.v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = y.v;
        b = p;
        break;
    case 2:
        r = p;
        g = y.v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = y.v;
        break;
    case 4:
        r = t;
        g = p;
        b = y.v;
        break;
    case 5:
        r = y.v;
        g = p;
        b = q;
        break;
    default:
        __builtin_trap();
    }

    a = y.a;
}

Rgb& Rgb::operator=(const Hsv& hsv) {
    Rgb r { hsv };
    swap(r);
    return *this;
}

Rgb Rgb::operator+(const Rgb& in) const {
    auto copy = *this;
    copy += in;
    return copy;
}

Rgb& Rgb::operator+=(const Rgb& in) {
    unsigned int red = r + in.r;
    r = (red < 255) ? red : 255;
    unsigned int green = g + in.g;
    g = (green < 255) ? green : 255;
    unsigned int blue = b + in.b;
    b = (blue < 255) ? blue : 255;
    return *this;
}

Rgb Rgb::operator-(const Rgb& in) const {
    auto copy = *this;
    copy -= in;
    return copy;
}

Rgb& Rgb::operator-=(const Rgb& in) {
    r = (in.r > r) ? 0 : r - in.r;
    g = (in.g > g) ? 0 : g - in.g;
    b = (in.b > b) ? 0 : b - in.b;
    return *this;
}

Rgb& Rgb::blend(const Rgb& in) {
    unsigned int inAlpha = in.a * (255 - a);
    unsigned int alpha = a + inAlpha;
    r = iRgbSqrt(((r * r * a) + (in.r * in.r * inAlpha)) / alpha);
    g = iRgbSqrt(((g * g * a) + (in.g * in.g * inAlpha)) / alpha);
    b = iRgbSqrt(((b * b * a) + (in.b * in.b * inAlpha)) / alpha);
    a = alpha;
    return *this;
}

Hsv::Hsv(const Rgb& r) {
    int min = std::min(r.r, std::min(r.g, r.b));
    int max = std::max(r.r, std::max(r.g, r.b));
    int chroma = max - min;

    v = max;
    if (chroma == 0) {
        h = s = 0;
        return;
    }

    s = up(chroma) / max;
    int hh;
    if (max == r.r)
        hh = (up(int(r.g) - int(r.b))) / chroma / 6;
    else if (max == r.g)
        hh = 255 / 3 + (up(int(r.b) - int(r.r))) / chroma / 6;
    else
        hh = 2 * 255 / 3 + (up(int(r.r) - int(r.g))) / chroma / 6;

    if (hh < 0)
        hh += 255;
    h = hh;

    a = r.a;
}

Hsv& Hsv::operator=(const Rgb& rgb) {
    Hsv h { rgb };
    swap(h);
    return *this;
}
