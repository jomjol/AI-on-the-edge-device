#include <string>
#include "CRotateImage.h"
#include "psram.h"

static const char *TAG = "C ROTATE IMG";

CRotateImage::CRotateImage(std::string _name, CImageBasis *_org, CImageBasis *_temp, bool _flip) : CImageBasis(_name)
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


void CRotateImage::Mirror(){
    int memsize = width * height * channels;
    uint8_t* odata;
    if (ImageTMP)
    {
        odata = ImageTMP->RGBImageLock();
    }
    else
    {
        odata = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->odata", memsize, MALLOC_CAP_SPIRAM);
    }


    int x_source, y_source;
    stbi_uc* p_target;
    stbi_uc* p_source;

    RGBImageLock();

    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_target = odata + (channels * (y * width + x));

            x_source = width - x;
            y_source = y;

            p_source = rgb_image + (channels * (y_source * width + x_source));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

    //    memcpy(rgb_image, odata, memsize);
    memCopy(odata, rgb_image, memsize);
    if (!ImageTMP)
        free_psram_heap(std::string(TAG) + "->odata", odata);

    if (ImageTMP)
        ImageTMP->RGBImageRelease();

    RGBImageRelease();
}

void CRotateImage::Rotate(float _angle, int _centerx, int _centery)
{
    int org_width, org_height;
    float m[2][3];

    float x_center = _centerx;
    float y_center = _centery;
    _angle = _angle / 180 * M_PI;

    if (doflip)
    {
        org_width = width;
        org_height = height;
        height = org_width;
        width = org_height;
        x_center =  x_center - (org_width/2) + (org_height/2);
        y_center =  y_center + (org_width/2) - (org_height/2);
        if (ImageOrg)
        {
            ImageOrg->height = height;
            ImageOrg->width = width;
        }
    }
    else
    {
        org_width = width;
        org_height = height;
    }

    m[0][0] = cos(_angle);
    m[0][1] = sin(_angle);
    m[0][2] = (1 - m[0][0]) * x_center - m[0][1] * y_center;

    m[1][0] = -m[0][1];
    m[1][1] = m[0][0];
    m[1][2] = m[0][1] * x_center + (1 - m[0][0]) * y_center;

    if (doflip)
    {
        m[0][2] = m[0][2] + (org_width/2) - (org_height/2);
        m[1][2] = m[1][2] - (org_width/2) + (org_height/2);
    }

    int memsize = width * height * channels;
    uint8_t* odata;
    if (ImageTMP)
    {
        odata = ImageTMP->RGBImageLock();
    }
    else
    {
        odata = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->odata", memsize, MALLOC_CAP_SPIRAM);
    }
    

    int x_source, y_source;
    stbi_uc* p_target;
    stbi_uc* p_source;

    RGBImageLock();

    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_target = odata + (channels * (y * width + x));

            x_source = int(m[0][0] * x + m[0][1] * y);
            y_source = int(m[1][0] * x + m[1][1] * y);

            x_source += int(m[0][2]);
            y_source += int(m[1][2]);

            if ((x_source >= 0) && (x_source < org_width) && (y_source >= 0) && (y_source < org_height))
            {
                p_source = rgb_image + (channels * (y_source * org_width + x_source));
                for (int _channels = 0; _channels < channels; ++_channels)
                    p_target[_channels] = p_source[_channels];
            }
            else
            {
                for (int _channels = 0; _channels < channels; ++_channels)
                    p_target[_channels] = 255;
            }
        }

    //    memcpy(rgb_image, odata, memsize);
    memCopy(odata, rgb_image, memsize);

    if (!ImageTMP)
    {
        free_psram_heap(std::string(TAG) + "->odata", odata);
    }
    if (ImageTMP)
        ImageTMP->RGBImageRelease();

    RGBImageRelease();
}



void CRotateImage::RotateAntiAliasing(float _angle, int _centerx, int _centery)
{
    int org_width, org_height;
    float m[2][3];

    float x_center = _centerx;
    float y_center = _centery;
    _angle = _angle / 180 * M_PI;

    if (doflip)
    {
        org_width = width;
        org_height = height;
        height = org_width;
        width = org_height;
        x_center =  x_center - (org_width/2) + (org_height/2);
        y_center =  y_center + (org_width/2) - (org_height/2);
        if (ImageOrg)
        {
            ImageOrg->height = height;
            ImageOrg->width = width;
        }
    }
    else
    {
        org_width = width;
        org_height = height;
    }

    m[0][0] = cos(_angle);
    m[0][1] = sin(_angle);
    m[0][2] = (1 - m[0][0]) * x_center - m[0][1] * y_center;

    m[1][0] = -m[0][1];
    m[1][1] = m[0][0];
    m[1][2] = m[0][1] * x_center + (1 - m[0][0]) * y_center;

    if (doflip)
    {
        m[0][2] = m[0][2] + (org_width/2) - (org_height/2);
        m[1][2] = m[1][2] - (org_width/2) + (org_height/2);
    }

    int memsize = width * height * channels;
    uint8_t* odata;
    if (ImageTMP)
    {
        odata = ImageTMP->RGBImageLock();
    }
    else
    {
        odata = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->odata", memsize, MALLOC_CAP_SPIRAM);
    }
    

    int x_source_1, y_source_1, x_source_2, y_source_2;
    float x_source, y_source;
    float quad_ul, quad_ur, quad_ol, quad_or;
    stbi_uc* p_target;
    stbi_uc *p_source_ul, *p_source_ur, *p_source_ol, *p_source_or;

    RGBImageLock();

    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_target = odata + (channels * (y * width + x));

            x_source = (m[0][0] * x + m[0][1] * y);
            y_source = (m[1][0] * x + m[1][1] * y);

            x_source += (m[0][2]);
            y_source += (m[1][2]);

            x_source_1 = (int)x_source;
            x_source_2 = x_source_1 + 1;
            y_source_1 = (int)y_source;
            y_source_2 = y_source_1 + 1;

            quad_ul = (x_source_2 - x_source) * (y_source_2 - y_source);
            quad_ur = (1- (x_source_2 - x_source)) * (y_source_2 - y_source);
            quad_or = (x_source_2 - x_source) * (1-(y_source_2 - y_source));
            quad_ol = (1- (x_source_2 - x_source)) * (1-(y_source_2 - y_source));


            if ((x_source_1 >= 0) && (x_source_2 < org_width) && (y_source_1 >= 0) && (y_source_2 < org_height))
            {
                p_source_ul = rgb_image + (channels * (y_source_1 * org_width + x_source_1));
                p_source_ur = rgb_image + (channels * (y_source_1 * org_width + x_source_2));
                p_source_or = rgb_image + (channels * (y_source_2 * org_width + x_source_1));
                p_source_ol = rgb_image + (channels * (y_source_2 * org_width + x_source_2));
                for (int _channels = 0; _channels < channels; ++_channels)
                {
                    p_target[_channels] = (int)((float)p_source_ul[_channels] * quad_ul
                                                + (float)p_source_ur[_channels] * quad_ur
                                                + (float)p_source_or[_channels] * quad_or
                                                + (float)p_source_ol[_channels] * quad_ol);
                }
            }
            else
            {
                for (int _channels = 0; _channels < channels; ++_channels)
                    p_target[_channels] = 255;
            }
        }

    //    memcpy(rgb_image, odata, memsize);
    memCopy(odata, rgb_image, memsize);

    if (!ImageTMP)
    {
        free_psram_heap(std::string(TAG) + "->odata", odata);
    }
    if (ImageTMP)
        ImageTMP->RGBImageRelease();

    RGBImageRelease();
}


void CRotateImage::Rotate(float _angle)
{
//    ESP_LOGD(TAG, "width %d, height %d", width, height);
    Rotate(_angle, width / 2, height / 2);
}

void CRotateImage::RotateAntiAliasing(float _angle)
{
//    ESP_LOGD(TAG, "width %d, height %d", width, height);
    RotateAntiAliasing(_angle, width / 2, height / 2);
}

void CRotateImage::Translate(int _dx, int _dy)
{
    int memsize = width * height * channels;
    uint8_t* odata;
    if (ImageTMP)
    {
        odata = ImageTMP->RGBImageLock();
    }
    else
    {
        odata = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->odata", memsize, MALLOC_CAP_SPIRAM);
    }



    int x_source, y_source;
    stbi_uc* p_target;
    stbi_uc* p_source;

    RGBImageLock();

    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_target = odata + (channels * (y * width + x));

            x_source = x - _dx;
            y_source = y - _dy;

            if ((x_source >= 0) && (x_source < width) && (y_source >= 0) && (y_source < height))
            {
                p_source = rgb_image + (channels * (y_source * width + x_source));
                for (int _channels = 0; _channels < channels; ++_channels)
                    p_target[_channels] = p_source[_channels];
            }
            else
            {
                for (int _channels = 0; _channels < channels; ++_channels)
                    p_target[_channels] = 255;
            }
        }

    //    memcpy(rgb_image, odata, memsize);
    memCopy(odata, rgb_image, memsize);
    if (!ImageTMP)
    {
        free_psram_heap(std::string(TAG) + "->odata", odata);
    }

    if (ImageTMP)
    {
        ImageTMP->RGBImageRelease();
    }
    RGBImageRelease();

}

