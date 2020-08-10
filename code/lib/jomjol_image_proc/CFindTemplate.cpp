#include "CFindTemplate.h"
#include "Helper.h"
#include "ClassLogFile.h"

#include "esp_system.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#define _ESP32_PSRAM

using namespace std;

#define GET_MEMORY malloc

/*
CResizeImage::CResizeImage(std::string _image, int _new_dx, int _new_dy)
{
    CImageBasis::CImageBasis(_image);
}
*/

uint8_t CImageBasis::GetPixelColor(int x, int y, int ch)
{
    stbi_uc* p_source;
    p_source = this->rgb_image + (this->channels * (y * this->width + x));
    return p_source[ch];
}

void CResizeImage::Resize(int _new_dx, int _new_dy)
{
    int memsize = _new_dx * _new_dy * this->channels;
    uint8_t* odata = (unsigned char*)GET_MEMORY(memsize);

    stbir_resize_uint8(this->rgb_image, this->width, this->height, 0, odata, _new_dx, _new_dy, 0, this->channels);

    stbi_image_free(this->rgb_image);
    this->rgb_image = (unsigned char*)GET_MEMORY(memsize);

    this->memCopy(odata, this->rgb_image, memsize);
    this->width = _new_dx;
    this->height = _new_dy;
    stbi_image_free(odata);
}

void CRotate::Rotate(float _angle, int _centerx, int _centery)
{
    float m[2][3];

    float x_center = _centerx;
    float y_center = _centery;
    _angle = _angle / 180 * M_PI;

    m[0][0] = cos(_angle);
    m[0][1] = sin(_angle);
    m[0][2] = (1 - m[0][0]) * x_center - m[0][1] * y_center;

    m[1][0] = -m[0][1];
    m[1][1] = m[0][0];
    m[1][2] = m[0][1] * x_center + (1 - m[0][0]) * y_center;

    int memsize = this->width * this->height * this->channels;
    uint8_t* odata = (unsigned char*)GET_MEMORY(memsize);

    int x_source, y_source;
    stbi_uc* p_target;
    stbi_uc* p_source;

    for (int x = 0; x < this->width; ++x)
        for (int y = 0; y < this->height; ++y)
        {
            p_target = odata + (this->channels * (y * this->width + x));

            x_source = int(m[0][0] * x + m[0][1] * y);
            y_source = int(m[1][0] * x + m[1][1] * y);

            x_source += int(m[0][2]);
            y_source += int(m[1][2]);

            if ((x_source >= 0) && (x_source < this->width) && (y_source >= 0) && (y_source < this->height))
            {
                p_source = this->rgb_image + (this->channels * (y_source * this->width + x_source));
                for (int channels = 0; channels < this->channels; ++channels)
                    p_target[channels] = p_source[channels];
            }
            else
            {
                for (int channels = 0; channels < this->channels; ++channels)
                    p_target[channels] = 255;
            }
        }

    //    memcpy(this->rgb_image, odata, memsize);
    this->memCopy(odata, this->rgb_image, memsize);
    stbi_image_free(odata);
}

void CRotate::Rotate(float _angle)
{
    this->Rotate(_angle, this->width / 2, this->height / 2);
}

void CRotate::Translate(int _dx, int _dy)
{
    int memsize = this->width * this->height * this->channels;
    uint8_t* odata = (unsigned char*)GET_MEMORY(memsize);


    int x_source, y_source;
    stbi_uc* p_target;
    stbi_uc* p_source;

    for (int x = 0; x < this->width; ++x)
        for (int y = 0; y < this->height; ++y)
        {
            p_target = odata + (this->channels * (y * this->width + x));

            x_source = x - _dx;
            y_source = y - _dy;

            if ((x_source >= 0) && (x_source < this->width) && (y_source >= 0) && (y_source < this->height))
            {
                p_source = this->rgb_image + (this->channels * (y_source * this->width + x_source));
                for (int channels = 0; channels < this->channels; ++channels)
                    p_target[channels] = p_source[channels];
            }
            else
            {
                for (int channels = 0; channels < this->channels; ++channels)
                    p_target[channels] = 255;
            }
        }

    //    memcpy(this->rgb_image, odata, memsize);
    this->memCopy(odata, this->rgb_image, memsize);
    stbi_image_free(odata);
}



CFindTemplate::CFindTemplate(std::string _image)
{
    this->channels = 1;
    this->rgb_image = stbi_load(_image.c_str(), &(this->width), &(this->height), &(this->bpp), this->channels);
}

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y)
{
    this->FindTemplate(_template, found_x, found_y, 0, 0);
}

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy)
{
    uint8_t* rgb_template = stbi_load(_template.c_str(), &tpl_width, &tpl_height, &tpl_bpp, this->channels);

    int ow, ow_start, ow_stop;
    int oh, oh_start, oh_stop;

    if (_dx == 0)
    {
        _dx = this->width;
        *found_x = 0;
    }

    if (_dy == 0)
    {
        _dy = this->height;
        *found_y = 0;
    }


    ow_start = *found_x - _dx;
    ow_start = std::max(ow_start, 0);
    ow_stop = *found_x + _dx;
    if ((ow_stop + tpl_width) > this->width)
        ow_stop = this->width - tpl_width;
    ow = ow_stop - ow_start + 1;

    oh_start = *found_y - _dy;
    oh_start = std::max(oh_start, 0);
    oh_stop = *found_y + _dy;
    if ((oh_stop + tpl_height) > this->height)
        oh_stop = this->height - tpl_height;
    oh = oh_stop - oh_start + 1;

    uint8_t* odata = (unsigned char*)GET_MEMORY(ow * oh * this->channels);

    double aktSAD;
    double minSAD = pow(tpl_width * tpl_height * 255, 2);

    for (int xouter = ow_start; xouter <= ow_stop; xouter++)
        for (int youter = oh_start; youter <= oh_stop; ++youter)
        {
            aktSAD = 0;
            for (int tpl_x = 0; tpl_x < tpl_width; tpl_x++)
                for (int tpl_y = 0; tpl_y < tpl_height; tpl_y++)
                {
                    stbi_uc* p_org = this->rgb_image + (this->channels * ((youter + tpl_y) * this->width + (xouter + tpl_x)));
                    stbi_uc* p_tpl = rgb_template + (this->channels * (tpl_y * tpl_width + tpl_x));
                    aktSAD += pow(p_tpl[0] - p_org[0], 2);
                }
            stbi_uc* p_out = odata + (this->channels * ((youter - oh_start) * ow + (xouter - ow_start)));

            p_out[0] = int(sqrt(aktSAD / (tpl_width * tpl_height)));
            if (aktSAD < minSAD)
            {
                minSAD = aktSAD;
                *found_x = xouter;
                *found_y = youter;
            }
        }

    stbi_write_bmp("sdcard\\find.bmp", ow, oh, this->channels, odata);

    stbi_image_free(odata);
    stbi_image_free(rgb_template);
}

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y, std::string _imageout)
{
    this->FindTemplate(_template, found_x, found_y);
    this->SaveToFile(_imageout);
}

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy, std::string _imageout)
{
    this->FindTemplate(_template, found_x, found_y, _dx, _dy);
    this->SaveToFile(_imageout);
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
    if ((x < 0) || (x > this->width - 1))
        return false;

    if ((y < 0) || (y > this->height- 1))
        return false;

    return true;
}

void CImageBasis::setPixelColor(int x, int y, int r, int g, int b)
{
    stbi_uc* p_source;

    p_source = this->rgb_image + (this->channels * (y * this->width + x));
    p_source[0] = r;
    if (this-> channels > 2)
    {
        p_source[1] = g;
        p_source[2] = b;
    }
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
    this->externalImage = false;
}

CImageBasis::CImageBasis(std::string _image)
{
    channels = 3;
    externalImage = false;
    filename = _image;

    rgb_image = stbi_load(_image.c_str(), &width, &height, &bpp, channels);
    if (rgb_image == NULL)
        LogFile.WriteToFile("Image Load failed:" + _image + " FreeHeapSize: " + to_string(esp_get_free_heap_size()));
    //    printf("CImageBasis after load\n");
    //    printf("w %d, h %d, b %d, c %d", this->width, this->height, this->bpp, this->channels);
}

CImageBasis::CImageBasis(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp)
{
    this->rgb_image = _rgb_image;
    this->channels = _channels;
    this->width = _width;
    this->height = _height;
    this->bpp = _bpp;
    this->externalImage = true;
}

CImageBasis::~CImageBasis()
{
    if (!this->externalImage)
        stbi_image_free(this->rgb_image);
}

void CImageBasis::SaveToFile(std::string _imageout)
{
    string typ = getFileType(_imageout);

    if ((typ == "jpg") || (typ == "JPG"))       // ACHTUNG PROBLEMATISCH IM ESP32
    {
        stbi_write_jpg(_imageout.c_str(), this->width, this->height, this->channels, this->rgb_image, 0);
    }

    if ((typ == "bmp") || (typ == "BMP"))
    {
        stbi_write_bmp(_imageout.c_str(), this->width, this->height, this->channels, this->rgb_image);
    }
    //    stbi_write_jpg(_imageout.c_str(), this->width, this->height, this->channels, this->rgb_image, 0);
    //      stbi_write_bmp(_imageout.c_str(), this->width, this->height, this->channels, this->rgb_image);
}



void CAlignAndCutImage::Align(std::string _template0, int ref0_x, int ref0_y, std::string _template1, int ref1_x, int ref1_y, int deltax, int deltay, std::string imageROI)
{
    int dx, dy;
    int r0_x, r0_y, r1_x, r1_y;

    CFindTemplate* ft = new CFindTemplate(this->filename);

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
        CImageBasis* imgzw = new CImageBasis(this->filename);
        imgzw->drawRect(r0_x, r0_y, t0_dx, t0_dy, 255, 0, 0, 2);
        imgzw->drawRect(r1_x, r1_y, t1_dx, t1_dy, 255, 0, 0, 2);
        imgzw->SaveToFile(imageROI);
        printf("Alignment: alignment ROI created: %s\n", imageROI.c_str());
        delete imgzw;
    }

    string zw = "\tdx:\t" + to_string(dx) + "\tdy:\t" + to_string(dy) + "\td_winkel:\t" + to_string(d_winkel);
    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", zw);

    CRotate rt(this->rgb_image, this->channels, this->width, this->height, this->bpp);
    rt.Translate(dx, dy);
    rt.Rotate(d_winkel, ref0_x, ref0_y);
    printf("Alignment: dx %d - dy %d - rot %f\n", dx, dy, d_winkel);
}

void CAlignAndCutImage::CutAndSave(std::string _template1, int x1, int y1, int dx, int dy)
{

    int x2, y2;

    x2 = x1 + dx;
    y2 = y1 + dy;
    x2 = min(x2, this->width - 1);
    y2 = min(y2, this->height - 1);

    dx = x2 - x1;
    dy = y2 - y1;

    int memsize = dx * dy * this->channels;
    uint8_t* odata = (unsigned char*)GET_MEMORY(memsize);


    int x_source, y_source;
    stbi_uc* p_target;
    stbi_uc* p_source;

    for (int x = x1; x < x2; ++x)
        for (int y = y1; y < y2; ++y)
        {
            p_target = odata + (this->channels * ((y - y1) * dx + (x - x1)));
            p_source = this->rgb_image + (this->channels * (y * this->width + x));
            for (int channels = 0; channels < this->channels; ++channels)
                p_target[channels] = p_source[channels];
        }

    //    stbi_write_jpg(_template1.c_str(), dx, dy, this->channels, odata, 0);
    stbi_write_bmp(_template1.c_str(), dx, dy, this->channels, odata);

    stbi_image_free(odata);
}
