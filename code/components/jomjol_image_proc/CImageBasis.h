#pragma once

#ifndef __CIMAGEBASIS
#define __CIMAGEBASIS

#include <stdint.h>
#include <string>
#include <esp_http_server.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

#include "esp_heap_caps.h"

//#define GET_MEMORY malloc
#define GET_MEMORY(X) heap_caps_malloc(X, MALLOC_CAP_SPIRAM)


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

        bool islocked;

    public:
        uint8_t* rgb_image;
        int channels;
        int width, height, bpp; 

        uint8_t * RGBImageLock(int _waitmaxsec = 60);
        void RGBImageRelease();
        uint8_t * RGBImageGet();

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
        CImageBasis(int _width, int _height, int _channels);
        CImageBasis(CImageBasis *_copyfrom, int _anzrepeat = 0);

        void Resize(int _new_dx, int _new_dy);        
        void Resize(int _new_dx, int _new_dy, CImageBasis *_target);        

        void LoadFromMemory(stbi_uc *_buffer, int len);

        ImageData* writeToMemoryAsJPG(const int quality = 90);   

        esp_err_t SendJPGtoHTTP(httpd_req_t *req, const int quality = 90);   

        uint8_t GetPixelColor(int x, int y, int ch);

        ~CImageBasis();

        void SaveToFile(std::string _imageout);
};


#endif

