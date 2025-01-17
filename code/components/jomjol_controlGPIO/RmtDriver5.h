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

#if SMARTLEDS_NEW_RMT_DRIVER
#include <driver/rmt_tx.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <type_traits>

#include "Color.h"

#if !defined(CONFIG_RMT_ISR_IRAM_SAFE) && !defined(SMARTLEDS_DISABLE_IRAM_WARNING)
#warning "Please enable CONFIG_RMT_ISR_IRAM_SAFE IDF option." \
    "without it, the IDF driver is not able to supply data fast enough."
#endif

namespace detail {

constexpr const int CHANNEL_COUNT = SOC_RMT_GROUPS * SOC_RMT_CHANNELS_PER_GROUP;

class RmtDriver;

// This is ridiculous
struct RmtEncoderWrapper {
    struct rmt_encoder_t base;
    struct rmt_encoder_t* bytes_encoder;
    struct rmt_encoder_t* copy_encoder;
    RmtDriver* driver;
    rmt_symbol_word_t reset_code;

    uint8_t buffer[SOC_RMT_MEM_WORDS_PER_CHANNEL / 8];
    rmt_encode_state_t last_state;
    size_t frame_idx;
    uint8_t component_idx;
    uint8_t buffer_len;
};

static_assert(std::is_standard_layout<RmtEncoderWrapper>::value == true);

class RmtDriver {
public:
    RmtDriver(const LedType& timing, int count, int pin, int channel_num, SemaphoreHandle_t finishedFlag);
    RmtDriver(const RmtDriver&) = delete;

    esp_err_t init();
    esp_err_t registerIsr(bool isFirstRegisteredChannel);
    esp_err_t unregisterIsr();
    esp_err_t transmit(const Rgb* buffer);

private:
    static bool IRAM_ATTR txDoneCallback(
        rmt_channel_handle_t tx_chan, const rmt_tx_done_event_data_t* edata, void* user_ctx);

    const LedType& _timing;
    int _count;
    int _pin;
    SemaphoreHandle_t _finishedFlag;

    rmt_channel_handle_t _channel;
    RmtEncoderWrapper _encoder;
};

};
#endif // !SMARTLEDS_NEW_RMT_DRIVER
