#include <string>
#include <cstring>
#include <math.h>
#include <algorithm>

#include "CRotateImage.h"
#include "psram.h"
#include "Helper.h"

static const char* TAG = "CRotateImage";

static const uint8_t FILL_VALUE = 0; // black

CRotateImage::CRotateImage(std::string _name, CImageBasis* _org, CImageBasis* _temp, bool _flip) : CImageBasis(_name)
{
    rgb_image = _org->rgb_image;
    channels = _org->channels;
    width = _org->width;
    height = _org->height;
    bpp = _org->bpp;
    externalImage = true;
    ImageTMP = _temp;
    ImageOrg = _org;
    islocked = false;
    doflip = _flip;
}

void CRotateImage::ImageTransform(float _angle, int _centerx, int _centery, int _dx, int _dy, bool antiAliasing)
{
    if ((_angle == 0.0f) && (_dx == 0) && (_dy == 0))
    {
        return;
    }

    int org_width = 0, org_height = 0;
    float m[2][3];
    float x_center = _centerx;
    float y_center = _centery;

    _angle = _angle / 180 * M_PI; // in Radian umrechnen

    org_width = width;
    org_height = height;

    // Transform-Matrix
    m[0][0] = cos(_angle);
    m[0][1] = sin(_angle);
    m[0][2] = (1 - m[0][0]) * x_center - m[0][1] * y_center;

    m[1][0] = -m[0][1];
    m[1][1] = m[0][0];
    m[1][2] = m[0][1] * x_center + (1 - m[0][0]) * y_center;

    int memsize = width * height * channels;

    uint8_t* odata = ImageTMP ? ImageTMP->RGBImageLock() : (unsigned char*)malloc_psram_heap("CRotateImage->odata", memsize, MALLOC_CAP_SPIRAM);

    int x_source_1 = 0, y_source_1 = 0, x_source_2 = 0, y_source_2 = 0;
    float x_source = 0.0f, y_source = 0.0f;
    float quad_ul = 0.0f, quad_ur = 0.0f, quad_ol = 0.0f, quad_or = 0.0f;

    uint8_t* p_target;
    uint8_t *p_source, *p_source_ul, *p_source_ur, *p_source_or, *p_source_ol;

    RGBImageLock();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            p_target = odata + (channels * (y * width + x));

            // Translation + Rotation
            x_source = m[0][0] * x + m[0][1] * y - _dx;
            y_source = m[1][0] * x + m[1][1] * y - _dy;

            x_source += m[0][2];
            y_source += m[1][2];

            if (!antiAliasing)
            {
                x_source_1 = int(x_source);
                y_source_1 = int(y_source);

                if ((x_source_1 >= 0) && (x_source_1 < org_width) && (y_source_1 >= 0) && (y_source_1 < org_height))
                {
                    p_source = rgb_image + (channels * (y_source_1 * org_width + x_source_1));
                    for (int _channels = 0; _channels < channels; ++_channels)
                    {
                        p_target[_channels] = p_source[_channels];
                    }
                }
                else
                {
                    for (int _channels = 0; _channels < channels; ++_channels)
                    {
                        p_target[_channels] = FILL_VALUE;
                    }
                }
            }
            else
            {
                // Anti-Aliasing (Bilinear)
                x_source_1 = (int)x_source;
                x_source_2 = x_source_1 + 1;
                y_source_1 = (int)y_source;
                y_source_2 = y_source_1 + 1;

                quad_ul = (x_source_2 - x_source) * (y_source_2 - y_source);
                quad_ur = (1 - (x_source_2 - x_source)) * (y_source_2 - y_source);
                quad_or = (x_source_2 - x_source) * (1 - (y_source_2 - y_source));
                quad_ol = (1 - (x_source_2 - x_source)) * (1 - (y_source_2 - y_source));

                if ((x_source_1 >= 0) && (x_source_2 < org_width) && (y_source_1 >= 0) && (y_source_2 < org_height))
                {
                    p_source_ul = rgb_image + (channels * (y_source_1 * org_width + x_source_1));
                    p_source_ur = rgb_image + (channels * (y_source_1 * org_width + x_source_2));
                    p_source_or = rgb_image + (channels * (y_source_2 * org_width + x_source_1));
                    p_source_ol = rgb_image + (channels * (y_source_2 * org_width + x_source_2));

                    for (int _channels = 0; _channels < channels; ++_channels)
                    {
                        p_target[_channels] = (int)((float)p_source_ul[_channels] * quad_ul + (float)p_source_ur[_channels] * quad_ur + (float)p_source_or[_channels] * quad_or + (float)p_source_ol[_channels] * quad_ol);
                    }
                }
                else
                {
                    for (int _channels = 0; _channels < channels; ++_channels)
                    {
                        p_target[_channels] = FILL_VALUE;
                    }
                }
            }
        }
    }

    std::memcpy(rgb_image, odata, memsize);
    ImageTMP ? ImageTMP->RGBImageRelease() : free_psram_heap("CRotateImage->odata", odata);

    RGBImageRelease();
}
