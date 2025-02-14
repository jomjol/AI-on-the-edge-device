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

#include "RmtDriver4.h"

#if !SMARTLEDS_NEW_RMT_DRIVER
#include "SmartLeds.h"

namespace detail {

// 8 still seems to work, but timings become marginal
static const int DIVIDER = 4;
// minimum time of a single RMT duration based on clock ns
static const double RMT_DURATION_NS = 12.5;

RmtDriver::RmtDriver(const LedType& timing, int count, int pin, int channel_num, SemaphoreHandle_t finishedFlag)
    : _timing(timing)
    , _count(count)
    , _pin((gpio_num_t)pin)
    , _finishedFlag(finishedFlag)
    , _channel((rmt_channel_t)channel_num) {
    _bitToRmt[0].level0 = 1;
    _bitToRmt[0].level1 = 0;
    _bitToRmt[0].duration0 = _timing.T0H / (RMT_DURATION_NS * DIVIDER);
    _bitToRmt[0].duration1 = _timing.T0L / (RMT_DURATION_NS * DIVIDER);

    _bitToRmt[1].level0 = 1;
    _bitToRmt[1].level1 = 0;
    _bitToRmt[1].duration0 = _timing.T1H / (RMT_DURATION_NS * DIVIDER);
    _bitToRmt[1].duration1 = _timing.T1L / (RMT_DURATION_NS * DIVIDER);
}

esp_err_t RmtDriver::init() {
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(_pin, _channel);
    config.rmt_mode = RMT_MODE_TX;
    config.clk_div = DIVIDER;
    config.mem_block_num = 1;

    return rmt_config(&config);
}

esp_err_t RmtDriver::registerIsr(bool isFirstRegisteredChannel) {
    auto err = rmt_driver_install(_channel, 0,
#if defined(CONFIG_RMT_ISR_IRAM_SAFE)
        ESP_INTR_FLAG_IRAM
#else
        0
#endif
    );
    if (err != ESP_OK) {
        return err;
    }

    if (isFirstRegisteredChannel) {
        rmt_register_tx_end_callback(txEndCallback, NULL);
    }

    err = rmt_translator_init(_channel, translateSample);
    if (err != ESP_OK) {
        return err;
    }
    return rmt_translator_set_context(_channel, this);
}

esp_err_t RmtDriver::unregisterIsr() { return rmt_driver_uninstall(_channel); }

void IRAM_ATTR RmtDriver::txEndCallback(rmt_channel_t channel, void* arg) {
    xSemaphoreGiveFromISR(SmartLed::ledForChannel(channel)->_finishedFlag, nullptr);
}

void IRAM_ATTR RmtDriver::translateSample(const void* src, rmt_item32_t* dest, size_t src_size,
    size_t wanted_rmt_items_num, size_t* out_consumed_src_bytes, size_t* out_used_rmt_items) {
    RmtDriver* self;
    ESP_ERROR_CHECK(rmt_translator_get_context(out_used_rmt_items, (void**)&self));

    const auto& _bitToRmt = self->_bitToRmt;
    const auto src_offset = self->_translatorSourceOffset;

    auto* src_components = (const uint8_t*)src;
    size_t consumed_src_bytes = 0;
    size_t used_rmt_items = 0;

    while (consumed_src_bytes < src_size && used_rmt_items + 7 < wanted_rmt_items_num) {
        uint8_t val = *src_components;

        // each bit, from highest to lowest
        for (uint8_t j = 0; j != 8; j++, val <<= 1) {
            dest->val = _bitToRmt[val >> 7].val;
            ++dest;
        }

        used_rmt_items += 8;
        ++src_components;
        ++consumed_src_bytes;

        // skip alpha byte
        if (((src_offset + consumed_src_bytes) % 4) == 3) {
            ++src_components;
            ++consumed_src_bytes;

            // TRST delay after last pixel in strip
            if (consumed_src_bytes == src_size) {
                (dest - 1)->duration1 = self->_timing.TRS / (detail::RMT_DURATION_NS * detail::DIVIDER);
            }
        }
    }

    self->_translatorSourceOffset = src_offset + consumed_src_bytes;
    *out_consumed_src_bytes = consumed_src_bytes;
    *out_used_rmt_items = used_rmt_items;
}

esp_err_t RmtDriver::transmit(const Rgb* buffer) {
    static_assert(sizeof(Rgb) == 4); // The translator code above assumes RGB is 4 bytes

    _translatorSourceOffset = 0;
    return rmt_write_sample(_channel, (const uint8_t*)buffer, _count * 4, false);
}
};
#endif // !SMARTLEDS_NEW_RMT_DRIVER
