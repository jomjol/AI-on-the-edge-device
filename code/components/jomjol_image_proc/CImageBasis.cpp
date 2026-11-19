#include "CImageBasis.h"
#include "Helper.h"
#include "psram.h"
#include "ClassLogFile.h"
#include "server_ota.h"

#include <esp_log.h>
#include "defines.h"

#include "esp_system.h"

#include <cstring>

#include <math.h>
#include <algorithm>

static const char *TAG = "C IMG BASIS";

struct SendJPGHTTP
{
    httpd_req_t *req;
    esp_err_t res;
    char buf[HTTP_BUFFER_SENT];
    int size = 0;
};

bool CImageBasis::LockImageMutex(int _waitmaxsec)
{
    if (imageMutex == nullptr)
    {
        ESP_LOGE(TAG, "Image mutex unavailable for '%s'", name.c_str());
        return false;
    }

    const TickType_t waitTicks = (_waitmaxsec < 0) ? portMAX_DELAY : pdMS_TO_TICKS(static_cast<uint32_t>(_waitmaxsec) * 1000U);

    if (xSemaphoreTakeRecursive(imageMutex, waitTicks) != pdTRUE)
    {
        ESP_LOGW(TAG, "Timed out waiting for image mutex '%s'", name.c_str());
        return false;
    }

    return true;
}

uint8_t *CImageBasis::RGBImageLock(int _waitmaxsec)
{
    if (!LockImageMutex(_waitmaxsec))
    {
        return nullptr;
    }

    return rgb_image;
}

void CImageBasis::UseImageMutexFrom(CImageBasis *owner)
{
    if (owner == nullptr || owner == this || owner->imageMutex == nullptr)
    {
        return;
    }

    if (ownsImageMutex && imageMutex != nullptr)
    {
        vSemaphoreDelete(imageMutex);
    }

    imageMutex = owner->imageMutex;
    ownsImageMutex = false;
}

void CImageBasis::RGBImageRelease(void)
{
    // Several legacy callers release after RGBImageLock() returned nullptr.
    // Only the task that actually owns this mutex may release it; this avoids
    // corrupting the FreeRTOS mutex state after a timeout or failed lock.
    if (imageMutex != nullptr && xSemaphoreGetMutexHolder(imageMutex) == xTaskGetCurrentTaskHandle())
    {
        (void)xSemaphoreGiveRecursive(imageMutex);
    }
}

bool CImageBasis::CalculateImageSize(int imageWidth, int imageHeight, int imageChannels, size_t *result) const
{
    if (result == nullptr || imageWidth <= 0 || imageHeight <= 0 || imageChannels <= 0)
    {
        return false;
    }

    const size_t w = static_cast<size_t>(imageWidth);
    const size_t h = static_cast<size_t>(imageHeight);
    const size_t c = static_cast<size_t>(imageChannels);

    if (w > SIZE_MAX / h || w * h > SIZE_MAX / c)
    {
        return false;
    }

    *result = w * h * c;

    return true;
}

void CImageBasis::ReleaseImageBuffer()
{
    if (rgb_image == nullptr)
    {
        bufferOwner = BufferOwner::None;
        memsize = 0;
        return;
    }

    switch (bufferOwner)
    {
    case BufferOwner::PsramHeap:
        free_psram_heap("CImageBasis->" + name + "(" + std::to_string(memsize) + ")", rgb_image);
        break;
    case BufferOwner::StbImage:
        stbi_image_free(rgb_image);
        break;
    case BufferOwner::SharedRegion:
        psram_free_shared_temp_image_memory();
        break;
    case BufferOwner::External:
    case BufferOwner::None:
    default:
        break;
    }

    rgb_image = nullptr;
    memsize = 0;
    bufferOwner = BufferOwner::None;
}

void CImageBasis::SetIndepended()
{
    if (!LockImageMutex())
    {
        return;
    }

    if (rgb_image == nullptr)
    {
        bufferOwner = BufferOwner::None;
        memsize = 0;
        RGBImageRelease();
        return;
    }

    size_t size = 0;

    if (!CalculateImageSize(width, height, channels, &size))
    {
        ESP_LOGE(TAG, "SetIndepended: invalid image dimensions for '%s'", name.c_str());
        RGBImageRelease();
        return;
    }

    bufferOwner = BufferOwner::PsramHeap;
    memsize = size;
    RGBImageRelease();
}

uint8_t *CImageBasis::RGBImageGet(void)
{
    return rgb_image;
}

void writejpghelp(void *context, void *data, int size)
{
    ImageData *image = static_cast<ImageData *>(context);

    if (image == nullptr || data == nullptr || size <= 0 || image->tooLarge)
    {
        return;
    }

    if (image->size > MAX_JPG_SIZE || static_cast<size_t>(size) > MAX_JPG_SIZE - image->size)
    {
        image->tooLarge = true;
        return;
    }

    std::memcpy(image->data + image->size, data, size);
    image->size += size;
}

ImageData *CImageBasis::writeToMemoryAsJPG(const int quality)
{
    ImageData *_ImageData = new (std::nothrow) ImageData;

    if (_ImageData == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "writeToMemoryAsJPG: cannot allocate ImageData");
        return nullptr;
    }

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        delete _ImageData;
        return nullptr;
    }

    stbi_write_jpg_to_func(writejpghelp, _ImageData, width, height, channels, rgb_image, quality);

    RGBImageRelease();

    if (_ImageData->tooLarge)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "writeToMemoryAsJPG: Creation aborted! JPG size > preallocated buffer: " + std::to_string(MAX_JPG_SIZE));
        delete _ImageData;
        return nullptr;
    }

    return _ImageData;
}

void CImageBasis::writeToMemoryAsJPG(ImageData *_Image_Data, const int quality)
{
    if (_Image_Data == nullptr)
    {
        return;
    }

    _Image_Data->size = 0;
    _Image_Data->tooLarge = false;

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    stbi_write_jpg_to_func(writejpghelp, _Image_Data, width, height, channels, rgb_image, quality);

    RGBImageRelease();

    if (_Image_Data->tooLarge)
    {
        _Image_Data->size = 0;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "writeToMemoryAsJPG: Creation aborted! JPG size > preallocated buffer: " + std::to_string(MAX_JPG_SIZE));
    }
}

inline void writejpgtohttphelp(void *context, void *data, int size)
{
    SendJPGHTTP *_send = (SendJPGHTTP *)context;

    if (_send == nullptr || data == nullptr || size <= 0 || _send->res != ESP_OK)
    {
        return;
    }

    const uint8_t *source = static_cast<const uint8_t *>(data);
    size_t remaining = static_cast<size_t>(size);

    while (remaining > 0)
    {
        const size_t available = HTTP_BUFFER_SENT - static_cast<size_t>(_send->size);
        const size_t copySize = std::min(available, remaining);
        std::memcpy(_send->buf + _send->size, source, copySize);
        _send->size += static_cast<int>(copySize);
        source += copySize;
        remaining -= copySize;

        if (_send->size == HTTP_BUFFER_SENT)
        {
            if (httpd_resp_send_chunk(_send->req, _send->buf, _send->size) != ESP_OK)
            {
                ESP_LOGE(TAG, "File sending failed!");
                _send->res = ESP_FAIL;
                return;
            }

            _send->size = 0;
        }
    }
}

esp_err_t CImageBasis::SendJPGtoHTTP(httpd_req_t *_req, const int quality)
{
    if (_req == nullptr || rgb_image == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }

    SendJPGHTTP _send;
    _send.req = _req;
    _send.res = ESP_OK;
    _send.size = 0;

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return ESP_FAIL;
    }

    stbi_write_jpg_to_func(writejpgtohttphelp, &_send, width, height, channels, rgb_image, quality);

    if (_send.res == ESP_OK && _send.size > 0)
    {
        if (httpd_resp_send_chunk(_req, (char *)_send.buf, _send.size) != ESP_OK)
        {
            ESP_LOGE(TAG, "File sending failed!");
            _send.res = ESP_FAIL;
        }
    }

    RGBImageRelease();

    return _send.res;
}

bool CImageBasis::CopyFromMemory(uint8_t *_source, int _size)
{
    size_t required = 0;

    if (_source == nullptr || rgb_image == nullptr || _size < 0 || !CalculateImageSize(width, height, channels, &required) || required != static_cast<size_t>(_size))
    {
        ESP_LOGE(TAG, "Cannot copy image from memory - invalid buffer or size mismatch (provided %d)", _size);
        return false;
    }

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return false;
    }

    std::memcpy(rgb_image, _source, _size);
    RGBImageRelease();

    return true;
}

uint8_t CImageBasis::GetPixelColor(int x, int y, int ch)
{
    if (x < 0 || y < 0 || x >= width || y >= height || ch < 0 || ch >= channels)
    {
        return 0;
    }

    uint8_t *image = RGBImageLock();

    if (image == nullptr)
    {
        RGBImageRelease();
        return 0;
    }

    const uint8_t value = image[(static_cast<size_t>(y) * width + x) * channels + ch];

    RGBImageRelease();

    return value;
}

void CImageBasis::memCopy(uint8_t *_source, uint8_t *_target, int _size)
{
#ifdef _ESP32_PSRAM
    for (int i = 0; i < _size; ++i)
    {
        *(_target + i) = *(_source + i);
    }
#else
    std::memcpy(_target, _source, _size);
#endif
}

bool CImageBasis::isInImage(int x, int y)
{
    if ((x < 0) || (x > width - 1))
    {
        return false;
    }

    if ((y < 0) || (y > height - 1))
    {
        return false;
    }

    return true;
}

void CImageBasis::setPixelColor(int x, int y, int r, int g, int b)
{
    if (!isInImage(x, y))
    {
        return;
    }

    uint8_t *image = RGBImageLock();

    if (image == nullptr)
    {
        RGBImageRelease();
        return;
    }

    stbi_uc *p_source = image + (channels * (y * width + x));
    p_source[0] = r;

    if (channels > 2)
    {
        p_source[1] = g;
        p_source[2] = b;
    }

    RGBImageRelease();
}

void CImageBasis::drawRect(int x, int y, int dx, int dy, int r, int g, int b, int thickness)
{
    if (rgb_image == nullptr || imageMutex == nullptr)
    {
        return;
    }

    int zwx1, zwx2, zwy1, zwy2;
    int _x, _y, _thick;

    zwx1 = x - thickness + 1;
    zwx2 = x + dx + thickness - 1;
    zwy1 = y;
    zwy2 = y;

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    for (_thick = 0; _thick < thickness; _thick++)
    {
        for (_x = zwx1; _x <= zwx2; ++_x)
        {
            for (_y = zwy1; _y <= zwy2; _y++)
            {
                if (isInImage(_x, _y))
                {
                    setPixelColor(_x, _y - _thick, r, g, b);
                }
            }
        }
    }

    zwx1 = x - thickness + 1;
    zwx2 = x + dx + thickness - 1;
    zwy1 = y + dy;
    zwy2 = y + dy;

    for (_thick = 0; _thick < thickness; _thick++)
    {
        for (_x = zwx1; _x <= zwx2; ++_x)
        {
            for (_y = zwy1; _y <= zwy2; _y++)
            {
                if (isInImage(_x, _y))
                {
                    setPixelColor(_x, _y + _thick, r, g, b);
                }
            }
        }
    }

    zwx1 = x;
    zwx2 = x;
    zwy1 = y;
    zwy2 = y + dy;

    for (_thick = 0; _thick < thickness; _thick++)
    {
        for (_x = zwx1; _x <= zwx2; ++_x)
        {
            for (_y = zwy1; _y <= zwy2; _y++)
            {
                if (isInImage(_x, _y))
                {
                    setPixelColor(_x - _thick, _y, r, g, b);
                }
            }
        }
    }

    zwx1 = x + dx;
    zwx2 = x + dx;
    zwy1 = y;
    zwy2 = y + dy;

    for (_thick = 0; _thick < thickness; _thick++)
    {
        for (_x = zwx1; _x <= zwx2; ++_x)
        {
            for (_y = zwy1; _y <= zwy2; _y++)
            {
                if (isInImage(_x, _y))
                {
                    setPixelColor(_x + _thick, _y, r, g, b);
                }
            }
        }
    }

    RGBImageRelease();
}

void CImageBasis::drawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int thickness)
{
    if (rgb_image == nullptr || imageMutex == nullptr)
    {
        return;
    }

    int _x, _y, _thick;
    int _zwy1, _zwy2;
    thickness = (thickness - 1) / 2;

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    for (_thick = 0; _thick <= thickness; ++_thick)
    {
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
            {
                if (isInImage(_x, _y))
                {
                    setPixelColor(_x, _y, r, g, b);
                }
            }
        }
    }

    RGBImageRelease();
}

void CImageBasis::drawEllipse(int x1, int y1, int radx, int rady, int r, int g, int b, int thickness)
{
    if (rgb_image == nullptr || imageMutex == nullptr)
    {
        return;
    }

    float deltarad, aktrad;
    int _thick, _x, _y;
    int rad = radx;

    if (rady > radx)
    {
        rad = rady;
    }

    deltarad = 1 / (4 * M_PI * (rad + thickness - 1));

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    for (aktrad = 0; aktrad <= (2 * M_PI); aktrad += deltarad)
    {
        for (_thick = 0; _thick < thickness; ++_thick)
        {
            _x = sin(aktrad) * (radx + _thick) + x1;
            _y = cos(aktrad) * (rady + _thick) + y1;

            if (isInImage(_x, _y))
            {
                setPixelColor(_x, _y, r, g, b);
            }
        }
    }

    RGBImageRelease();
}

void CImageBasis::drawCircle(int x1, int y1, int rad, int r, int g, int b, int thickness)
{
    if (rgb_image == nullptr || imageMutex == nullptr)
    {
        return;
    }

    float deltarad, aktrad;
    int _thick, _x, _y;

    deltarad = 1 / (4 * M_PI * (rad + thickness - 1));

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    for (aktrad = 0; aktrad <= (2 * M_PI); aktrad += deltarad)
    {
        for (_thick = 0; _thick < thickness; ++_thick)
        {
            _x = sin(aktrad) * (rad + _thick) + x1;
            _y = cos(aktrad) * (rad + _thick) + y1;

            if (isInImage(_x, _y))
            {
                setPixelColor(_x, _y, r, g, b);
            }
        }
    }

    RGBImageRelease();
}

CImageBasis::CImageBasis(string _name)
{
    name = _name;
    rgb_image = NULL;
    width = 0;
    height = 0;
    channels = 0;
    bpp = 0;

    imageMutex = xSemaphoreCreateRecursiveMutex();

    if (imageMutex == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create image mutex for '%s'", name.c_str());
    }
}

void CImageBasis::CreateEmptyImage(int _width, int _height, int _channels)
{
    if (imageMutex == nullptr)
    {
        return;
    }

    if (!LockImageMutex())
    {
        return;
    }

    ReleaseImageBuffer();

    size_t newMemSize = 0;

    if (!CalculateImageSize(_width, _height, _channels, &newMemSize))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CreateEmptyImage: invalid dimensions");
        width = height = channels = bpp = 0;
        RGBImageRelease();
        return;
    }

    bpp = _channels;
    width = _width;
    height = _height;
    channels = _channels;

    rgb_image = (unsigned char *)malloc_psram_heap("CImageBasis->" + name, newMemSize, MALLOC_CAP_SPIRAM);

    if (rgb_image == NULL)
    {
        ESP_LOGE(TAG, "CreateEmptyImage: Can't allocate enough memory: %d", (int)newMemSize);
        LogFile.WriteHeapInfo("CreateEmptyImage");
        width = height = channels = bpp = 0;
        RGBImageRelease();
        return;
    }

    bufferOwner = BufferOwner::PsramHeap;
    memsize = newMemSize;
    std::memset(rgb_image, 0, newMemSize);

    RGBImageRelease();
}

void CImageBasis::EmptyImage(void)
{
    if (rgb_image == nullptr)
    {
        return;
    }

    size_t size = 0;

    if (!CalculateImageSize(width, height, channels, &size) || RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    // One contiguous memset instead of a per-pixel, column-by-column loop through PSRAM
    std::memset(rgb_image, 0, size);

    RGBImageRelease();
}

void CImageBasis::LoadFromMemory(stbi_uc *_buffer, int len)
{
    if (imageMutex == nullptr)
    {
        return;
    }

    if (!LockImageMutex())
    {
        return;
    }

    ReleaseImageBuffer();

    width = height = channels = bpp = 0;

    if (_buffer == nullptr || len <= 0)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadFromMemory: empty input buffer");
        RGBImageRelease();
        return;
    }

    int sourceChannels = 0;
    rgb_image = stbi_load_from_memory(_buffer, len, &width, &height, &sourceChannels, STBI_rgb);

    if (rgb_image == nullptr)
    {
        width = height = channels = bpp = 0;
        memsize = 0;
        bufferOwner = BufferOwner::None;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadFromMemory: STBI failed to decode image");
        LogFile.WriteHeapInfo("LoadFromMemory");
        RGBImageRelease();
        return;
    }

    // req_comp=STBI_rgb means the returned pixel buffer is always RGB888.
    // sourceChannels is only the original file's component count and must not
    // be used for addressing or calculating the allocated output buffer size.
    channels = STBI_rgb;
    bpp = sourceChannels;
    bufferOwner = BufferOwner::StbImage;

    if (!CalculateImageSize(width, height, channels, &memsize))
    {
        ReleaseImageBuffer();
        width = height = channels = bpp = 0;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadFromMemory: decoder returned invalid image dimensions");
        RGBImageRelease();
        return;
    }

    ESP_LOGD(TAG, "Image loaded from memory: %d, %d, %d", width, height, channels);

    RGBImageRelease();
}

void CImageBasis::crop_image(unsigned short cropLeft, unsigned short cropRight, unsigned short cropTop, unsigned short cropBottom)
{
    if (rgb_image == nullptr || static_cast<size_t>(cropLeft) + cropRight >= static_cast<size_t>(width) || static_cast<size_t>(cropTop) + cropBottom >= static_cast<size_t>(height))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "crop_image: invalid crop rectangle");
        return;
    }

    const int newWidth = width - cropLeft - cropRight;
    const int newHeight = height - cropTop - cropBottom;

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    uint8_t *pSrc = rgb_image + (cropTop * width + cropLeft) * channels;

    for (int y = 0; y < newHeight; ++y)
    {
        std::memmove(rgb_image + y * newWidth * channels, pSrc + y * width * channels, newWidth * channels);
    }

    width = newWidth;
    height = newHeight;
    (void)CalculateImageSize(width, height, channels, &memsize);

    RGBImageRelease();
}

CImageBasis::CImageBasis(string _name, CImageBasis *_copyfrom, bool useSharedRegion)
{
    name = _name;
    width = height = channels = bpp = 0;
    imageMutex = xSemaphoreCreateRecursiveMutex();

    if (_copyfrom == nullptr || !_copyfrom->ImageOkay() || imageMutex == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-Copyfrom: source image is invalid");
        return;
    }

    channels = _copyfrom->channels;
    width = _copyfrom->width;
    height = _copyfrom->height;
    bpp = _copyfrom->bpp;

    if (!LockImageMutex())
    {
        return;
    }

    if (!CalculateImageSize(width, height, channels, &memsize))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-Copyfrom: invalid source dimensions");
        RGBImageRelease();
        return;
    }

    if (useSharedRegion)
    {
        rgb_image = (unsigned char *)psram_reserve_shared_tmp_image_memory(memsize);

        if (rgb_image != nullptr)
        {
            bufferOwner = BufferOwner::SharedRegion;
        }
    }
    else
    {
        rgb_image = (unsigned char *)malloc_psram_heap("CImageBasis->" + name, memsize, MALLOC_CAP_SPIRAM);

        if (rgb_image != nullptr)
        {
            bufferOwner = BufferOwner::PsramHeap;
        }
    }

    if (rgb_image == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-Copyfrom: Can't allocate enough memory: " + std::to_string(memsize));
        LogFile.WriteHeapInfo("CImageBasis-Copyfrom");
        RGBImageRelease();
        return;
    }

    uint8_t *sourceImage = _copyfrom->RGBImageLock();

    if (sourceImage == nullptr)
    {
        _copyfrom->RGBImageRelease();
        ReleaseImageBuffer();
        RGBImageRelease();
        return;
    }

    std::memcpy(rgb_image, sourceImage, memsize);
    _copyfrom->RGBImageRelease();

    RGBImageRelease();
}

CImageBasis::CImageBasis(string _name, int _width, int _height, int _channels)
{
    name = _name;
    imageMutex = xSemaphoreCreateRecursiveMutex();

    if (imageMutex == nullptr)
    {
        return;
    }

    channels = _channels;
    width = _width;
    height = _height;
    bpp = _channels;

    if (!LockImageMutex())
    {
        return;
    }

    if (!CalculateImageSize(width, height, channels, &memsize))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-width,height,ch: invalid image dimensions");
        width = height = channels = bpp = 0;
        RGBImageRelease();
        return;
    }

    rgb_image = (unsigned char *)malloc_psram_heap("CImageBasis->" + name, memsize, MALLOC_CAP_SPIRAM);

    if (rgb_image == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-width,height,ch: Can't allocate enough memory: " + std::to_string(memsize));
        LogFile.WriteHeapInfo("CImageBasis-width,height,ch");
        RGBImageRelease();
        return;
    }

    bufferOwner = BufferOwner::PsramHeap;

    RGBImageRelease();
}

CImageBasis::CImageBasis(string _name, std::string _image)
{
    name = _name;
    channels = 3;
    width = height = bpp = 0;
    imageMutex = xSemaphoreCreateRecursiveMutex();

    if (imageMutex == nullptr)
    {
        return;
    }

    filename = _image;

    if (file_size(_image.c_str()) == 0)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, _image + " is empty!");
        return;
    }

    RGBImageLock();

    rgb_image = stbi_load(_image.c_str(), &width, &height, &bpp, channels);

    if (rgb_image == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-image: Failed to load " + _image + "! Is it corrupted?");
        LogFile.WriteHeapInfo("CImageBasis-image");
        RGBImageRelease();
        return;
    }

    bufferOwner = BufferOwner::StbImage;

    if (!CalculateImageSize(width, height, channels, &memsize))
    {
        ReleaseImageBuffer();
        width = height = channels = bpp = 0;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CImageBasis-image: decoder returned invalid dimensions");
        RGBImageRelease();
        return;
    }

    RGBImageRelease();
}

bool CImageBasis::ImageOkay(void)
{
    return rgb_image != NULL;
}

CImageBasis::CImageBasis(string _name, uint8_t *_rgb_image, int _channels, int _width, int _height, int _bpp)
{
    name = _name;
    imageMutex = xSemaphoreCreateRecursiveMutex();

    if (imageMutex == nullptr)
    {
        return;
    }

    rgb_image = _rgb_image;
    channels = _channels;
    width = _width;
    height = _height;
    bpp = _bpp;
    bufferOwner = BufferOwner::External;

    if (rgb_image == nullptr || !CalculateImageSize(width, height, channels, &memsize))
    {
        rgb_image = nullptr;
        memsize = 0;
        bufferOwner = BufferOwner::None;
        width = height = channels = bpp = 0;
    }
}

void CImageBasis::Negative(void)
{
    if (rgb_image == nullptr || imageMutex == nullptr)
    {
        return;
    }

    uint8_t *image = RGBImageLock();

    if (image == nullptr)
    {
        RGBImageRelease();
        return;
    }

    size_t imageSize = 0;

    if (!CalculateImageSize(width, height, channels, &imageSize))
    {
        RGBImageRelease();
        return;
    }

    for (size_t i = 0; i < imageSize; ++i)
    {
        image[i] = static_cast<uint8_t>(255 - image[i]);
    }

    RGBImageRelease();
}

// input range [-100..100]
void CImageBasis::Contrast(float _contrast)
{
    if (rgb_image == nullptr || imageMutex == nullptr)
    {
        return;
    }

    stbi_uc *p_source;

    float contrast = (_contrast / 100) + 1; // convert to decimal & shift range: [0..2]
    float intercept = 128 * (1 - contrast);

    uint8_t *image = RGBImageLock();

    if (image == nullptr)
    {
        RGBImageRelease();
        return;
    }

    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            p_source = image + (channels * (y * width + x));

            for (int _channels = 0; _channels < channels; ++_channels)
            {
                p_source[_channels] = (uint8_t)std::min(255, std::max(0, (int)(p_source[_channels] * contrast + intercept)));
            }
        }
    }

    RGBImageRelease();
}

CImageBasis::~CImageBasis()
{
    if (LockImageMutex(-1))
    {
        ReleaseImageBuffer();
        RGBImageRelease();
    }

    if (imageMutex != nullptr && ownsImageMutex)
    {
        vSemaphoreDelete(imageMutex);
    }

    imageMutex = nullptr;
}

void CImageBasis::SaveToFile(std::string _imageout)
{
    if (_imageout.empty() || rgb_image == nullptr)
    {
        return;
    }

    string typ = getFileType(_imageout);

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    if ((typ == "jpg") || (typ == "JPG"))
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
    if (rgb_image == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Resize: not possible for image " + name);
        return;
    }

    size_t new_memsize = 0;

    if (!CalculateImageSize(_new_dx, _new_dy, channels, &new_memsize))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Resize: invalid target dimensions for image " + name);
        return;
    }

    uint8_t *newImage = (unsigned char *)malloc_psram_heap("CImageBasis->" + name, new_memsize, MALLOC_CAP_SPIRAM);

    if (newImage == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Resize: Can't allocate enough memory: " + std::to_string(new_memsize));
        return;
    }

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        free_psram_heap("CImageBasis->" + name + " (failed resize)", newImage);
        return;
    }

    if (!stbir_resize_uint8(rgb_image, width, height, 0, newImage, _new_dx, _new_dy, 0, channels))
    {
        RGBImageRelease();
        free_psram_heap("CImageBasis->" + name + " (resize failed)", newImage);
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Resize: stb resize failed for image " + name);
        return;
    }

    ReleaseImageBuffer();

    rgb_image = newImage;
    memsize = new_memsize;
    bufferOwner = BufferOwner::PsramHeap;
    width = _new_dx;
    height = _new_dy;

    RGBImageRelease();
}

void CImageBasis::Resize(int _new_dx, int _new_dy, CImageBasis *_target)
{
    if (_target == nullptr || rgb_image == nullptr || !_target->ImageOkay())
    {
        ESP_LOGE(TAG, "Resize - source or target image is invalid");
        return;
    }

    if ((_target->height != _new_dy) || (_target->width != _new_dx) || (_target->channels != channels))
    {
        ESP_LOGE(TAG, "Resize - Target image size does not fit!");
        return;
    }

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        return;
    }

    uint8_t *targetData = _target->RGBImageLock();

    if (targetData == nullptr)
    {
        _target->RGBImageRelease();
        RGBImageRelease();
        return;
    }

    if (!stbir_resize_uint8(rgb_image, width, height, 0, targetData, _new_dx, _new_dy, 0, channels))
    {
        ESP_LOGE(TAG, "Resize - stb resize failed");
    }

    _target->RGBImageRelease();
    RGBImageRelease();
}
