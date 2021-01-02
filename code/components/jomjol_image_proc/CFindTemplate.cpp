#include "CFindTemplate.h"

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y)
{
    FindTemplate(_template, found_x, found_y, 0, 0);
}

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy)
{
    uint8_t* rgb_template = stbi_load(_template.c_str(), &tpl_width, &tpl_height, &tpl_bpp, channels);

    int ow, ow_start, ow_stop;
    int oh, oh_start, oh_stop;

    if (_dx == 0)
    {
        _dx = width;
        *found_x = 0;
    }

    if (_dy == 0)
    {
        _dy = height;
        *found_y = 0;
    }


    ow_start = *found_x - _dx;
    ow_start = std::max(ow_start, 0);
    ow_stop = *found_x + _dx;
    if ((ow_stop + tpl_width) > width)
        ow_stop = width - tpl_width;
    ow = ow_stop - ow_start + 1;

    oh_start = *found_y - _dy;
    oh_start = std::max(oh_start, 0);
    oh_stop = *found_y + _dy;
    if ((oh_stop + tpl_height) > height)
        oh_stop = height - tpl_height;
    oh = oh_stop - oh_start + 1;

    uint8_t* odata = (unsigned char*)GET_MEMORY(ow * oh * channels);

    double aktSAD;
    double minSAD = pow(tpl_width * tpl_height * 255, 2);

    RGBImageLock();

    for (int xouter = ow_start; xouter <= ow_stop; xouter++)
        for (int youter = oh_start; youter <= oh_stop; ++youter)
        {
            aktSAD = 0;
            for (int tpl_x = 0; tpl_x < tpl_width; tpl_x++)
                for (int tpl_y = 0; tpl_y < tpl_height; tpl_y++)
                {
                    stbi_uc* p_org = rgb_image + (channels * ((youter + tpl_y) * width + (xouter + tpl_x)));
                    stbi_uc* p_tpl = rgb_template + (channels * (tpl_y * tpl_width + tpl_x));
                    aktSAD += pow(p_tpl[0] - p_org[0], 2);
                }
            stbi_uc* p_out = odata + (channels * ((youter - oh_start) * ow + (xouter - ow_start)));

            p_out[0] = int(sqrt(aktSAD / (tpl_width * tpl_height)));
            if (aktSAD < minSAD)
            {
                minSAD = aktSAD;
                *found_x = xouter;
                *found_y = youter;
            }
        }

    RGBImageRelease();

    stbi_write_bmp("sdcard\\find.bmp", ow, oh, channels, odata);

    stbi_image_free(odata);
    stbi_image_free(rgb_template);
}

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y, std::string _imageout)
{
    FindTemplate(_template, found_x, found_y);
    SaveToFile(_imageout);
}

void CFindTemplate::FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy, std::string _imageout)
{
    FindTemplate(_template, found_x, found_y, _dx, _dy);
    SaveToFile(_imageout);
}


