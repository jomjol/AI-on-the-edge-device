#include "CImageBasis.h"
#include "Helper.h"
#include "psram.h"
#include "ClassLogFile.h"
#include "server_ota.h"

#include <esp_log.h>
#include "../../include/defines.h"

#include "esp_system.h"

#include <cstring>

#include <math.h>
#include <algorithm>


using namespace std;

static const char *TAG = "C IMG BASIS";

bool jpgFileTooLarge = false;   // JPG creation verfication


//#define DEBUG_DETAIL_ON


uint8_t * CImageBasis::RGBImageLock(int _waitmaxsec)
{
    if (islocked)
    {
        #ifdef DEBUG_DETAIL_ON   
                ESP_LOGD(TAG, "Image is locked: sleep for: %ds", _waitmaxsec);
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
//    ESP_LOGD(TAG, "Size all: %d, size %d", ((ImageData*)context)->size, size);
    ImageData* _zw = (ImageData*) context;
    uint8_t *voidstart = _zw->data;
    uint8_t *datastart = (uint8_t*) data;
    
    if ((_zw->size < MAX_JPG_SIZE)) {   // Abort copy to prevent buffer overflow
        voidstart += _zw->size;

        for (int i = 0; i < size; ++i)
            *(voidstart + i) = *(datastart + i);

        _zw->size += size;
    }
    else {
        jpgFileTooLarge = true;
    }
}


ImageData* CImageBasis::writeToMemoryAsJPG(const int quality)
{
    ImageData* ii = new ImageData;

    RGBImageLock();
    stbi_write_jpg_to_func(writejpghelp, ii, width, height, channels, rgb_image, quality);
    RGBImageRelease();

    if (jpgFileTooLarge) {
        jpgFileTooLarge = false;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "writeToMemoryAsJPG: Creation aborted! JPG size > preallocated buffer: " + std::to_string(MAX_JPG_SIZE));
    }
    return ii;
}


void CImageBasis::writeToMemoryAsJPG(ImageData* i, const int quality)
{
    ImageData* ii = new ImageData;
    
    RGBImageLock();
    stbi_write_jpg_to_func(writejpghelp, ii, width, height, channels, rgb_image, quality);
    RGBImageRelease();

    if (jpgFileTooLarge) {
        jpgFileTooLarge = false;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "writeToMemoryAsJPG: Creation aborted! JPG size > preallocated buffer: " + std::to_string(MAX_JPG_SIZE));
    }
    memCopy((uint8_t*) ii, (uint8_t*) i, sizeof(ImageData));
    delete ii;
}


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
    if ((_send->size + size) >= HTTP_BUFFER_SENT)     // data no longer fits in buffer
    {
        if (httpd_resp_send_chunk(_send->req, _send->buf, _send->size) != ESP_OK) 
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

    if (ii.size > 0)
    {
        if (httpd_resp_send_chunk(_req, (char*) ii.buf, ii.size) != ESP_OK)             //still send the rest
        {
            ESP_LOGE(TAG, "File sending failed!");
            ii.res = ESP_FAIL;  
        }
    }

    RGBImageRelease();

    return ii.res;
}  


bool CImageBasis::CopyFromMemory(uint8_t* _source, int _size)
{
    int gr = height * width * channels;
    if (gr != _size)            // Size does not fit
    {
        ESP_LOGE(TAG, "Cannot copy image from memory - sizes do not match: should be %d, but is %d", _size, gr);
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

    RGBImageLock();

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

    RGBImageRelease();
}


void CImageBasis::drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int thickness)
{
    int _x, _y, _thick;
    int _zwy1, _zwy2;
    thickness = (thickness-1) / 2;

    RGBImageLock();

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
    
    RGBImageRelease();
}


void CImageBasis::drawEllipse(int x1, int y1, int radx, int rady, int r, int g, int b, int thickness)
{
    float deltarad, aktrad;
    int _thick, _x, _y;
    int rad = radx;

    if (rady > radx)
        rad = rady;

    deltarad = 1 / (4 * M_PI * (rad + thickness - 1));

    RGBImageLock();

    for (aktrad = 0; aktrad <= (2 * M_PI); aktrad += deltarad)
        for (_thick = 0; _thick < thickness; ++_thick)
        {
            _x = sin(aktrad) * (radx + _thick) + x1;
            _y = cos(aktrad) * (rady + _thick) + y1;
            if (isInImage(_x, _y))
                setPixelColor(_x, _y, r, g, b);
        }

    RGBImageRelease();
}


void CImageBasis::drawCircle(int x1, int y1, int rad, int r, int g, int b, int thickness)
{
    float deltarad, aktrad;
    int _thick, _x, _y;

    deltarad = 1 / (4 * M_PI * (rad + thickness - 1));

    RGBImageLock();

    for (aktrad = 0; aktrad <= (2 * M_PI); aktrad += deltarad)
        for (_thick = 0; _thick < thickness; ++_thick)
        {
            _x = sin(aktrad) * (rad + _thick) + x1;
            _y = cos(aktrad) * (rad + _thick) + y1;
            if (isInImage(_x, _y))
                setPixelColor(_x, _y, r, g, b);
        }

    RGBImageRelease();
}


CImageBasis::CImageBasis(string _name)
{
    name = _name;
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

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CreateEmptyImage");
    #endif

    memsize = width * height * channels;

    rgb_image = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->CImageBasis (" + name + ")", memsize, MALLOC_CAP_SPIRAM);

    if (rgb_image == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CreateEmptyImage: Can't allocate enough memory: " + std::to_string(memsize));
        LogFile.WriteHeapInfo("CreateEmptyImage");
        RGBImageRelease();
        return;
    }

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


void CImageBasis::EmptyImage()
{
    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("EmptyImage");
    #endif

    stbi_uc* p_source;

    RGBImageLock();

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

    if (rgb_image != NULL) {
        stbi_image_free(rgb_image);
        //free_psram_heap(std::string(TAG) + "->rgb_image (LoadFromMemory)", rgb_image);
    }

    rgb_image = stbi_load_from_memory(_buffer, len, &width, &height, &channels, STBI_rgb);
    bpp = channels;
    ESP_LOGD(TAG, "Image loaded from memory: %d, %d, %d", width, height, channels);
    
    if ((width * height * channels) == 0)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Image with size 0 loaded --> reboot to be done! "
                "Check that your camera module is working and connected properly.");
        LogFile.WriteHeapInfo("LoadFromMemory");

        doReboot();
    }
    RGBImageRelease();
}


void CImageBasis::crop_image(unsigned short cropLeft, unsigned short cropRight, unsigned short cropTop, unsigned short cropBottom)
{
    unsigned int maxTopIndex = cropTop * width * channels;
    unsigned int minBottomIndex = ((width*height) - (cropBottom * width)) * channels;
    unsigned short maxX = width - cropRight; // In pixels
    unsigned short newWidth = width - cropLeft - cropRight;
    unsigned short newHeight = height - cropTop - cropBottom;

    unsigned int writeIndex = 0;
    // Loop over all bytes
    for (int i = 0; i < width * height * channels; i += channels) {
        // Calculate current X, Y pixel position
        int x = (i/channels) % width;

        // Crop from the top
        if (i < maxTopIndex) { continue; }

        // Crop from the bottom
        if (i > minBottomIndex) { continue; }

        // Crop from the left
        if (x <= cropLeft) { continue; }

        // Crop from the right
        if (x > maxX) { continue; }

        // If we get here, keep the pixels
        for (int c = 0; c < channels; c++) {
            rgb_image[writeIndex++] = rgb_image[i+c];
        }
    }

    // Set the new dimensions of the framebuffer for further use.
    width = newWidth;
    height = newHeight;
}


CImageBasis::CImageBasis(string _name, CImageBasis *_copyfrom) 
{
    name = _name;
    islocked = false;
    externalImage = false;
    channels = _copyfrom->channels;
    width = _copyfrom->width;
    height = _copyfrom->height;
    bpp = _copyfrom->bpp;

    RGBImageLock();

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CImageBasis_copyfrom - Start");
    #endif

    memsize = width * height * channels;


    if (name == "tmpImage") {
        rgb_image = (unsigned char*)psram_reserve_shared_tmp_image_memory();
    }
    else {
        rgb_image = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->CImageBasis (" + name + ")", memsize, MALLOC_CAP_SPIRAM);
    }

    if (rgb_image == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-Copyfrom: Can't allocate enough memory: " + std::to_string(memsize));
        LogFile.WriteHeapInfo("CImageBasis-Copyfrom");
        RGBImageRelease();
        return;
    }

    memCopy(_copyfrom->rgb_image, rgb_image, memsize);
    RGBImageRelease();

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CImageBasis_copyfrom - done");
    #endif
}


CImageBasis::CImageBasis(string _name, int _width, int _height, int _channels)
{
    name = _name;
    islocked = false;
    externalImage = false;
    channels = _channels;
    width = _width;
    height = _height;
    bpp = _channels;

    RGBImageLock();

     #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CImageBasis_width,height,ch - Start");
    #endif

    memsize = width * height * channels;

    rgb_image = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->CImageBasis (" + name + ")", memsize, MALLOC_CAP_SPIRAM);

    if (rgb_image == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-width,height,ch: Can't allocate enough memory: " + std::to_string(memsize));
        LogFile.WriteHeapInfo("CImageBasis-width,height,ch");
        RGBImageRelease();
        return;
    }

    RGBImageRelease();

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CImageBasis_width,height,ch - done");
    #endif
}


CImageBasis::CImageBasis(string _name, std::string _image)
{
    name = _name;
    islocked = false;
    channels = 3;
    externalImage = false;
    filename = _image;

    if (file_size(_image.c_str()) == 0) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, _image + " is empty!");
        return;
    }

    RGBImageLock();

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CImageBasis_image - Start");
    #endif

    rgb_image = stbi_load(_image.c_str(), &width, &height, &bpp, channels);

    if (rgb_image == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-image: Failed to load " + _image + "! Is it corrupted?");
        LogFile.WriteHeapInfo("CImageBasis-image");
        RGBImageRelease();
        return;
    }
    
    RGBImageRelease();

    #ifdef DEBUG_DETAIL_ON 
        std::string zw = "CImageBasis after load " + _image;
        ESP_LOGD(TAG, "%s", zw.c_str());
        ESP_LOGD(TAG, "w %d, h %d, b %d, c %d", width, height, bpp, channels);
    #endif

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CImageBasis_image - done");
    #endif
}


bool CImageBasis::ImageOkay(){
    return rgb_image != NULL;
}


CImageBasis::CImageBasis(string _name, uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp)
{
    name = _name;
    islocked = false;
    rgb_image = _rgb_image;
    channels = _channels;
    width = _width;
    height = _height;
    bpp = _bpp;
    externalImage = true;
}


void CImageBasis::Negative(void)
{
    RGBImageLock();

    for (int i = 0; i < width * height * channels; i += channels) {
        for (int c = 0; c < channels; c++) {
            rgb_image[i+c] = 255 - rgb_image[i+c];
        }
    }

    RGBImageRelease();
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


    if (!externalImage) {
        if (name == "tmpImage") { // This image should be placed in the shared part of PSRAM
            psram_free_shared_temp_image_memory();
        }
        else { // All other images are much smaller and can go into the normal PSRAM region
            //stbi_image_free(rgb_image);
            if (memsize == 0) {
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Not freeing (" + name + " as there was never PSRAM allocated for it)");
            }
            else {
                free_psram_heap(std::string(TAG) + "->CImageBasis (" + name + ", " + to_string(memsize) + ")", rgb_image);
            }
        }
    }

    RGBImageRelease();
}


void CImageBasis::SaveToFile(std::string _imageout)
{
    string typ = getFileType(_imageout);

    RGBImageLock();

    if ((typ == "jpg") || (typ == "JPG"))       // CAUTION PROBLEMATIC IN ESP32
    {
        stbi_write_jpg(_imageout.c_str(), width, height, channels, rgb_image, 0);
    }
 
#ifndef STBI_ONLY_JPEG
    if ((typ == "bmp") || (typ == "BMP"))
    {
        stbi_write_bmp(_imageout.c_str(), width, height, channels, rgb_image);
    }
#endif
    RGBImageRelease();
}


void CImageBasis::Resize(int _new_dx, int _new_dy)
{
    memsize = _new_dx * _new_dy * channels;
    uint8_t* odata = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->odata", memsize, MALLOC_CAP_SPIRAM);

    RGBImageLock();

    stbir_resize_uint8(rgb_image, width, height, 0, odata, _new_dx, _new_dy, 0, channels);

    rgb_image = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->CImageBasis Resize (" + name + ")", memsize, MALLOC_CAP_SPIRAM);
    memCopy(odata, rgb_image, memsize);
    width = _new_dx;
    height = _new_dy;

    free_psram_heap(std::string(TAG) + "->odata", odata);

    RGBImageRelease();
}


void CImageBasis::Resize(int _new_dx, int _new_dy, CImageBasis *_target)
{
    if ((_target->height != _new_dy) || (_target->width != _new_dx) || (_target->channels != channels))
    {
        ESP_LOGE(TAG, "Resize - Target image size does not fit!");
        return;
    }

    RGBImageLock();

    uint8_t* odata = _target->rgb_image;
    stbir_resize_uint8(rgb_image, width, height, 0, odata, _new_dx, _new_dy, 0, channels);

    RGBImageRelease();
}

