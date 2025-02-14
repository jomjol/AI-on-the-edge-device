/********************************************************************************
 * https://github.com/RoboticsBrno/SmartLeds
 *
 * MIT License
 * 
 * Copyright (c) 2017 RoboticsBrno (RobotikaBrno)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#pragma once

#include "RmtDriver.h"

#if !SMARTLEDS_NEW_RMT_DRIVER
#include "Color.h"
#include <driver/rmt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace detail {

constexpr const int CHANNEL_COUNT = RMT_CHANNEL_MAX;

class RmtDriver {
public:
    RmtDriver(const LedType& timing, int count, int pin, int channel_num, SemaphoreHandle_t finishedFlag);
    RmtDriver(const RmtDriver&) = delete;

    esp_err_t init();
    esp_err_t registerIsr(bool isFirstRegisteredChannel);
    esp_err_t unregisterIsr();
    esp_err_t transmit(const Rgb* buffer);

private:
    static void IRAM_ATTR txEndCallback(rmt_channel_t channel, void* arg);

    static void IRAM_ATTR translateSample(const void* src, rmt_item32_t* dest, size_t src_size,
        size_t wanted_rmt_items_num, size_t* out_consumed_src_bytes, size_t* out_used_rmt_items);

    const LedType& _timing;
    int _count;
    gpio_num_t _pin;
    SemaphoreHandle_t _finishedFlag;

    rmt_channel_t _channel;
    rmt_item32_t _bitToRmt[2];
    size_t _translatorSourceOffset;
};

};
#endif // !SMARTLEDS_NEW_RMT_DRIVER
