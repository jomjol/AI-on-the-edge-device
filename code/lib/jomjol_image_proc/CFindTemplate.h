#pragma once


#ifndef __CFINDTEMPLATE
#define __CFINGTEMPLATE

#include <stdint.h>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"


class CImageBasis
{
    protected:
        uint8_t* rgb_image;
        int channels;
        int width, height, bpp; 
        bool externalImage;
        std::string filename;

        void memCopy(uint8_t* _source, uint8_t* _target, int _size);
        bool isInImage(int x, int y);

    public:
        int getWidth(){return this->width;};   
        int getHeight(){return this->height;};   
        int getChannels(){return this->channels;};   
        void drawRect(int x, int y, int dx, int dy, int r = 255, int g = 255, int b = 255, int thickness = 1);
        void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int thickness = 1);
        void drawCircle(int x1, int y1, int rad, int r, int g, int b, int thickness = 1);
        void setPixelColor(int x, int y, int r, int g, int b);
        void Contrast(float _contrast);
        bool ImageOkay();


        CImageBasis();
        CImageBasis(std::string _image);
        CImageBasis(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp);
        uint8_t GetPixelColor(int x, int y, int ch);

        ~CImageBasis();

        void SaveToFile(std::string _imageout);
};

class CFindTemplate : public CImageBasis
{
    public:
        int tpl_width, tpl_height, tpl_bpp;    
        CFindTemplate(std::string _image);

        void FindTemplate(std::string _template, int* found_x, int* found_y, std::string _imageout);
        void FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy, std::string _imageout);
        void FindTemplate(std::string _template, int* found_x, int* found_y);
        void FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy);
};

class CRotate: public CImageBasis
{
    public:
        CRotate(std::string _image) : CImageBasis(_image) {};
        CRotate(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {};

        void Rotate(float _angle);
        void Rotate(float _angle, int _centerx, int _centery);
        void Translate(int _dx, int _dy);
        void Mirror();
};


class CAlignAndCutImage : public CImageBasis
{
    public:
        int t0_dx, t0_dy, t1_dx, t1_dy;
        CAlignAndCutImage(std::string _image) : CImageBasis(_image) {};

        void Align(std::string _template1, int x1, int y1, std::string _template2, int x2, int y2, int deltax = 40, int deltay = 40, std::string imageROI = "");
        void CutAndSave(std::string _template1, int x1, int y1, int dx, int dy);
};


class CResizeImage : public CImageBasis
{
public:
    CResizeImage(std::string _image) : CImageBasis(_image) {};
//    CResizeImage(std::string _image, int _new_dx, int _new_dy);
    void Resize(int _new_dx, int _new_dy);
};


#endif

