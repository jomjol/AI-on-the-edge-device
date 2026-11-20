#include "CFindTemplate.h"

#include "ClassLogFile.h"
#include "Helper.h"
#include "defines.h"

#include <esp_log.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdint>
#include <climits>
#include <algorithm>

static const char *TAG = "CFindTemplate";

bool CFindTemplate::FindTemplate(RefInfo *_ref)
{
    tpl_width = tpl_height = tpl_bpp = 0;

    if (_ref == nullptr || rgb_image == nullptr || width <= 0 || height <= 0 || channels <= 0)
    {
        ESP_LOGE(TAG, "FindTemplate: invalid source image or reference");
        return false;
    }

    _ref->found_x = -1;
    _ref->found_y = -1;

    if (file_size(_ref->image_file.c_str()) == 0)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, _ref->image_file + " is empty or unavailable!");
        return false;
    }

    uint8_t *rgb_template = stbi_load(_ref->image_file.c_str(), &tpl_width, &tpl_height, &tpl_bpp, channels);

    if (rgb_template == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to load " + _ref->image_file + "! Is it corrupted?");
        tpl_width = tpl_height = tpl_bpp = 0;
        return false;
    }

    size_t templateBytes = 0;

    if (tpl_width <= 0 || tpl_height <= 0 || !CalculateImageSize(tpl_width, tpl_height, channels, &templateBytes) || tpl_width > width || tpl_height > height)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "FindTemplate: invalid template dimensions");
        stbi_image_free(rgb_template);
        return false;
    }

    (void)templateBytes;

    // Clamp the search rectangle using 64-bit intermediates so invalid
    // config coordinates cannot overflow signed int arithmetic.
    if (_ref->search_x == 0)
    {
        _ref->search_x = width;
    }

    if (_ref->search_y == 0)
    {
        _ref->search_y = height;
    }

    const int64_t searchX = std::max<int64_t>(0, _ref->search_x);
    const int64_t searchY = std::max<int64_t>(0, _ref->search_y);
    const int64_t owStart64 = std::max<int64_t>(0, static_cast<int64_t>(_ref->target_x) - searchX);
    const int64_t ohStart64 = std::max<int64_t>(0, static_cast<int64_t>(_ref->target_y) - searchY);
    const int64_t owStop64 = std::min<int64_t>(static_cast<int64_t>(width) - tpl_width, static_cast<int64_t>(_ref->target_x) + searchX);
    const int64_t ohStop64 = std::min<int64_t>(static_cast<int64_t>(height) - tpl_height, static_cast<int64_t>(_ref->target_y) + searchY);

    if (owStop64 < owStart64 || ohStop64 < ohStart64)
    {
        ESP_LOGW(TAG, "FindTemplate: search area does not intersect the source image");
        stbi_image_free(rgb_template);
        return false;
    }

    const int ow_start = static_cast<int>(owStart64);
    const int oh_start = static_cast<int>(ohStart64);
    const int ow_stop = static_cast<int>(owStop64);
    const int oh_stop = static_cast<int>(ohStop64);

    uint8_t *sourceData = RGBImageLock();

    if (sourceData == nullptr)
    {
        RGBImageRelease();
        stbi_image_free(rgb_template);
        ESP_LOGE(TAG, "FindTemplate: source image is not available");
        return false;
    }

    float avg = 0, SAD = 0;
    int min = 0, max = 0;
    bool isSimilar = false;

    if ((_ref->alignment_algo == 2) && (_ref->fastalg_x > -1) && (_ref->fastalg_y > -1))
    {
        isSimilar = CalculateSimularities(rgb_template, _ref->fastalg_x, _ref->fastalg_y, min, avg, max, SAD, _ref->fastalg_SAD, _ref->fastalg_SAD_criteria);
    }

    if (isSimilar)
    {
        _ref->found_x = _ref->fastalg_x;
        _ref->found_y = _ref->fastalg_y;

        RGBImageRelease();
        stbi_image_free(rgb_template);

        return true;
    }

    uint64_t aktSAD;
    uint64_t minSAD = UINT64_MAX;
    bool foundCandidate = false;
    int _anzchannels = channels;

    if (_ref->alignment_algo == 0)
    {
        _anzchannels = 1;
    }

    const size_t tplRowLength = static_cast<size_t>(tpl_width) * static_cast<size_t>(channels);

    if (tplRowLength > static_cast<size_t>(INT_MAX))
    {
        RGBImageRelease();
        stbi_image_free(rgb_template);
        ESP_LOGE(TAG, "FindTemplate: template row is too large");
        return false;
    }

    // Keep the search bounded while periodically yielding to the Wi-Fi tasks.
    const int tpl_row_len = static_cast<int>(tplRowLength);
    int64_t nextYieldUs = esp_timer_get_time() + 50000;

    for (int xouter = ow_start; xouter <= ow_stop; ++xouter)
    {
        for (int youter = oh_start; youter <= oh_stop; ++youter)
        {
            aktSAD = 0;

            for (int tpl_y = 0; tpl_y < tpl_height; ++tpl_y)
            {
                const stbi_uc *p_org = sourceData + (static_cast<size_t>(youter + tpl_y) * static_cast<size_t>(width) + static_cast<size_t>(xouter)) * static_cast<size_t>(channels);
                const stbi_uc *p_tpl = rgb_template + static_cast<size_t>(tpl_y) * tplRowLength;
                uint64_t rowSAD = 0;

                for (int tpl_x = 0; tpl_x < tpl_row_len; tpl_x += channels)
                {
                    for (int ch = 0; ch < _anzchannels; ++ch)
                    {
                        const int dif = static_cast<int>(p_tpl[tpl_x + ch]) - static_cast<int>(p_org[tpl_x + ch]);
                        rowSAD += static_cast<uint64_t>(dif * dif);
                    }
                }

                aktSAD += rowSAD;

                if (aktSAD >= minSAD)
                {
                    break;
                }
            }

            if (aktSAD < minSAD)
            {
                minSAD = aktSAD;
                _ref->found_x = xouter;
                _ref->found_y = youter;
                foundCandidate = true;
            }

            if (esp_timer_get_time() >= nextYieldUs)
            {
                vTaskDelay(1);
                nextYieldUs = esp_timer_get_time() + 50000;
            }
        }
    }

    if (foundCandidate && _ref->alignment_algo == 2)
    {
        CalculateSimularities(rgb_template, _ref->found_x, _ref->found_y, min, avg, max, SAD, _ref->fastalg_SAD, _ref->fastalg_SAD_criteria);
    }

    if (foundCandidate)
    {
        _ref->fastalg_x = _ref->found_x;
        _ref->fastalg_y = _ref->found_y;
        _ref->fastalg_min = min;
        _ref->fastalg_avg = avg;
        _ref->fastalg_max = max;
        _ref->fastalg_SAD = SAD;
    }

    RGBImageRelease();
    stbi_image_free(rgb_template);

    return isSimilar;
}

// Compares the template with the image at (_startx, _starty) and reports whether the match is still as
// good as the stored one. Used by the "Fast" alignment to skip the full search.
bool CFindTemplate::CalculateSimularities(uint8_t *_rgb_tmpl, int _startx, int _starty, int &min, float &avg, int &max, float &SAD, float _SADold, float _SADcrit)
{
    int dif;
    int minDif = 255;
    int maxDif = -255;
    double avgDifSum = 0;
    long int anz = 0;
    double aktSAD = 0;

    int xouter, youter, _ch;

    // The template has to lie completely inside the image at this position. The stored position comes
    // from /config/align.txt and may belong to an older reference image.
    if (_rgb_tmpl == nullptr || rgb_image == nullptr || channels <= 0 || tpl_width <= 0 || tpl_height <= 0 || _startx < 0 || _starty < 0 || static_cast<int64_t>(_startx) + tpl_width > width || static_cast<int64_t>(_starty) + tpl_height > height)
    {
        return false;
    }

    // Walk the template, not the search area: the former loop ran over the search area size (plus one)
    // and so read past the end of the template buffer and compared pixels that are not part of the marker.
    for (xouter = 0; xouter < tpl_width; xouter++)
        for (youter = 0; youter < tpl_height; ++youter)
        {
            const size_t orgIndex = (static_cast<size_t>(youter + _starty) * static_cast<size_t>(width) + static_cast<size_t>(xouter + _startx)) * static_cast<size_t>(channels);
            const size_t templateIndex = (static_cast<size_t>(youter) * static_cast<size_t>(tpl_width) + static_cast<size_t>(xouter)) * static_cast<size_t>(channels);
            stbi_uc *p_org = rgb_image + orgIndex;
            stbi_uc *p_tpl = _rgb_tmpl + templateIndex;

            for (_ch = 0; _ch < channels; ++_ch)
            {
                dif = (int)p_tpl[_ch] - (int)p_org[_ch];
                aktSAD += dif * dif;

                if (dif < minDif)
                {
                    minDif = dif;
                }

                if (dif > maxDif)
                {
                    maxDif = dif;
                }

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
    {
        return true;
    }

    return false;
}
