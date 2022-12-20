#include "CFindTemplate.h"

#include "ClassLogFile.h"
#include "Helper.h"
#include "../../include/defines.h"

#include <esp_log.h>

static const char* TAG = "C FIND TEMPL";

// #define DEBUG_DETAIL_ON  


bool CFindTemplate::FindTemplate(RefInfo *_ref)
{
    uint8_t* rgb_template;

    if (file_size(_ref->image_file.c_str()) == 0) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, _ref->image_file + " is empty!");
        return false;
    }
   
    rgb_template = stbi_load(_ref->image_file.c_str(), &tpl_width, &tpl_height, &tpl_bpp, channels);

    if (rgb_template == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to load " + _ref->image_file + "! Is it corrupted?");
        return false;
    }

//    ESP_LOGD(TAG, "FindTemplate 01");

    int ow, ow_start, ow_stop;
    int oh, oh_start, oh_stop;

    if (_ref->search_x == 0)
    {
        _ref->search_x = width;
        _ref->found_x = 0;
    }

    if (_ref->search_y == 0)
    {
        _ref->search_y = height;
        _ref->found_y = 0;
    }


    ow_start = _ref->target_x - _ref->search_x;
    ow_start = std::max(ow_start, 0);
    ow_stop = _ref->target_x + _ref->search_x;
    if ((ow_stop + tpl_width) > width)
        ow_stop = width - tpl_width;
    ow = ow_stop - ow_start + 1;

    oh_start = _ref->target_y - _ref->search_y;
    oh_start = std::max(oh_start, 0);
    oh_stop = _ref->target_y + _ref->search_y;
    if ((oh_stop + tpl_height) > height)
        oh_stop = height - tpl_height;
    oh = oh_stop - oh_start + 1;

    float avg, SAD;
    int min, max;
    bool isSimilar = false;

//    ESP_LOGD(TAG, "FindTemplate 02");

    if ((_ref->alignment_algo == 2) && (_ref->fastalg_x > -1) && (_ref->fastalg_y > -1))     // für Testzwecke immer Berechnen
    {
        isSimilar = CalculateSimularities(rgb_template, _ref->fastalg_x, _ref->fastalg_y, ow, oh, min, avg, max, SAD, _ref->fastalg_SAD, _ref->fastalg_SAD_criteria);
/*#ifdef DEBUG_DETAIL_ON
        std::string zw = "\t" + _ref->image_file + "\tt1_x_y:\t" + std::to_string(_ref->fastalg_x) + "\t" + std::to_string(_ref->fastalg_y);
        zw = zw + "\tpara1_found_min_avg_max_SAD:\t" + std::to_string(min) + "\t" + std::to_string(avg) + "\t" + std::to_string(max) + "\t"+ std::to_string(SAD);
        LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", zw);
#endif*/
    }

//    ESP_LOGD(TAG, "FindTemplate 03");


    if (isSimilar)
    {
#ifdef DEBUG_DETAIL_ON  
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Use FastAlignment sucessfull");
#endif
        _ref->found_x = _ref->fastalg_x;
        _ref->found_y = _ref->fastalg_y;

        stbi_image_free(rgb_template);
        
        return true;
    }

//    ESP_LOGD(TAG, "FindTemplate 04");


    double aktSAD;
    double minSAD = pow(tpl_width * tpl_height * 255, 2);

    RGBImageLock();

//    ESP_LOGD(TAG, "FindTemplate 05");
    int xouter, youter, tpl_x, tpl_y, _ch;
    int _anzchannels = channels;
    if (_ref->alignment_algo == 0)  // 0 = "Default" (nur R-Kanal)
        _anzchannels = 1;

    for (xouter = ow_start; xouter <= ow_stop; xouter++)
        for (youter = oh_start; youter <= oh_stop; ++youter)
        {
            aktSAD = 0;
            for (tpl_x = 0; tpl_x < tpl_width; tpl_x++)
                for (tpl_y = 0; tpl_y < tpl_height; tpl_y++)
                {
                    stbi_uc* p_org = rgb_image + (channels * ((youter + tpl_y) * width + (xouter + tpl_x)));
                    stbi_uc* p_tpl = rgb_template + (channels * (tpl_y * tpl_width + tpl_x));
                    for (_ch = 0; _ch < _anzchannels; ++_ch)
                    {
                        aktSAD += pow(p_tpl[_ch] - p_org[_ch], 2);
                    }
                }
            if (aktSAD < minSAD)
            {
                minSAD = aktSAD;
                _ref->found_x = xouter;
                _ref->found_y = youter;
            }
        }

//    ESP_LOGD(TAG, "FindTemplate 06");


    if (_ref->alignment_algo == 2)
        CalculateSimularities(rgb_template, _ref->found_x, _ref->found_y, ow, oh, min, avg, max, SAD, _ref->fastalg_SAD, _ref->fastalg_SAD_criteria);


//    ESP_LOGD(TAG, "FindTemplate 07");

    _ref->fastalg_x = _ref->found_x;
    _ref->fastalg_y = _ref->found_y;
    _ref->fastalg_min = min;
    _ref->fastalg_avg = avg;
    _ref->fastalg_max = max;
    _ref->fastalg_SAD = SAD;

    
/*#ifdef DEBUG_DETAIL_ON
    std::string zw = "\t" + _ref->image_file + "\tt1_x_y:\t" + std::to_string(_ref->fastalg_x) + "\t" + std::to_string(_ref->fastalg_y);
    zw = zw + "\tpara1_found_min_avg_max_SAD:\t" + std::to_string(min) + "\t" + std::to_string(avg) + "\t" + std::to_string(max) + "\t"+ std::to_string(SAD);
    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", zw);
#endif*/

    RGBImageRelease();
    stbi_image_free(rgb_template);
    
//    ESP_LOGD(TAG, "FindTemplate 08");

    return false;
}



bool CFindTemplate::CalculateSimularities(uint8_t* _rgb_tmpl, int _startx, int _starty, int _sizex, int _sizey, int &min, float &avg, int &max, float &SAD, float _SADold, float _SADcrit)
{
    int dif;
    int minDif = 255;
    int maxDif = -255;
    double avgDifSum = 0;
    long int anz = 0;
    double aktSAD = 0;    

    int xouter, youter, _ch;

    for (xouter = 0; xouter <= _sizex; xouter++)
        for (youter = 0; youter <= _sizey; ++youter)
        {
            stbi_uc* p_org = rgb_image + (channels * ((youter + _starty) * width + (xouter + _startx)));
            stbi_uc* p_tpl = _rgb_tmpl + (channels * (youter * tpl_width + xouter));
            for (_ch = 0; _ch < channels; ++_ch)
            {
                dif = p_tpl[_ch] - p_org[_ch];
                aktSAD += pow(p_tpl[_ch] - p_org[_ch], 2);                
                if (dif < minDif) minDif = dif;
                if (dif > maxDif) maxDif = dif;
                avgDifSum += dif;
                anz++;
            }
        }

    avg = avgDifSum / anz;
    min = minDif;
    max = maxDif;
    SAD = sqrt(aktSAD) / anz;

    float _SADdif = abs(SAD - _SADold);

    ESP_LOGD(TAG, "Anzahl %ld, avgDifSum %fd, avg %f, SAD_neu: %fd, _SAD_old: %f, _SAD_crit:%f", anz, avgDifSum, avg, SAD, _SADold, _SADdif);

    if (_SADdif <= _SADcrit)
        return true;

    return false;
}



