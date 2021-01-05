#include "CAlignAndCutImage.h"
#include "CRotateImage.h"
#include "ClassLogFile.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

//#define GET_MEMORY malloc
#define GET_MEMORY(X) heap_caps_malloc(X, MALLOC_CAP_SPIRAM)



CAlignAndCutImage::CAlignAndCutImage(CImageBasis *_org, CImageBasis *_temp)
{
    rgb_image = _org->rgb_image;
    channels = _org->channels;
    width = _org->width;
    height = _org->height;
    bpp = _org->bpp;
    externalImage = true;   

    islocked = false; 

    ImageTMP = _temp;
}

void CAlignAndCutImage::GetRefSize(int *ref_dx, int *ref_dy)
{
    ref_dx[0] = t0_dx;
    ref_dy[0] = t0_dy;
    ref_dx[1] = t1_dx;
    ref_dy[1] = t1_dy;
}

bool CAlignAndCutImage::Align(RefInfo *_temp1, RefInfo *_temp2)
{
    int dx, dy;
    int r0_x, r0_y, r1_x, r1_y;
    bool isSimilar1, isSimilar2;

    CFindTemplate* ft = new CFindTemplate(rgb_image, channels, width, height, bpp);

    r0_x = _temp1->target_x;
    r0_y = _temp1->target_y;
    printf("Vor ft->FindTemplate(_temp1);  %s\n", _temp1->image_file.c_str());
    isSimilar1 = ft->FindTemplate(_temp1);
    _temp1->width = ft->tpl_width;
    _temp1->height = ft->tpl_height; 

    r1_x = _temp2->target_x;
    r1_y = _temp2->target_y;
    printf("Vor ft->FindTemplate(_temp2);  %s\n", _temp2->image_file.c_str());
    isSimilar2 = ft->FindTemplate(_temp2);
    _temp2->width = ft->tpl_width;
    _temp2->height = ft->tpl_height; 

    delete ft;


    dx = _temp1->target_x - _temp1->found_x;
    dy = _temp1->target_y - _temp1->found_y;

    r0_x += dx;
    r0_y += dy;

    r1_x += dx;
    r1_y += dy;

    float w_org, w_ist, d_winkel;

    w_org = atan2(_temp2->found_y - _temp1->found_y, _temp2->found_x - _temp1->found_x);
    w_ist = atan2(r1_y - r0_y, r1_x - r0_x);

    d_winkel = (w_ist - w_org) * 180 / M_PI;

#ifdef DEBUG_DETAIL_ON
    std::string zw = "\tdx:\t" + std::to_string(dx) + "\tdy:\t" + std::to_string(dy) + "\td_winkel:\t" + std::to_string(d_winkel);
    zw = zw + "\tt1_x_y:\t" + std::to_string(_temp1->found_x) + "\t" + std::to_string(_temp1->found_y);
    zw = zw + "\tpara1_found_min_avg_max_SAD:\t" + std::to_string(_temp1->fastalg_min) + "\t" + std::to_string(_temp1->fastalg_avg) + "\t" + std::to_string(_temp1->fastalg_max) + "\t"+ std::to_string(_temp1->fastalg_SAD);
    zw = zw + "\tt2_x_y:\t" + std::to_string(_temp2->found_x) + "\t" + std::to_string(_temp2->found_y);
    zw = zw + "\tpara2_found_min_avg_max:\t" + std::to_string(_temp2->fastalg_min) + "\t" + std::to_string(_temp2->fastalg_avg) + "\t" + std::to_string(_temp2->fastalg_max) + "\t"+ std::to_string(_temp2->fastalg_SAD);
    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", zw);
#endif

    CRotateImage rt(this, ImageTMP);
    rt.Translate(dx, dy);
    rt.Rotate(d_winkel, _temp1->target_x, _temp1->target_y);
    printf("Alignment: dx %d - dy %d - rot %f\n", dx, dy, d_winkel);

    return (isSimilar1 && isSimilar2);
}





void CAlignAndCutImage::CutAndSave(std::string _template1, int x1, int y1, int dx, int dy)
{

    int x2, y2;

    x2 = x1 + dx;
    y2 = y1 + dy;
    x2 = std::min(x2, width - 1);
    y2 = std::min(y2, height - 1);

    dx = x2 - x1;
    dy = y2 - y1;

    int memsize = dx * dy * channels;
    uint8_t* odata = (unsigned char*) GET_MEMORY(memsize);

    stbi_uc* p_target;
    stbi_uc* p_source;

    RGBImageLock();

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            p_target = odata + (channels * ((y - y1) * dx + (x - x1)));
            p_source = rgb_image + (channels * (y * width + x));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

    //    stbi_write_jpg(_template1.c_str(), dx, dy, channels, odata, 0);
    stbi_write_bmp(_template1.c_str(), dx, dy, channels, odata);

    RGBImageRelease();

    stbi_image_free(odata);
}

void CAlignAndCutImage::CutAndSave(int x1, int y1, int dx, int dy, CImageBasis *_target)
{
    int x2, y2;

    x2 = x1 + dx;
    y2 = y1 + dy;
    x2 = std::min(x2, width - 1);
    y2 = std::min(y2, height - 1);

    dx = x2 - x1;
    dy = y2 - y1;

    if ((_target->height != dy) || (_target->width != dx) || (_target->channels != channels))
    {
        printf("CAlignAndCutImage::CutAndSave - Bildgröße passt nicht !!!!!!!!!");
        return;
    }

    uint8_t* odata = _target->RGBImageLock();
    RGBImageLock();

    stbi_uc* p_target;
    stbi_uc* p_source;

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            p_target = odata + (channels * ((y - y1) * dx + (x - x1)));
            p_source = rgb_image + (channels * (y * width + x));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

    RGBImageRelease();
    _target->RGBImageRelease();
}


CImageBasis* CAlignAndCutImage::CutAndSave(int x1, int y1, int dx, int dy)
{
    int x2, y2;

    x2 = x1 + dx;
    y2 = y1 + dy;
    x2 = std::min(x2, width - 1);
    y2 = std::min(y2, height - 1);

    dx = x2 - x1;
    dy = y2 - y1;

    int memsize = dx * dy * channels;
    uint8_t* odata = (unsigned char*)GET_MEMORY(memsize);

    stbi_uc* p_target;
    stbi_uc* p_source;

    RGBImageLock();

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            p_target = odata + (channels * ((y - y1) * dx + (x - x1)));
            p_source = rgb_image + (channels * (y * width + x));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

    CImageBasis* rs = new CImageBasis(odata, channels, dx, dy, bpp);
    RGBImageRelease();
    rs->SetIndepended();
    return rs;
}
