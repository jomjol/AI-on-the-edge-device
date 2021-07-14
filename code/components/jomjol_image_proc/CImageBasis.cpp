#include "CImageBasis.h"
#include "Helper.h"
#include "ClassLogFile.h"

#include <esp_log.h>

#include "esp_system.h"

#include <cstring>


#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#define _ESP32_PSRAM

using namespace std;

static const char *TAG = "CImageBasis";

//#define DEBUG_DETAIL_ON   



uint8_t * CImageBasis::RGBImageLock(int _waitmaxsec)
{
    if (islocked)
    {
#ifdef DEBUG_DETAIL_ON   
        printf("Image is locked: sleep for : %ds\n", _waitmaxsec);
#endif
        TickType_t xDelay;
        xDelay = 1000 / portTICK_PERIOD_MS;
        for (int i = 0; i <= _waitmaxsec; ++i)
        {
            vTaskDelay( xDelay ); 
            if (!islocked)
                break;
        }
    }

    if (islocked)
        return NULL;

    return rgb_image;
}

void CImageBasis::RGBImageRelease()
{
    islocked = false;
}

uint8_t * CImageBasis::RGBImageGet()
{
    return rgb_image;
}



void writejpghelp(void *context, void *data, int size)
{
//    printf("Size all: %d, size %d\n", ((ImageData*)context)->size, size);
    ImageData* _zw = (ImageData*) context;
    uint8_t *voidstart = _zw->data;
    uint8_t *datastart = (uint8_t*) data;
    voidstart += _zw->size;

    for (int i = 0; i < size; ++i)
        *(voidstart + i) = *(datastart + i);

    _zw->size += size;
}




ImageData* CImageBasis::writeToMemoryAsJPG(const int quality)
{
    ImageData* ii = new ImageData;

    RGBImageLock();
    stbi_write_jpg_to_func(writejpghelp, ii, width, height, channels, rgb_image, quality);
    RGBImageRelease();

    return ii;
}

#define HTTP_BUFFER_SENT 1024

struct SendJPGHTTP
{
    httpd_req_t *req;
    esp_err_t res;
    char buf[HTTP_BUFFER_SENT];
    int size = 0;
};

inline void writejpgtohttphelp(void *context, void *data, int size)
{
    SendJPGHTTP* _send = (SendJPGHTTP*) context;
    if ((_send->size + size) >= HTTP_BUFFER_SENT)     // data passt nich mehr in buffer
    {
        httpd_req_t *_req = _send->req;
        if (httpd_resp_send_chunk(_req, _send->buf, _send->size) != ESP_OK) 
        {
                    ESP_LOGE(TAG, "File sending failed!");
                    _send->res = ESP_FAIL;  
        }
        _send->size = 0;      
    }
    std::memcpy((void*) (&(_send->buf[0]) + _send->size), data, size);
    _send->size+= size;
} 



esp_err_t CImageBasis::SendJPGtoHTTP(httpd_req_t *_req, const int quality)
{
    SendJPGHTTP ii;
    ii.req = _req;
    ii.res = ESP_OK;
    ii.size = 0;

    RGBImageLock();
    stbi_write_jpg_to_func(writejpgtohttphelp, &ii, width, height, channels, rgb_image, quality);
    RGBImageRelease();

    if (ii.size > 0)
    {
        if (httpd_resp_send_chunk(_req, (char*) ii.buf, ii.size) != ESP_OK)             // verschicke noch den Rest
        {
            ESP_LOGE(TAG, "File sending failed!");
            ii.res = ESP_FAIL;  
        }
    }

    return ii.res;
}  



bool CImageBasis::CopyFromMemory(uint8_t* _source, int _size)
{
    int gr = height * width * channels;
    if (gr != _size)            // Größe passt nicht
    {
        printf("Kann Bild nicht von Speicher kopierte - Größen passen nicht zusammen: soll %d, ist %d\n", _size, gr);
        return false;
    }

    RGBImageLock();
    memCopy(_source, rgb_image, _size);
    RGBImageRelease();

    return true;
}

uint8_t CImageBasis::GetPixelColor(int x, int y, int ch)
{
    stbi_uc* p_source;
    p_source = rgb_image + (channels * (y * width + x));
    return p_source[ch];
}


void CImageBasis::memCopy(uint8_t* _source, uint8_t* _target, int _size)
{
#ifdef _ESP32_PSRAM
    for (int i = 0; i < _size; ++i)
        *(_target + i) = *(_source + i);
#else
    memcpy(_target, _source, _size);
#endif
}

bool CImageBasis::isInImage(int x, int y)
{
    if ((x < 0) || (x > width - 1))
        return false;

    if ((y < 0) || (y > height- 1))
        return false;

    return true;
}

void CImageBasis::setPixelColor(int x, int y, int r, int g, int b)
{
    stbi_uc* p_source;

    RGBImageLock();
    p_source = rgb_image + (channels * (y * width + x));
    p_source[0] = r;
    if ( channels > 2)
    {
        p_source[1] = g;
        p_source[2] = b;
    }
    RGBImageRelease();
}

void CImageBasis::drawRect(int x, int y, int dx, int dy, int r, int g, int b, int thickness)
{
    int zwx1, zwx2, zwy1, zwy2;
    int _x, _y, _thick;

    zwx1 = x - thickness + 1;
    zwx2 = x + dx + thickness - 1;
    zwy1 = y;
    zwy2 = y;
    for (_thick = 0; _thick < thickness; _thick++)
        for (_x = zwx1; _x <= zwx2; ++_x)
            for (_y = zwy1; _y <= zwy2; _y++)
                if (isInImage(_x, _y))
                    setPixelColor(_x, _y - _thick, r, g, b);

    zwx1 = x - thickness + 1;
    zwx2 = x + dx + thickness - 1;
    zwy1 = y + dy;
    zwy2 = y + dy;
    for (_thick = 0; _thick < thickness; _thick++)
        for (_x = zwx1; _x <= zwx2; ++_x)
            for (_y = zwy1; _y <= zwy2; _y++)
                if (isInImage(_x, _y))
                    setPixelColor(_x, _y + _thick, r, g, b);

    zwx1 = x;
    zwx2 = x;
    zwy1 = y;
    zwy2 = y + dy;
    for (_thick = 0; _thick < thickness; _thick++)
        for (_x = zwx1; _x <= zwx2; ++_x)
            for (_y = zwy1; _y <= zwy2; _y++)
                if (isInImage(_x, _y))
                    setPixelColor(_x - _thick, _y, r, g, b);

    zwx1 = x + dx;
    zwx2 = x + dx;
    zwy1 = y;
    zwy2 = y + dy;
    for (_thick = 0; _thick < thickness; _thick++)
        for (_x = zwx1; _x <= zwx2; ++_x)
            for (_y = zwy1; _y <= zwy2; _y++)
                if (isInImage(_x, _y))
                    setPixelColor(_x + _thick, _y, r, g, b);

}

void CImageBasis::drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int thickness)
{
    int _x, _y, _thick;
    int _zwy1, _zwy2;
    thickness = (thickness-1) / 2;

    for (_thick = 0; _thick <= thickness; ++_thick)
        for (_x = x1 - _thick; _x <= x2 + _thick; ++_x)
        {
            if (x2 == x1)
            {
                _zwy1 = y1;
                _zwy2 = y2;
            }
            else
            {
                _zwy1 = (y2 - y1) * (float)(_x - x1) / (float)(x2 - x1) + y1;
                _zwy2 = (y2 - y1) * (float)(_x + 1 - x1) / (float)(x2 - x1) + y1;
            }

            for (_y = _zwy1 - _thick; _y <= _zwy2 + _thick; _y++)
                if (isInImage(_x, _y))
                    setPixelColor(_x, _y, r, g, b);
        }
}

void CImageBasis::drawCircle(int x1, int y1, int rad, int r, int g, int b, int thickness)
{
    float deltarad, aktrad;
    int _thick, _x, _y;

    deltarad = 1 / (4 * M_PI * (rad + thickness - 1));

    for (aktrad = 0; aktrad <= (2 * M_PI); aktrad += deltarad)
        for (_thick = 0; _thick < thickness; ++_thick)
        {
            _x = sin(aktrad) * (rad + _thick) + x1;
            _y = cos(aktrad) * (rad + _thick) + y1;
            if (isInImage(_x, _y))
                setPixelColor(_x, _y, r, g, b);
        }
}

CImageBasis::CImageBasis()
{
    externalImage = false;
    rgb_image = NULL;
    width = 0;
    height = 0;
    channels = 0;    
    islocked = false;
}

void CImageBasis::CreateEmptyImage(int _width, int _height, int _channels)
{
    bpp = _channels;
    width = _width;
    height = _height;
    channels = _channels;

    RGBImageLock();


    int memsize = width * height * channels;
    rgb_image = (unsigned char*)GET_MEMORY(memsize);


    stbi_uc* p_source;    

    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_source = rgb_image + (channels * (y * width + x));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_source[_channels] = (uint8_t) 0;
        }

    RGBImageRelease();


}

void CImageBasis::LoadFromMemory(stbi_uc *_buffer, int len)
{
    RGBImageLock();
    if (rgb_image)
        stbi_image_free(rgb_image);
    rgb_image = stbi_load_from_memory(_buffer, len, &width, &height, &channels, 3);
    bpp = channels;
    printf("Image loaded from memory: %d, %d, %d\n", width, height, channels);
    RGBImageRelease();
}

CImageBasis::CImageBasis(CImageBasis *_copyfrom, int _anzrepeat) 
{
    islocked = false;
    externalImage = false;
    channels = _copyfrom->channels;
    width = _copyfrom->width;
    height = _copyfrom->height;
    bpp = _copyfrom->bpp;

    RGBImageLock();

    int memsize = width * height * channels;
    rgb_image = (unsigned char*)GET_MEMORY(memsize);

    int anz = 1;
    while (!rgb_image && (anz < _anzrepeat))    
    {
	    printf("Create Image from Copy - Speicher ist voll - Versuche es erneut: %d.\n", anz);
        rgb_image = (unsigned char*) malloc(memsize);
        anz++;
    }

    
    if (!rgb_image)
    {
        printf(getESPHeapInfo().c_str());
        printf("\nKein freier Speicher mehr!!!! Benötigt: %d %d %d %d\n", width, height, channels, memsize);
        RGBImageRelease();
        return;
    }

    memCopy(_copyfrom->rgb_image, rgb_image, memsize);
    RGBImageRelease();
}

CImageBasis::CImageBasis(int _width, int _height, int _channels)
{
    islocked = false;
    externalImage = false;
    channels = _channels;
    width = _width;
    height = _height;
    bpp = _channels;

    int memsize = width * height * channels;

    rgb_image = (unsigned char*)GET_MEMORY(memsize);
    if (!rgb_image)
    {
        printf(getESPHeapInfo().c_str());
        printf("\nKein freier Speicher mehr!!!! Benötigt: %d %d %d %d\n", width, height, channels, memsize);
        return;
    }
}


CImageBasis::CImageBasis(std::string _image)
{
    islocked = false;
    channels = 3;
    externalImage = false;
    filename = _image;
    long zwld = esp_get_free_heap_size();
    printf("freeheapsize before: %ld\n", zwld);

    RGBImageLock();
    rgb_image = stbi_load(_image.c_str(), &width, &height, &bpp, channels);
    RGBImageRelease();

    zwld = esp_get_free_heap_size();
    printf("freeheapsize after : %ld\n", zwld);

    std::string zw = "Image Load failed:" + _image + "\n";
    if (rgb_image == NULL)
        printf(zw.c_str());
    zw = "CImageBasis after load " + _image + "\n";
    printf(zw.c_str());
    printf("w %d, h %d, b %d, c %d\n", width, height, bpp, channels);

}

bool CImageBasis::ImageOkay(){
    return rgb_image != NULL;
}

CImageBasis::CImageBasis(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp)
{
    islocked = false;
    rgb_image = _rgb_image;
    channels = _channels;
    width = _width;
    height = _height;
    bpp = _bpp;
    externalImage = true;
}

void CImageBasis::Contrast(float _contrast)  //input range [-100..100]
{
    stbi_uc* p_source;
    
    float contrast = (_contrast/100) + 1;  //convert to decimal & shift range: [0..2]
    float intercept = 128 * (1 - contrast);

    RGBImageLock();


    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_source = rgb_image + (channels * (y * width + x));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_source[_channels] = (uint8_t) std::min(255, std::max(0, (int) (p_source[_channels] * contrast + intercept)));
        }

    RGBImageRelease();
}

CImageBasis::~CImageBasis()
{
    RGBImageLock();
    if (!externalImage)
        stbi_image_free(rgb_image);
}

void CImageBasis::SaveToFile(std::string _imageout)
{
    string typ = getFileType(_imageout);

    RGBImageLock();

    if ((typ == "jpg") || (typ == "JPG"))       // ACHTUNG PROBLEMATISCH IM ESP32
    {
        stbi_write_jpg(_imageout.c_str(), width, height, channels, rgb_image, 0);
    }

    if ((typ == "bmp") || (typ == "BMP"))
    {
        stbi_write_bmp(_imageout.c_str(), width, height, channels, rgb_image);
    }
    RGBImageRelease();
}


void CImageBasis::Resize(int _new_dx, int _new_dy)
{
    int memsize = _new_dx * _new_dy * channels;
    uint8_t* odata = (unsigned char*)GET_MEMORY(memsize);

    RGBImageLock();


    stbir_resize_uint8(rgb_image, width, height, 0, odata, _new_dx, _new_dy, 0, channels);

    stbi_image_free(rgb_image);
    rgb_image = (unsigned char*)GET_MEMORY(memsize);

    memCopy(odata, rgb_image, memsize);
    RGBImageRelease();

    width = _new_dx;
    height = _new_dy;
    stbi_image_free(odata);
}

void CImageBasis::Resize(int _new_dx, int _new_dy, CImageBasis *_target)
{
    if ((_target->height != _new_dy) || (_target->width != _new_dx) || (_target->channels != channels))
    {
        printf("CImageBasis::Resize - Targetbildgröße passt nicht !!!!!!!!!");
        return;
    }

    RGBImageLock();

    uint8_t* odata = _target->rgb_image;
    stbir_resize_uint8(rgb_image, width, height, 0, odata, _new_dx, _new_dy, 0, channels);
    RGBImageRelease();
}

