#pragma once

#ifndef CIMAGEBASIS_H
#define CIMAGEBASIS_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <esp_http_server.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "defines.h"

#include <math.h>

#include "../stb/stb_image.h"
#include "../stb/stb_image_write.h"
#include "../stb/stb_image_resize.h"

#include "esp_heap_caps.h"

struct ImageData
{
    uint8_t data[MAX_JPG_SIZE];
    size_t size = 0;
    bool tooLarge = false;
};

class CImageBasis
{
protected:
    enum class BufferOwner : uint8_t
    {
        None,
        PsramHeap,
        StbImage,
        SharedRegion,
        External
    };

    std::string filename;
    std::string name; // Just used for diagnostics
    size_t memsize = 0;

    BufferOwner bufferOwner = BufferOwner::None;
    SemaphoreHandle_t imageMutex = nullptr;

    bool ownsImageMutex = true;

    void UseImageMutexFrom(CImageBasis *owner);
    void memCopy(uint8_t *_source, uint8_t *_target, int _size);
    bool isInImage(int x, int y);
    void ReleaseImageBuffer();
    bool LockImageMutex(int waitmaxsec = 60);
    bool CalculateImageSize(int imageWidth, int imageHeight, int imageChannels, size_t *result) const;

public:
    uint8_t *rgb_image = NULL;

    int channels;
    int width, height, bpp;

    uint8_t *RGBImageLock(int _waitmaxsec = 60);
    void RGBImageRelease();
    uint8_t *RGBImageGet();

    int getWidth() { return width; };
    int getHeight() { return height; };
    int getChannels() { return channels; };

    void drawRect(int x, int y, int dx, int dy, int r = 255, int g = 255, int b = 255, int thickness = 1);
    void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int thickness = 1);
    void drawCircle(int x1, int y1, int rad, int r, int g, int b, int thickness = 1);
    void drawEllipse(int x1, int y1, int radx, int rady, int r, int g, int b, int thickness = 1);

    void setPixelColor(int x, int y, int r, int g, int b);
    void Negative(void);
    void Contrast(float _contrast);
    bool ImageOkay();
    bool CopyFromMemory(uint8_t *_source, int _size);

    // Adopt a buffer allocated with malloc_psram_heap(). Kept under the
    // historic name because existing image-processing code calls it.
    void SetIndepended();

    void CreateEmptyImage(int _width, int _height, int _channels);
    void EmptyImage();

    CImageBasis(std::string name);
    CImageBasis(std::string name, std::string _image);
    CImageBasis(std::string name, uint8_t *_rgb_image, int _channels, int _width, int _height, int _bpp);
    CImageBasis(std::string name, int _width, int _height, int _channels);
    CImageBasis(std::string name, CImageBasis *_copyfrom, bool useSharedRegion = false);

    void Resize(int _new_dx, int _new_dy);
    void Resize(int _new_dx, int _new_dy, CImageBasis *_target);
    void crop_image(unsigned short cropLeft, unsigned short cropRight, unsigned short cropTop, unsigned short cropBottom);

    void LoadFromMemory(stbi_uc *_buffer, int len);

    ImageData *writeToMemoryAsJPG(const int quality = 90);
    void writeToMemoryAsJPG(ImageData *ii, const int quality = 90);

    esp_err_t SendJPGtoHTTP(httpd_req_t *req, const int quality = 90);

    uint8_t GetPixelColor(int x, int y, int ch);

    ~CImageBasis();

    void SaveToFile(std::string _imageout);
};

#endif // CIMAGEBASIS_H
