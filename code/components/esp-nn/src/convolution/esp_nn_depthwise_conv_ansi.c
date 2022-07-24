// Copyright 2020-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>

#include <common_functions.h>

int esp_nn_get_depthwise_conv_scratch_size_ansi(const uint16_t input_wd,
                                                const uint16_t input_ht,
                                                const uint16_t channels,
                                                const uint16_t ch_mult,
                                                const uint16_t filter_wd,
                                                const uint16_t filter_ht)
{
    return 0;
}

void esp_nn_set_depthwise_conv_scratch_buf_ansi(const void *buf)
{

}

void esp_nn_depthwise_conv_s8_ansi(const int8_t *input_data,
                                   const uint16_t input_wd,
                                   const uint16_t input_ht,
                                   const uint16_t channels,
                                   const int32_t input_offset,
                                   const uint16_t pad_wd,
                                   const uint16_t pad_ht,
                                   const uint16_t stride_wd,
                                   const uint16_t stride_ht,
                                   const uint16_t ch_mult,
                                   const int8_t *filter_data,
                                   const uint16_t filter_wd,
                                   const uint16_t filter_ht,
                                   const int32_t *bias,
                                   int8_t *out_data,
                                   const uint16_t out_wd,
                                   const uint16_t out_ht,
                                   const int32_t out_offset,
                                   const int32_t *out_shift,
                                   const int32_t *out_mult,
                                   const int32_t activation_min,
                                   const int32_t activation_max)
{
    int out_idx = 0;
    for (int out_y = 0; out_y < out_ht; out_y++) { //height loop
        const int16_t base_y = (out_y * stride_ht) - pad_ht;
        for (int out_x = 0; out_x < out_wd; out_x++) { //width_loop
            const int16_t base_x = (out_x * stride_wd) - pad_wd;
            for (int ch_idx = 0; ch_idx < channels; ch_idx++) {//channel_loop
                for (int ch_mult_idx = 0; ch_mult_idx < ch_mult; ch_mult_idx++) {
                    int32_t result = 0;
                    const int out_ch_idx = ch_mult_idx + ch_idx * ch_mult;

                    /* Select filter so as the point doesn't lie outside block */
                    int filter_y_start = max(0, -base_y);
                    int filter_x_start = max(0, -base_x);
                    int filter_y_end = min(filter_ht, input_ht - base_y);
                    int filter_x_end = min(filter_wd, input_wd - base_x);

                    for (int filter_y_idx = filter_y_start; filter_y_idx < filter_y_end; filter_y_idx++) {
                        const int32_t idx_y = base_y + filter_y_idx;
                        for (int filter_x_idx = filter_x_start; filter_x_idx < filter_x_end; filter_x_idx++) {
                            const int32_t idx_x = base_x + filter_x_idx;
                            int32_t input_index = (idx_y * input_wd + idx_x) * channels + ch_idx;
                            int32_t filter_index = (filter_y_idx * filter_wd + filter_x_idx) * (channels * ch_mult) + out_ch_idx;
                            int32_t input_val = input_data[input_index] + input_offset;
                            int32_t filter_val = filter_data[filter_index];
                            result += input_val * filter_val;
                        }
                    }
                    if (bias) {
                        result += bias[out_ch_idx];
                    }
                    result = esp_nn_multiply_by_quantized_mult(result, out_mult[out_ch_idx], out_shift[out_ch_idx]);
                    result += out_offset;
                    result = max(result, activation_min);
                    result = min(result, activation_max);

                    out_data[out_idx++] = result;
                }
            }
        }
    }
}
