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
#include <stdio.h>

#include <common_functions.h>

static int16_t *scratch_buffer = NULL;

extern void esp_nn_conv_s16_mult8_1x1_esp32s3(const int8_t *input_data,
                                              const uint16_t input_wd,
                                              const uint16_t input_ht,
                                              const uint16_t in_channels,
                                              const int32_t input_offset,
                                              const int16_t *filter_data,
                                              const int32_t *bias,
                                              int8_t *out_data,
                                              const uint16_t out_wd,
                                              const uint16_t out_ht,
                                              const uint16_t out_channels,
                                              const int32_t out_offset,
                                              const int32_t *out_shift,
                                              const int32_t *out_mult,
                                              const int32_t activation_min,
                                              const int32_t activation_max,
                                              void *buffer /* scratch buffer */);

extern void esp_nn_conv_s16_mult4_1x1_esp32s3(const int16_t *input_data,
                                              const uint16_t input_wd,
                                              const uint16_t input_ht,
                                              const uint16_t in_channels,
                                              const int16_t *filter_data,
                                              const int32_t *bias,
                                              int8_t *out_data,
                                              const uint16_t out_wd,
                                              const uint16_t out_ht,
                                              const uint16_t out_channels,
                                              const int32_t out_offset,
                                              const int32_t *out_shift,
                                              const int32_t *out_mult,
                                              const int32_t activation_min,
                                              const int32_t activation_max,
                                              void *buffer /* scratch buffer */);

extern void esp_nn_conv_s16_mult8_esp32s3(const int16_t *input_data,
                                          const uint16_t input_wd,
                                          const uint16_t input_ht,
                                          const uint16_t in_channels,
                                          const uint16_t pad_wd,
                                          const uint16_t pad_ht,
                                          const uint16_t stride_wd,
                                          const uint16_t stride_ht,
                                          const int16_t *filter_data,
                                          const uint16_t filter_wd,
                                          const uint16_t filter_ht,
                                          const int32_t *bias,
                                          int8_t *out_data,
                                          const uint16_t out_wd,
                                          const uint16_t out_ht,
                                          const uint16_t out_channels,
                                          const int32_t out_offset,
                                          const int32_t *out_shift,
                                          const int32_t *out_mult,
                                          const int32_t activation_min,
                                          const int32_t activation_max);

extern void esp_nn_aligned_s8_to_s16_with_offset_esp32s3(const int8_t *src, int16_t *dst,
                                                         const int size, const int32_t offset);

extern void esp_nn_s8_to_s16_esp32s3(const int8_t *src, int16_t *dst, const int size);

static void esp_nn_conv_s8_unrolled(const int8_t *input_data,
                                    const uint16_t input_wd,
                                    const uint16_t input_ht,
                                    const uint16_t in_channels,
                                    const int32_t input_offset,
                                    const uint16_t pad_wd,
                                    const uint16_t pad_ht,
                                    const uint16_t stride_wd,
                                    const uint16_t stride_ht,
                                    const int8_t *filter_data,
                                    const uint16_t filter_wd,
                                    const uint16_t filter_ht,
                                    const int32_t *bias,
                                    int8_t *out_data,
                                    const uint16_t out_wd,
                                    const uint16_t out_ht,
                                    const uint16_t out_channels,
                                    const int32_t out_offset,
                                    const int32_t *out_shift,
                                    const int32_t *out_mult,
                                    const int32_t activation_min,
                                    const int32_t activation_max)
{
    int32_t out_ch_idx, out_y, out_x, in_ch_idx, filter_y_idx, filter_x_idx;

    for (out_y = 0; out_y < out_ht; out_y++) {
        for (out_x = 0; out_x < out_wd; out_x++) {
            for (out_ch_idx = 0; out_ch_idx < out_channels; out_ch_idx++) {
                int32_t conv_out = 0;

                const int32_t base_y = stride_ht * out_y - pad_ht;
                const int32_t base_x = stride_wd * out_x - pad_wd;

                const int32_t filter_y_start = max(0, -base_y);
                const int32_t filter_x_start = max(0, -base_x);

                const int32_t filter_y_end = min(filter_ht, input_ht - base_y);
                const int32_t filter_x_end = min(filter_wd, input_wd - base_x);

                for (filter_y_idx = filter_y_start; filter_y_idx < filter_y_end; filter_y_idx++) {
                    for (filter_x_idx = filter_x_start; filter_x_idx < filter_x_end; filter_x_idx++) {
                        const int32_t in_row = base_y + filter_y_idx;
                        const int32_t in_col = base_x + filter_x_idx;
                        int32_t input_base_offset = (in_row * input_wd + in_col) * in_channels;
                        int32_t filter_base_offset = out_ch_idx * in_channels * filter_ht * filter_wd +
                                                       (filter_y_idx * filter_wd + filter_x_idx) * in_channels;
                        for (in_ch_idx = 0; in_ch_idx < in_channels; in_ch_idx++) {
                            conv_out +=
                                (input_data[input_base_offset + in_ch_idx] + input_offset) *
                                filter_data[filter_base_offset + in_ch_idx];
                        }
                    }
                }
                if (bias) {
                    conv_out += bias[out_ch_idx];
                }
                conv_out = esp_nn_multiply_by_quantized_mult_fast(conv_out, out_mult[out_ch_idx], out_shift[out_ch_idx]);
                conv_out += out_offset;
                conv_out = max(conv_out, activation_min);
                conv_out = min(conv_out, activation_max);
                *out_data++ = (int8_t) conv_out;
            }
        }
    }
}

static void esp_nn_conv_s8_pad_valid(const int8_t *input_data,
                                     const uint16_t input_wd,
                                     const uint16_t input_ht,
                                     const uint16_t in_channels,
                                     const int32_t input_offset,
                                     const uint16_t stride_wd,
                                     const uint16_t stride_ht,
                                     const int8_t *filter_data,
                                     const uint16_t filter_wd,
                                     const uint16_t filter_ht,
                                     const int32_t *bias,
                                     int8_t *out_data,
                                     const uint16_t out_wd,
                                     const uint16_t out_ht,
                                     const uint16_t out_channels,
                                     const int32_t out_offset,
                                     const int32_t *out_shift,
                                     const int32_t *out_mult,
                                     const int32_t activation_min,
                                     const int32_t activation_max)
{
    int32_t out_ch_idx, out_y, out_x, in_ch_idx, filter_y_idx, filter_x_idx;

    for (out_y = 0; out_y < out_ht; out_y++) {
        for (out_x = 0; out_x < out_wd; out_x++) {
            for (out_ch_idx = 0; out_ch_idx < out_channels; out_ch_idx++) {
                int32_t conv_out = 0;

                const int32_t base_y = stride_ht * out_y;
                const int32_t base_x = stride_wd * out_x;

                for (filter_y_idx = 0; filter_y_idx < filter_ht; filter_y_idx++) {
                    for (filter_x_idx = 0; filter_x_idx < filter_wd; filter_x_idx++) {
                        const int32_t in_row = base_y + filter_y_idx;
                        const int32_t in_col = base_x + filter_x_idx;
                        int32_t input_base_offset = (in_row * input_wd + in_col) * in_channels;
                        int32_t filter_base_offset = out_ch_idx * in_channels * filter_ht * filter_wd +
                                                       (filter_y_idx * filter_wd + filter_x_idx) * in_channels;
                        const int8_t *input_data_ptr = input_data + input_base_offset;
                        const int8_t *filter_data_ptr = filter_data + filter_base_offset;
                        for (in_ch_idx = 0; in_ch_idx < in_channels; in_ch_idx++) {
                            conv_out += (*input_data_ptr++ + input_offset) * *filter_data_ptr++;
                        }
                    }
                }
                if (bias) {
                    conv_out += bias[out_ch_idx];
                }
                conv_out = esp_nn_multiply_by_quantized_mult_fast(conv_out, out_mult[out_ch_idx], out_shift[out_ch_idx]);
                conv_out += out_offset;
                conv_out = max(conv_out, activation_min);
                conv_out = min(conv_out, activation_max);
                *out_data++ = (int8_t) conv_out;
            }
        }
    }
}

static void esp_nn_conv_s8_pad_valid_3x3(const int8_t *input_data,
                                         const uint16_t input_wd,
                                         const uint16_t input_ht,
                                         const uint16_t in_channels,
                                         const int32_t input_offset,
                                         const uint16_t stride_wd,
                                         const uint16_t stride_ht,
                                         const int8_t *filter_data,
                                         const int32_t *bias,
                                         int8_t *out_data,
                                         const uint16_t out_wd,
                                         const uint16_t out_ht,
                                         const uint16_t out_channels,
                                         const int32_t out_offset,
                                         const int32_t *out_shift,
                                         const int32_t *out_mult,
                                         const int32_t activation_min,
                                         const int32_t activation_max)
{
    int32_t out_ch_idx, out_y, out_x, in_ch_idx, filter_y_idx, filter_x_idx;

    for (out_y = 0; out_y < out_ht; out_y++) {
        for (out_x = 0; out_x < out_wd; out_x++) {
            const int32_t base_y = stride_ht * out_y;
            const int32_t base_x = stride_wd * out_x;
            for (out_ch_idx = 0; out_ch_idx < out_channels; out_ch_idx++) {
                int32_t conv_out = 0;
                for (filter_y_idx = 0; filter_y_idx < 3; filter_y_idx++) {
                    for (filter_x_idx = 0; filter_x_idx < 3; filter_x_idx++) {
                        const int32_t in_row = base_y + filter_y_idx;
                        const int32_t in_col = base_x + filter_x_idx;
                        int32_t input_base_offset = (in_row * input_wd + in_col) * in_channels;
                        int32_t filter_base_offset = out_ch_idx * in_channels * 3 * 3 +
                                                       (filter_y_idx * 3 + filter_x_idx) * in_channels;
                        const int8_t *input_data_ptr = input_data + input_base_offset;
                        const int8_t *filter_data_ptr = filter_data + filter_base_offset;
                        for (in_ch_idx = 0; in_ch_idx < in_channels; in_ch_idx++) {
                            conv_out += (*input_data_ptr++ + input_offset) * *filter_data_ptr++;
                        }
                    }
                }
                if (bias) {
                    conv_out += bias[out_ch_idx];
                }
                conv_out = esp_nn_multiply_by_quantized_mult_fast(conv_out, out_mult[out_ch_idx], out_shift[out_ch_idx]);
                conv_out += out_offset;
                conv_out = max(conv_out, activation_min);
                conv_out = min(conv_out, activation_max);
                *out_data++ = (int8_t) conv_out;
            }
        }
    }
}

static void esp_nn_conv_s8_pad_valid_ch3_3x3(const int8_t *input_data,
                                             const uint16_t input_wd,
                                             const uint16_t input_ht,
                                             const int32_t input_offset,
                                             const uint16_t stride_wd,
                                             const uint16_t stride_ht,
                                             const int8_t *filter_data,
                                             const int32_t *bias,
                                             int8_t *out_data,
                                             const uint16_t out_wd,
                                             const uint16_t out_ht,
                                             const uint16_t out_channels,
                                             const int32_t out_offset,
                                             const int32_t *out_shift,
                                             const int32_t *out_mult,
                                             const int32_t activation_min,
                                             const int32_t activation_max)
{
    int32_t out_ch_idx, out_y, out_x, filter_y_idx;

    /* use scratch_buffer to pre-compute offset factor */
    int16_t *filter_sum = (int16_t *) scratch_buffer;
    const int8_t *filter_ptr = filter_data;
    for (out_ch_idx = 0; out_ch_idx < out_channels; out_ch_idx++) {
        int16_t sum_val = 0;
        for (int i = 0; i < 9; i++) {
            sum_val += *filter_ptr++;
            sum_val += *filter_ptr++;
            sum_val += *filter_ptr++;
        }
        *filter_sum++ = sum_val;
    }

    for (out_y = 0; out_y < out_ht; out_y++) {
        for (out_x = 0; out_x < out_wd; out_x++) {
            const int8_t *filter_data_ptr = filter_data;
            const int32_t base_y = stride_ht * out_y;
            const int32_t base_x = stride_wd * out_x;
            const int8_t *input_base_ptr = input_data + (base_y * input_wd + base_x) * 3;
            int16_t *filter_sum = (int16_t *) scratch_buffer;
            for (out_ch_idx = 0; out_ch_idx < out_channels; out_ch_idx++) {
                int32_t conv_out = 0;

                for (filter_y_idx = 0; filter_y_idx < 3; filter_y_idx++) {
                    const int8_t *input_data_ptr = input_base_ptr + (filter_y_idx * input_wd) * 3;
                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);
                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);
                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);

                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);
                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);
                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);

                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);
                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);
                    conv_out += (*input_data_ptr++) * (*filter_data_ptr++);
                }

                conv_out += *filter_sum++ * input_offset;

                if (bias) {
                    conv_out += bias[out_ch_idx];
                }
                conv_out = esp_nn_multiply_by_quantized_mult_fast(conv_out, out_mult[out_ch_idx], out_shift[out_ch_idx]);
                conv_out += out_offset;
                conv_out = max(conv_out, activation_min);
                conv_out = min(conv_out, activation_max);
                *out_data++ = (int8_t) conv_out;
            }
        }
    }
}

int esp_nn_get_conv_scratch_size_esp32s3(const uint16_t input_wd,
                                         const uint16_t input_ht,
                                         const uint16_t in_ch,
                                         const uint16_t out_ch,
                                         const uint16_t filter_wd,
                                         const uint16_t filter_ht)
{
    int filter_size = filter_wd * filter_ht * in_ch * out_ch;
    int input_size = input_wd * input_ht * in_ch;
    int transpose_buf_size = 8 * in_ch; /* to store intermediate data */
    int align_buf_size = 32; /* extra buffer for alignment */
    return 2 * (filter_size + input_size +  transpose_buf_size) + align_buf_size;
}

void esp_nn_set_conv_scratch_buf_esp32s3(void *buf)
{
    scratch_buffer = (int16_t *) buf;
}

void esp_nn_conv_s8_esp32s3(const int8_t *input,
                            const uint16_t input_wd,
                            const uint16_t input_ht,
                            const uint16_t channels,
                            const int32_t input_offset,
                            const uint16_t pad_wd,
                            const uint16_t pad_ht,
                            const uint16_t stride_wd,
                            const uint16_t stride_ht,
                            const int8_t *filter_data,
                            const uint16_t filter_wd,
                            const uint16_t filter_ht,
                            const int32_t *bias,
                            int8_t *out_data,
                            const uint16_t out_wd,
                            const uint16_t out_ht,
                            const uint16_t out_channels,
                            const int32_t out_offset,
                            const int32_t *out_shift,
                            const int32_t *out_mult,
                            const int32_t activation_min,
                            const int32_t activation_max)
{
    int filter_size = filter_wd * filter_ht * channels * out_channels;
    int input_size = input_wd * input_ht * channels;
    int align_len = 16 - (filter_size & 15);
    int16_t *filter_data16 = scratch_buffer;
    int16_t *input_data16 = scratch_buffer + filter_size + align_len;

    if (scratch_buffer == NULL) {
        printf("esp_nn_conv error! scratch_buffer not set!\n");
        return;
    }

    if (channels % 8 == 0 && filter_wd == 1 && filter_ht == 1 &&
            pad_wd == 0 && pad_ht == 0 && stride_wd == 1 && stride_ht == 1) {
        int scratch_offset = (int) (filter_data16 + filter_size);
        void *scratch_buf = (void *) (scratch_offset + 16 - (scratch_offset & 15));
        esp_nn_s8_to_s16_esp32s3(filter_data, filter_data16, filter_size);
        esp_nn_conv_s16_mult8_1x1_esp32s3(
            input, input_wd, input_ht, channels, input_offset, filter_data16,
            bias, out_data, out_wd, out_ht, out_channels, out_offset,
            out_shift, out_mult, activation_min, activation_max, scratch_buf);
    } else if (channels % 4 == 0 && filter_wd == 1 && filter_ht == 1 &&
            (input_wd * input_ht) % 16 == 0 && /* TODO: remove this check */
            pad_wd == 0 && pad_ht == 0 && stride_wd == 1 && stride_ht == 1) {
        int scratch_offset = (int) (input_data16 + input_size);
        void *scratch_buf = (void *) (scratch_offset + 16 - (scratch_offset & 15));
        esp_nn_s8_to_s16_esp32s3(filter_data, filter_data16, filter_size);
        esp_nn_aligned_s8_to_s16_with_offset_esp32s3(input, input_data16, input_size, input_offset);
        esp_nn_conv_s16_mult4_1x1_esp32s3(
            input_data16, input_wd, input_ht, channels, filter_data16,
            bias, out_data, out_wd, out_ht, out_channels, out_offset,
            out_shift, out_mult, activation_min, activation_max, scratch_buf);
    } else if (channels % 8 == 0) {
        esp_nn_s8_to_s16_esp32s3(filter_data, filter_data16, filter_size);
        esp_nn_aligned_s8_to_s16_with_offset_esp32s3(input, input_data16, input_size, input_offset);
        esp_nn_conv_s16_mult8_esp32s3(
            input_data16, input_wd, input_ht, channels, pad_wd, pad_ht,
            stride_wd, stride_ht, filter_data16, filter_wd, filter_ht, bias,
            out_data, out_wd, out_ht, out_channels, out_offset, out_shift,
            out_mult, activation_min, activation_max);
    } else if (pad_wd == 0 && pad_ht == 0) {
        if (filter_wd == 3 && filter_ht == 3 && channels == 3) {
            esp_nn_conv_s8_pad_valid_ch3_3x3(input, input_wd, input_ht, input_offset,
                                             stride_wd, stride_ht, filter_data, bias,
                                             out_data, out_wd, out_ht, out_channels, out_offset,
                                             out_shift, out_mult, activation_min, activation_max);
        } else {
            esp_nn_conv_s8_pad_valid(input, input_wd, input_ht, channels, input_offset,
                                     stride_wd, stride_ht, filter_data, filter_wd, filter_ht, bias,
                                     out_data, out_wd, out_ht, out_channels, out_offset, out_shift,
                                     out_mult, activation_min, activation_max);
        }
    } else {
        /* Basic unrolled version */
        esp_nn_conv_s8_unrolled(input, input_wd, input_ht, channels, input_offset,
                                pad_wd, pad_ht, stride_wd, stride_ht,
                                filter_data, filter_wd, filter_ht, bias,
                                out_data, out_wd, out_ht, out_channels, out_offset, out_shift,
                                out_mult, activation_min, activation_max);
    }
}
