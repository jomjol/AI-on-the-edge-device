#include "CAlignAndCutImage.h"
#include "CRotateImage.h"
#include "ClassLogFile.h"

#include <math.h>
#include <algorithm>
#include <esp_log.h>
#include "psram.h"
#include "../../include/defines.h"

static const char* TAG = "c_align_and_cut_image";

// How far a ROI may reach past the image edge and still be padded rather than moved. A row or two
// of replicated edge pixels is invisible to the model after downsampling and keeps the dial exactly
// where the ROI was drawn; a ROI overhanging by much more is misconfigured, and quietly synthesising a
// large part of it would just be the old silent failure in new clothes.
static const int ROI_PAD_MAX = 8;

// Pixels of the span [a, a+d) lying outside [0, n): those before 0 plus those at or beyond n.
static int Overhang(int a, int d, int n)
{
    return std::max(0, -a) + std::max(0, a + d - n);
}

// A ROI can reach outside the image in two ways: its origin can lie before the edge, or the origin
// can be legal while the far side runs past it. Either way the crop is ALWAYS produced at the
// configured size - pixels outside the image replicate the nearest edge pixel (see the copy loops) -
// so the framing the operator chose is preserved. The origin is moved only when the overhang exceeds
// ROI_PAD_MAX. Padding within the cap is intended behaviour and is noted at debug level; exceeding
// the cap - whether the ROI is then moved, or is too large to fit at all - is a real deviation from
// the configuration and is logged as a warning.
static void FitROIToImage(int &x1, int &y1, int dx, int dy, int width, int height)
{
    const int orig_x1 = x1;
    const int orig_y1 = y1;
    const bool over_cap = (Overhang(x1, dx, width) > ROI_PAD_MAX) || (Overhang(y1, dy, height) > ROI_PAD_MAX);

    if (over_cap)
    {
        // The copy loops read up to and including column width-1 / row height-1, so the last origin
        // that still fits the whole ROI is width-dx / height-dy.
        x1 = std::max(0, std::min(x1, width - dx));
        y1 = std::max(0, std::min(y1, height - dy));
    }

    const int pad = Overhang(x1, dx, width) + Overhang(y1, dy, height);
    const bool moved = (x1 != orig_x1) || (y1 != orig_y1);
    if (moved || (pad > 0))
    {
        std::string msg = "CutAndSave: ROI (" + std::to_string(orig_x1) + "," + std::to_string(orig_y1) + " " +
                          std::to_string(dx) + "x" + std::to_string(dy) + ") reaches outside the image (" +
                          std::to_string(width) + "x" + std::to_string(height) + ") - ";
        msg += moved ? ("moved to (" + std::to_string(x1) + "," + std::to_string(y1) + ")")
                     : (over_cap ? "too large to fit" : "kept in place");
        if (pad > 0)
            msg += ", " + std::to_string(pad) + " px outside the image replicate the edge";
        LogFile.WriteToFile(over_cap ? ESP_LOG_WARN : ESP_LOG_DEBUG, TAG, msg + ". Check the ROI configuration.");
    }
}

CAlignAndCutImage::CAlignAndCutImage(std::string _name, CImageBasis *_org, CImageBasis *_temp) : CImageBasis(_name)
{
    name = _name;
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

    CFindTemplate* ft = new CFindTemplate("align", rgb_image, channels, width, height, bpp);

    r0_x = _temp1->target_x;
    r0_y = _temp1->target_y;
    ESP_LOGD(TAG, "Before ft->FindTemplate(_temp1); %s", _temp1->image_file.c_str());
    isSimilar1 = ft->FindTemplate(_temp1);
    _temp1->width = ft->tpl_width;
    _temp1->height = ft->tpl_height; 

    r1_x = _temp2->target_x;
    r1_y = _temp2->target_y;
    ESP_LOGD(TAG, "Before ft->FindTemplate(_temp2); %s", _temp2->image_file.c_str());
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

/*#ifdef DEBUG_DETAIL_ON
    std::string zw = "\tdx:\t" + std::to_string(dx) + "\tdy:\t" + std::to_string(dy) + "\td_winkel:\t" + std::to_string(d_winkel);
    zw = zw + "\tt1_x_y:\t" + std::to_string(_temp1->found_x) + "\t" + std::to_string(_temp1->found_y);
    zw = zw + "\tpara1_found_min_avg_max_SAD:\t" + std::to_string(_temp1->fastalg_min) + "\t" + std::to_string(_temp1->fastalg_avg) + "\t" + std::to_string(_temp1->fastalg_max) + "\t"+ std::to_string(_temp1->fastalg_SAD);
    zw = zw + "\tt2_x_y:\t" + std::to_string(_temp2->found_x) + "\t" + std::to_string(_temp2->found_y);
    zw = zw + "\tpara2_found_min_avg_max:\t" + std::to_string(_temp2->fastalg_min) + "\t" + std::to_string(_temp2->fastalg_avg) + "\t" + std::to_string(_temp2->fastalg_max) + "\t"+ std::to_string(_temp2->fastalg_SAD);
    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", zw);
#endif*/

    CRotateImage rt("Align", this, ImageTMP);
    rt.Translate(dx, dy);
    rt.Rotate(d_winkel, _temp1->target_x, _temp1->target_y);
    ESP_LOGD(TAG, "Alignment: dx %d - dy %d - rot %f", dx, dy, d_winkel);

    return (isSimilar1 && isSimilar2);
}





void CAlignAndCutImage::CutAndSave(std::string _template1, int x1, int y1, int dx, int dy)
{

    int x2, y2;

    FitROIToImage(x1, y1, dx, dy, width, height);

    // Always the full configured size: FitROIToImage has already decided whether the origin moves,
    // and the copy loop pads whatever still lies outside the image.
    x2 = x1 + dx;
    y2 = y1 + dy;

    int memsize = dx * dy * channels;
    uint8_t* odata = (unsigned char*) malloc_psram_heap(std::string(TAG) + "->odata", memsize, MALLOC_CAP_SPIRAM);

    stbi_uc* p_target;
    stbi_uc* p_source;

    RGBImageLock();

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            // Pixels outside the image replicate the nearest edge pixel (see FitROIToImage).
            const int sx = std::min(std::max(x, 0), width - 1);
            const int sy = std::min(std::max(y, 0), height - 1);
            p_target = odata + (channels * ((y - y1) * dx + (x - x1)));
            p_source = rgb_image + (channels * (sy * width + sx));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

#ifdef STBI_ONLY_JPEG
    stbi_write_jpg(_template1.c_str(), dx, dy, channels, odata, 100);
#else
    stbi_write_bmp(_template1.c_str(), dx, dy, channels, odata);
#endif
    

    RGBImageRelease();

    stbi_image_free(odata);
}

void CAlignAndCutImage::CutAndSave(int x1, int y1, int dx, int dy, CImageBasis *_target)
{
    int x2, y2;

    FitROIToImage(x1, y1, dx, dy, width, height);

    // Always the full configured size: FitROIToImage has already decided whether the origin moves,
    // and the copy loop pads whatever still lies outside the image.
    x2 = x1 + dx;
    y2 = y1 + dy;

    if ((_target->height != dy) || (_target->width != dx) || (_target->channels != channels))
    {
        // dx/dy are never altered above, so this can only trip if the caller sized the target for a
        // different ROI. Skipping leaves the target holding stale content that reaches the CNN as if
        // it were a fresh reading, so it must not stay silent.
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "CutAndSave: ROI cannot be cut to the expected size (" +
                             std::to_string(_target->width) + "x" + std::to_string(_target->height) + "x" +
                             std::to_string(_target->channels) + " expected, " + std::to_string(dx) + "x" +
                             std::to_string(dy) + "x" + std::to_string(channels) +
                             " available) - ROI skipped. Check the ROI configuration.");
        return;
    }

    uint8_t* odata = _target->RGBImageLock();
    RGBImageLock();

    stbi_uc* p_target;
    stbi_uc* p_source;

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            // Pixels outside the image replicate the nearest edge pixel (see FitROIToImage).
            const int sx = std::min(std::max(x, 0), width - 1);
            const int sy = std::min(std::max(y, 0), height - 1);
            p_target = odata + (channels * ((y - y1) * dx + (x - x1)));
            p_source = rgb_image + (channels * (sy * width + sx));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

    RGBImageRelease();
    _target->RGBImageRelease();
}


CImageBasis* CAlignAndCutImage::CutAndSave(int x1, int y1, int dx, int dy)
{
    int x2, y2;

    FitROIToImage(x1, y1, dx, dy, width, height);

    // Always the full configured size: FitROIToImage has already decided whether the origin moves,
    // and the copy loop pads whatever still lies outside the image.
    x2 = x1 + dx;
    y2 = y1 + dy;

    int memsize = dx * dy * channels;
    uint8_t* odata = (unsigned char*)malloc_psram_heap(std::string(TAG) + "->odata", memsize, MALLOC_CAP_SPIRAM);

    stbi_uc* p_target;
    stbi_uc* p_source;

    RGBImageLock();

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            // Pixels outside the image replicate the nearest edge pixel (see FitROIToImage).
            const int sx = std::min(std::max(x, 0), width - 1);
            const int sy = std::min(std::max(y, 0), height - 1);
            p_target = odata + (channels * ((y - y1) * dx + (x - x1)));
            p_source = rgb_image + (channels * (sy * width + sx));
            for (int _channels = 0; _channels < channels; ++_channels)
                p_target[_channels] = p_source[_channels];
        }

    CImageBasis* rs = new CImageBasis("CutAndSave", odata, channels, dx, dy, bpp);
    RGBImageRelease();
    rs->SetIndepended();
    return rs;
}
