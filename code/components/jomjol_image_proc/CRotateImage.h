#pragma once

#ifndef CROTATEIMAGE_H
#define CROTATEIMAGE_H

#include <stdint.h>
#include <string>

#include "CImageBasis.h"

class CRotateImage : public CImageBasis
{
public:
    CImageBasis *ImageTMP, *ImageOrg;
    bool doflip;
    CRotateImage(std::string name, std::string _image, bool _flip = false) : CImageBasis(name, _image)
    {
        ImageTMP = NULL;
        ImageOrg = NULL;
        doflip = _flip;
    };
    CRotateImage(std::string name, uint8_t *_rgb_image, int _channels, int _width, int _height, int _bpp, bool _flip = false) : CImageBasis(name, _rgb_image, _channels, _width, _height, _bpp)
    {
        ImageTMP = NULL;
        ImageOrg = NULL;
        doflip = _flip;
    };
    CRotateImage(std::string name, CImageBasis *_org, CImageBasis *_temp, bool _flip = false);

    void ImageTransform(float _angle, int _centerx, int _centery, int _dx, int _dy, bool _antiAlias);

    void ImageRotate(float _angle)
    {
        ImageTransform(_angle, width / 2, height / 2, 0, 0, false);
    }
    void ImageRotate(float _angle, int _centerx, int _centery)
    {
        ImageTransform(_angle, _centerx, _centery, 0, 0, false);
    }
    void ImageRotateAntiAliasing(float _angle)
    {
        ImageTransform(_angle, width / 2, height / 2, 0, 0, true);
    }
    void ImageRotateAntiAliasing(float _angle, int _centerx, int _centery)
    {
        ImageTransform(_angle, _centerx, _centery, 0, 0, true);
    }
    void ImageTranslate(int _dx, int _dy)
    {
        ImageTransform(0, width / 2, height / 2, _dx, _dy, false);
    }
};

#endif // CROTATEIMAGE_H
