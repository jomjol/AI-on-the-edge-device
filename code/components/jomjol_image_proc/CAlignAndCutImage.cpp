#include "CAlignAndCutImage.h"
#include "CRotateImage.h"
#include "ClassLogFile.h"

#include <math.h>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <esp_log.h>
#include "psram.h"

#include "defines.h"
#include "Helper.h"

static const char *TAG = "c_align_and_cut_image";

CAlignAndCutImage::CAlignAndCutImage(std::string _name, CImageBasis *_org, CImageBasis *_temp) : CImageBasis(_name)
{
    name = _name;
    width = height = channels = bpp = 0;
    rgb_image = nullptr;
    bufferOwner = BufferOwner::None;
    ImageTMP = _temp;

    if (_org == nullptr || !_org->ImageOkay())
    {
        ESP_LOGE(TAG, "CAlignAndCutImage: source image is invalid");
        return;
    }

    UseImageMutexFrom(_org);
    rgb_image = _org->rgb_image;
    channels = _org->channels;
    width = _org->width;
    height = _org->height;
    bpp = _org->bpp;
    bufferOwner = BufferOwner::External;
}

void CAlignAndCutImage::GetRefSize(int *ref_dx, int *ref_dy)
{
    ref_dx[0] = t0_dx;
    ref_dy[0] = t0_dy;
    ref_dx[1] = t1_dx;
    ref_dy[1] = t1_dy;
}

bool CAlignAndCutImage::Align(RefInfo *_temp1, RefInfo *_temp2, bool _antiAlias)
{
    if (_temp1 != nullptr)
    {
        _temp1->found_x = _temp1->found_y = -1;
    }

    if (_temp2 != nullptr)
    {
        _temp2->found_x = _temp2->found_y = -1;
    }

    if (rgb_image == nullptr || _temp1 == nullptr || _temp2 == nullptr || ImageTMP == nullptr || !ImageTMP->ImageOkay())
    {
        ESP_LOGE(TAG, "Align: source, template or temporary image is invalid");
        return false;
    }

    CFindTemplate *ft = new (std::nothrow) CFindTemplate("align", rgb_image, channels, width, height, bpp, this);

    if (ft == nullptr)
    {
        ESP_LOGE(TAG, "Align: cannot allocate template finder");
        return false;
    }

    int r0_x = _temp1->target_x;
    int r0_y = _temp1->target_y;

    ESP_LOGD(TAG, "Before ft->FindTemplate(_temp1); %s", _temp1->image_file.c_str());
    bool isSimilar1 = ft->FindTemplate(_temp1);

    if (_temp1->found_x < 0 || _temp1->found_y < 0)
    {
        ESP_LOGE(TAG, "Align: first reference template was not found");
        delete ft;
        return false;
    }

    _temp1->width = ft->tpl_width;
    _temp1->height = ft->tpl_height;

    int r1_x = _temp2->target_x;
    int r1_y = _temp2->target_y;

    ESP_LOGD(TAG, "Before ft->FindTemplate(_temp2); %s", _temp2->image_file.c_str());
    bool isSimilar2 = ft->FindTemplate(_temp2);

    if (_temp2->found_x < 0 || _temp2->found_y < 0)
    {
        ESP_LOGE(TAG, "Align: second reference template was not found");
        delete ft;
        return false;
    }

    _temp2->width = ft->tpl_width;
    _temp2->height = ft->tpl_height;

    delete ft;

    int dx = _temp1->target_x - _temp1->found_x;
    int dy = _temp1->target_y - _temp1->found_y;

    r0_x += dx;
    r0_y += dy;

    r1_x += dx;
    r1_y += dy;

    float w_org, w_ist, d_winkel;

    w_org = atan2((float)(_temp2->found_y - _temp1->found_y), (float)(_temp2->found_x - _temp1->found_x));
    w_ist = atan2((float)(r1_y - r0_y), (float)(r1_x - r0_x));

    d_winkel = (w_ist - w_org) * 180 / M_PI;

    CRotateImage rt("Align", this, ImageTMP);
    rt.ImageTransform(d_winkel, _temp1->target_x, _temp1->target_y, dx, dy, _antiAlias);
    ESP_LOGD(TAG, "Alignment: dx %d - dy %d - rot %f", dx, dy, d_winkel);

    return (isSimilar1 && isSimilar2);
}

bool CAlignAndCutImage::CutAndSave(std::string _template1, int x1, int y1, int dx, int dy)
{
    if (rgb_image == nullptr || x1 < 0 || y1 < 0 || dx <= 0 || dy <= 0 || x1 >= width || y1 >= height)
    {
        ESP_LOGE(TAG, "CutAndSave: invalid source image or crop rectangle");
        return false;
    }

    const int x2 = static_cast<int>(std::min<int64_t>(static_cast<int64_t>(x1) + dx, width));
    const int y2 = static_cast<int>(std::min<int64_t>(static_cast<int64_t>(y1) + dy, height));

    dx = x2 - x1;
    dy = y2 - y1;

    size_t memsize = 0;

    if (!CalculateImageSize(dx, dy, channels, &memsize))
    {
        ESP_LOGE(TAG, "CutAndSave: invalid crop dimensions");
        return false;
    }

    uint8_t *odata = static_cast<uint8_t *>(malloc_psram_heap("CAlignAndCutImage" + name, memsize, MALLOC_CAP_SPIRAM));

    if (odata == nullptr)
    {
        ESP_LOGE(TAG, "CutAndSave: cannot allocate %u bytes", static_cast<unsigned>(memsize));
        return false;
    }

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        free_psram_heap("CAlignAndCutImage" + name, odata);
        return false;
    }

    for (int y = y1; y < y2; ++y)
    {
        const uint8_t *source = rgb_image + (static_cast<size_t>(y) * width + x1) * channels;
        uint8_t *target = odata + static_cast<size_t>(y - y1) * dx * channels;
        std::memcpy(target, source, static_cast<size_t>(dx) * channels);
    }

#ifdef STBI_ONLY_JPEG
    const int writeResult = stbi_write_jpg(_template1.c_str(), dx, dy, channels, odata, 100);
#else
    const int writeResult = stbi_write_bmp(_template1.c_str(), dx, dy, channels, odata);
#endif

    RGBImageRelease();

    free_psram_heap("CAlignAndCutImage" + name, odata);

    if (writeResult == 0)
    {
        ESP_LOGE(TAG, "CutAndSave: failed to write output file '%s'", _template1.c_str());
    }

    return writeResult != 0;
}

bool CAlignAndCutImage::CutAndSave(int x1, int y1, int dx, int dy, CImageBasis *_target)
{
    if (_target == nullptr || rgb_image == nullptr || x1 < 0 || y1 < 0 || dx <= 0 || dy <= 0 || x1 >= width || y1 >= height)
    {
        ESP_LOGE(TAG, "CutAndSave: invalid target, source image or crop rectangle");
        return false;
    }

    const int x2 = static_cast<int>(std::min<int64_t>(static_cast<int64_t>(x1) + dx, width));
    const int y2 = static_cast<int>(std::min<int64_t>(static_cast<int64_t>(y1) + dy, height));

    dx = x2 - x1;
    dy = y2 - y1;

    if ((_target->height != dy) || (_target->width != dx) || (_target->channels != channels))
    {
        ESP_LOGD(TAG, "CAlignAndCutImage::CutAndSave - Image size does not match!");
        return false;
    }

    uint8_t *targetData = _target->RGBImageLock();

    if (targetData == nullptr)
    {
        _target->RGBImageRelease();
        return false;
    }

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        _target->RGBImageRelease();
        return false;
    }

    for (int y = y1; y < y2; ++y)
    {
        const uint8_t *source = rgb_image + (static_cast<size_t>(y) * width + x1) * channels;
        uint8_t *target = targetData + static_cast<size_t>(y - y1) * dx * channels;
        std::memcpy(target, source, static_cast<size_t>(dx) * channels);
    }

    RGBImageRelease();
    _target->RGBImageRelease();

    return true;
}

CImageBasis *CAlignAndCutImage::CutAndSave(int x1, int y1, int dx, int dy)
{
    if (rgb_image == nullptr || x1 < 0 || y1 < 0 || dx <= 0 || dy <= 0 || x1 >= width || y1 >= height)
    {
        ESP_LOGE(TAG, "CutAndSave: invalid source image or crop rectangle");
        return nullptr;
    }

    const int x2 = static_cast<int>(std::min<int64_t>(static_cast<int64_t>(x1) + dx, width));
    const int y2 = static_cast<int>(std::min<int64_t>(static_cast<int64_t>(y1) + dy, height));

    dx = x2 - x1;
    dy = y2 - y1;

    size_t memsize = 0;

    if (!CalculateImageSize(dx, dy, channels, &memsize))
    {
        ESP_LOGE(TAG, "CutAndSave: invalid crop dimensions");
        return nullptr;
    }

    uint8_t *odata = static_cast<uint8_t *>(malloc_psram_heap("CAlignAndCutImage" + name, memsize, MALLOC_CAP_SPIRAM));

    if (odata == nullptr)
    {
        ESP_LOGE(TAG, "CutAndSave: cannot allocate %u bytes", static_cast<unsigned>(memsize));
        return nullptr;
    }

    if (RGBImageLock() == nullptr)
    {
        RGBImageRelease();
        free_psram_heap("CAlignAndCutImage" + name, odata);
        return nullptr;
    }

    for (int y = y1; y < y2; ++y)
    {
        const uint8_t *source = rgb_image + (static_cast<size_t>(y) * width + x1) * channels;
        uint8_t *target = odata + static_cast<size_t>(y - y1) * dx * channels;
        std::memcpy(target, source, static_cast<size_t>(dx) * channels);
    }

    RGBImageRelease();

    CImageBasis *result = new (std::nothrow) CImageBasis("CutAndSave", odata, channels, dx, dy, bpp);

    if (result == nullptr || !result->ImageOkay())
    {
        delete result;
        free_psram_heap("CAlignAndCutImage" + name, odata);
        return nullptr;
    }

    result->SetIndepended();

    return result;
}
