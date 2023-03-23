#pragma once

#ifndef CALIGNANDCUTIMAGE_H
#define CALIGNANDCUTIMAGE_H

#include "CImageBasis.h"
#include "CFindTemplate.h"


class CAlignAndCutImage : public CImageBasis
{
    public:
        int t0_dx, t0_dy, t1_dx, t1_dy;
        CImageBasis *ImageTMP;
        CAlignAndCutImage(std::string name, std::string _image) : CImageBasis(name, _image) {ImageTMP = NULL;};
        CAlignAndCutImage(std::string name, uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(name, _rgb_image, _channels, _width, _height, _bpp) {ImageTMP = NULL;};
        CAlignAndCutImage(std::string name, CImageBasis *_org, CImageBasis *_temp);

        bool Align(RefInfo *_temp1, RefInfo *_temp2);
//        void Align(std::string _template1, int x1, int y1, std::string _template2, int x2, int y2, int deltax = 40, int deltay = 40, std::string imageROI = "");
        void CutAndSave(std::string _template1, int x1, int y1, int dx, int dy);
        CImageBasis* CutAndSave(int x1, int y1, int dx, int dy);
        void CutAndSave(int x1, int y1, int dx, int dy, CImageBasis *_target);
        void GetRefSize(int *ref_dx, int *ref_dy);
};

#endif //CALIGNANDCUTIMAGE_H