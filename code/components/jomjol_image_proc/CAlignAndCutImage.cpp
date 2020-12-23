#include "CAlignAndCutImage.h"
#include "CRotateImage.h"
#include "CFindTemplate.h"

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

    ImageTMP = _temp;
}

void CAlignAndCutImage::GetRefSize(int *ref_dx, int *ref_dy)
{
    ref_dx[0] = t0_dx;
    ref_dy[0] = t0_dy;
    ref_dx[1] = t1_dx;
    ref_dy[1] = t1_dy;
}

void CAlignAndCutImage::Align(std::string _template0, int ref0_x, int ref0_y, std::string _template1, int ref1_x, int ref1_y, int deltax, int deltay, std::string imageROI)
{
    int dx, dy;
    int r0_x, r0_y, r1_x, r1_y;

//    CFindTemplate* ft = new CFindTemplate(filename);
    CFindTemplate* ft = new CFindTemplate(rgb_image, channels, width, height, bpp);

    r0_x = ref0_x;
    r0_y = ref0_y;
    ft->FindTemplate(_template0, &r0_x, &r0_y, deltax, deltay);
    t0_dx = ft->tpl_width;
    t0_dy = ft->tpl_height;    

    r1_x = ref1_x;
    r1_y = ref1_y;
    ft->FindTemplate(_template1, &r1_x, &r1_y, deltax, deltay);
    t1_dx = ft->tpl_width;
    t1_dy = ft->tpl_height;

    delete ft;


    dx = ref0_x - r0_x;
    dy = ref0_y - r0_y;

    r0_x += dx;
    r0_y += dy;

    r1_x += dx;
    r1_y += dy;

    float w_org, w_ist, d_winkel;

    w_org = atan2(ref1_y - ref0_y, ref1_x - ref0_x);
    w_ist = atan2(r1_y - r0_y, r1_x - r0_x);

    d_winkel = (w_org - w_ist) * 180 / M_PI;

    if (imageROI.length() > 0)
    {
        CImageBasis* imgzw = new CImageBasis(this);
        imgzw->drawRect(r0_x, r0_y, t0_dx, t0_dy, 255, 0, 0, 2);
        imgzw->drawRect(r1_x, r1_y, t1_dx, t1_dy, 255, 0, 0, 2);
        imgzw->SaveToFile(imageROI);
        printf("Alignment: alignment ROI created: %s\n", imageROI.c_str());
        delete imgzw;
    }

    std::string zw = "\tdx:\t" + std::to_string(dx) + "\tdy:\t" + std::to_string(dy) + "\td_winkel:\t" + std::to_string(d_winkel);
//    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", zw);

    CRotateImage rt(this, ImageTMP);
    rt.Translate(dx, dy);
    rt.Rotate(d_winkel, ref0_x, ref0_y);
    printf("Alignment: dx %d - dy %d - rot %f\n", dx, dy, d_winkel);
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

    uint8_t* odata = _target->rgb_image;

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

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            p_target = odata + (channels * ((y - y1) * dx + (x - x1)));
            p_source = rgb_image + (channels * (y * width + x));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

    CImageBasis* rs = new CImageBasis(odata, channels, dx, dy, bpp);
    rs->SetIndepended();
    return rs;
}
