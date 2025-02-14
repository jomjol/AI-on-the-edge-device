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

#include "RmtDriver5.h"

#if SMARTLEDS_NEW_RMT_DRIVER
#include <cstddef>

#include "SmartLeds.h"

namespace detail {

static constexpr const uint32_t RMT_RESOLUTION_HZ = 20 * 1000 * 1000; // 20 MHz
static constexpr const uint32_t RMT_NS_PER_TICK = 1000000000LLU / RMT_RESOLUTION_HZ;

static RmtEncoderWrapper* IRAM_ATTR encSelf(rmt_encoder_t* encoder) {
    return (RmtEncoderWrapper*)(((intptr_t)encoder) - offsetof(RmtEncoderWrapper, base));
}

static size_t IRAM_ATTR encEncode(rmt_encoder_t* encoder, rmt_channel_handle_t tx_channel, const void* primary_data,
    size_t data_size, rmt_encode_state_t* ret_state) {
    auto* self = encSelf(encoder);

    // Delay after last pixel
    if ((self->last_state & RMT_ENCODING_COMPLETE) && self->frame_idx == data_size) {
        *ret_state = (rmt_encode_state_t)0;
        return self->copy_encoder->encode(
            self->copy_encoder, tx_channel, (const void*)&self->reset_code, sizeof(self->reset_code), ret_state);
    }

    if (self->last_state & RMT_ENCODING_COMPLETE) {
        Rgb* pixel = ((Rgb*)primary_data) + self->frame_idx;
        self->buffer_len = sizeof(self->buffer);
        for (size_t i = 0; i < sizeof(self->buffer); ++i) {
            self->buffer[i] = pixel->getGrb(self->component_idx);
            if (++self->component_idx == 3) {
                self->component_idx = 0;
                if (++self->frame_idx == data_size) {
                    self->buffer_len = i + 1;
                    break;
                }
                ++pixel;
            }
        }
    }

    self->last_state = (rmt_encode_state_t)0;
    auto encoded_symbols = self->bytes_encoder->encode(
        self->bytes_encoder, tx_channel, (const void*)&self->buffer, self->buffer_len, &self->last_state);
    if (self->last_state & RMT_ENCODING_MEM_FULL) {
        *ret_state = RMT_ENCODING_MEM_FULL;
    } else {
        *ret_state = (rmt_encode_state_t)0;
    }

    return encoded_symbols;
}

static esp_err_t encReset(rmt_encoder_t* encoder) {
    auto* self = encSelf(encoder);
    rmt_encoder_reset(self->bytes_encoder);
    rmt_encoder_reset(self->copy_encoder);
    self->last_state = RMT_ENCODING_COMPLETE;
    self->frame_idx = 0;
    self->component_idx = 0;
    return ESP_OK;
}

static esp_err_t encDelete(rmt_encoder_t* encoder) {
    auto* self = encSelf(encoder);
    rmt_del_encoder(self->bytes_encoder);
    rmt_del_encoder(self->copy_encoder);
    return ESP_OK;
}

RmtDriver::RmtDriver(const LedType& timing, int count, int pin, int channel_num, SemaphoreHandle_t finishedFlag)
    : _timing(timing)
    , _count(count)
    , _pin(pin)
    , _finishedFlag(finishedFlag)
    , _channel(nullptr)
    , _encoder {} {}

esp_err_t RmtDriver::init() {
    _encoder.base.encode = encEncode;
    _encoder.base.reset = encReset;
    _encoder.base.del = encDelete;

    _encoder.reset_code.duration0 = _timing.TRS / RMT_NS_PER_TICK;

    rmt_bytes_encoder_config_t bytes_cfg = {
        .bit0 = {
            .duration0 = uint16_t(_timing.T0H  / RMT_NS_PER_TICK),
            .level0 = 1,
            .duration1 = uint16_t(_timing.T0L  / RMT_NS_PER_TICK),
            .level1 = 0,
        },
        .bit1 = {
            .duration0 = uint16_t(_timing.T1H / RMT_NS_PER_TICK),
            .level0 = 1,
            .duration1 = uint16_t(_timing.T1L  / RMT_NS_PER_TICK),
            .level1 = 0,
        },
        .flags = {
            .msb_first = 1,
        },
    };

    auto err = rmt_new_bytes_encoder(&bytes_cfg, &_encoder.bytes_encoder);
    if (err != ESP_OK) {
        return err;
    }

    rmt_copy_encoder_config_t copy_cfg = {};
    err = rmt_new_copy_encoder(&copy_cfg, &_encoder.copy_encoder);
    if (err != ESP_OK) {
        return err;
    }

    // The config must be in registerIsr, because rmt_new_tx_channel
    // registers the ISR
    return ESP_OK;
}

esp_err_t RmtDriver::registerIsr(bool isFirstRegisteredChannel) {
    rmt_tx_channel_config_t conf = {
        .gpio_num = (gpio_num_t)_pin,
        .clk_src = RMT_CLK_SRC_DEFAULT, //.clk_src = RMT_CLK_SRC_APB,
        .resolution_hz = RMT_RESOLUTION_HZ,
        .mem_block_symbols = SOC_RMT_MEM_WORDS_PER_CHANNEL,
        .trans_queue_depth = 1,
        .flags = {},
    };

    auto err = rmt_new_tx_channel(&conf, &_channel);
    if (err != ESP_OK) {
        return err;
    }

    rmt_tx_event_callbacks_t callbacks_cfg = {};
    callbacks_cfg.on_trans_done = txDoneCallback;

    err = rmt_tx_register_event_callbacks(_channel, &callbacks_cfg, this);
    if (err != ESP_OK) {
        return err;
    }

    return rmt_enable(_channel);
}

esp_err_t RmtDriver::unregisterIsr() {
    auto err = rmt_del_encoder(&_encoder.base);
    if (err != ESP_OK) {
        return err;
    }

    err = rmt_disable(_channel);
    if (err != ESP_OK) {
        return err;
    }

    return rmt_del_channel(_channel);
}

bool IRAM_ATTR RmtDriver::txDoneCallback(
    rmt_channel_handle_t tx_chan, const rmt_tx_done_event_data_t* edata, void* user_ctx) {
    auto* self = (RmtDriver*)user_ctx;
    auto taskWoken = pdTRUE;
    xSemaphoreGiveFromISR(self->_finishedFlag, &taskWoken);
    return taskWoken == pdTRUE;
}

esp_err_t RmtDriver::transmit(const Rgb* buffer) {
    rmt_encoder_reset(&_encoder.base);
    rmt_transmit_config_t cfg = {};
    return rmt_transmit(_channel, &_encoder.base, buffer, _count, &cfg);
}
};
#endif // !SMARTLEDS_NEW_RMT_DRIVER
