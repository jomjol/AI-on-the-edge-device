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


#define MAX_JPG_SIZE 128000

struct ImageData
{
    uint8_t data[MAX_JPG_SIZE];
    size_t size = 0;
};


class CImageBasis
{
    protected:
        bool externalImage;
        std::string filename;

        void memCopy(uint8_t* _source, uint8_t* _target, int _size);
        bool isInImage(int x, int y);

    public:
        uint8_t* rgb_image;
        int channels;
        int width, height, bpp; 

        int getWidth(){return this->width;};   
        int getHeight(){return this->height;};   
        int getChannels(){return this->channels;};   
        void drawRect(int x, int y, int dx, int dy, int r = 255, int g = 255, int b = 255, int thickness = 1);
        void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int thickness = 1);
        void drawCircle(int x1, int y1, int rad, int r, int g, int b, int thickness = 1);
        void setPixelColor(int x, int y, int r, int g, int b);
        void Contrast(float _contrast);
        bool ImageOkay();
        bool CopyFromMemory(uint8_t* _source, int _size);

        void SetIndepended(){externalImage = false;};

        void CreateEmptyImage(int _width, int _height, int _channels);


        CImageBasis();
        CImageBasis(std::string _image);
        CImageBasis(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp);
        CImageBasis(CImageBasis *_copyfrom);

        void LoadFromMemory(stbi_uc *_buffer, int len);

        ImageData* writeToMemoryAsJPG(const int quality = 90);       

        uint8_t GetPixelColor(int x, int y, int ch);

        ~CImageBasis();

        void SaveToFile(std::string _imageout);
};

class CFindTemplate : public CImageBasis
{
    public:
        int tpl_width, tpl_height, tpl_bpp;    
//        CFindTemplate(std::string _image);
        CFindTemplate(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {};


        void FindTemplate(std::string _template, int* found_x, int* found_y, std::string _imageout);
        void FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy, std::string _imageout);
        void FindTemplate(std::string _template, int* found_x, int* found_y);
        void FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy);
};

class CRotate: public CImageBasis
{
    public:
        CImageBasis *ImageTMP;
        CRotate(std::string _image) : CImageBasis(_image) {ImageTMP = NULL;};
        CRotate(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {ImageTMP = NULL;};
        CRotate(CImageBasis *_org, CImageBasis *_temp);

        void Rotate(float _angle);
        void Rotate(float _angle, int _centerx, int _centery);
        void Translate(int _dx, int _dy);
        void Mirror();
};


class CResizeImage : public CImageBasis
{
public:
    CResizeImage(std::string _image) : CImageBasis(_image) {};
    CResizeImage(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {};

//    CResizeImage(std::string _image, int _new_dx, int _new_dy);
    void Resize(int _new_dx, int _new_dy);
};



class CAlignAndCutImage : public CImageBasis
{
    public:
        int t0_dx, t0_dy, t1_dx, t1_dy;
        CImageBasis *ImageTMP;
        CAlignAndCutImage(std::string _image) : CImageBasis(_image) {ImageTMP = NULL;};
        CAlignAndCutImage(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {ImageTMP = NULL;};
        CAlignAndCutImage(CImageBasis *_org, CImageBasis *_temp);

        void Align(std::string _template1, int x1, int y1, std::string _template2, int x2, int y2, int deltax = 40, int deltay = 40, std::string imageROI = "");
        void CutAndSave(std::string _template1, int x1, int y1, int dx, int dy);
        CResizeImage* CutAndSave(int x1, int y1, int dx, int dy);
        void GetRefSize(int *ref_dx, int *ref_dy);
};


#endif

