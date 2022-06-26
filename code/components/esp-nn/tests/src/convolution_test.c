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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <esp_nn.h>
#include "test_utils.h"

#if CONFIG_IDF_CMAKE
#define IDF_HEAP_CAPS 1

#if IDF_HEAP_CAPS
#include "esp_heap_caps.h"
#endif
#endif

void esp_nn_depthwise_conv_s8_test()
{
    int8_t *input = NULL, *filter_data = NULL, *out_data_c = NULL, *out_data_opt = NULL;
    int32_t *bias = NULL;
    int32_t input_offset = 5; /* some number in [-128, 127] */
    int32_t out_offset = 7;
    int32_t activation_min = -125;
    int32_t activation_max = 120;
    void *scratch_buf = NULL;

    /* independent variables */
    int input_wd, input_ht, channels;
    uint16_t filter_ht, filter_wd, ch_mult;
    uint16_t pad_wd, pad_ht, stride_wd, stride_ht;

    // run for 10 iterations
    for (int itr = 0; itr < 10; itr++) {
        /* prepare data */
        switch (itr) {
        case 0: // (ch_mult 1, (channels % 16) = 0), filter (3,3), pad (0,0)
            input_wd = 18;
            input_ht = 18;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 1;
            channels = 16;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 1: // (ch_mult 1, (channels % 16) = 0), filter (3,3), pad (1,1)
            input_wd = 10;
            input_ht = 10;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 1;
            channels = 16;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 2: // (ch_mult 1, (channels % 8) = 0), filter (3,3), pad (1,1)
            input_wd = 10;
            input_ht = 10;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 1;
            channels = 24;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 3: // other filter sizes (ch_mult 1, (channels % 8) = 0)
            input_wd = 10;
            input_ht = 10;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 1;
            channels = 24;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 4: // other filter sizes (ch_mult 8 = 0)
            input_wd = 6;
            input_ht = 6;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 8;
            channels = 4;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 5: // other filter sizes (ch_mult 8 = 0)
            input_wd = 12;
            input_ht = 12;
            filter_ht = 5;
            filter_wd = 5;
            ch_mult = 8;
            channels = 4;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 6: // other filter sizes (ch_mult 4 = 0)
            input_wd = 6;
            input_ht = 6;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 4;
            channels = 4;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 7: // (ch_mult 1, (channels % 16) = 0), filter (3,3), pad (0,0)  stride (2,2)
            input_wd = 6;
            input_ht = 6;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 1;
            channels = 16;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 2;
            stride_ht = 2;
            break;
        default:
            input_wd = 4;
            input_ht = 4;
            filter_ht = 3;
            filter_wd = 3;
            ch_mult = 4;
            channels = 4;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        }

        uint16_t out_wd = (input_wd - filter_wd + 1) / stride_wd;
        uint16_t out_ht = (input_ht - filter_ht + 1) / stride_ht;
        int in_size = input_wd * input_ht * channels;
        int out_size = out_wd * out_ht * channels * ch_mult;
        int filter_size = filter_wd * filter_ht * channels * ch_mult + 4;
        int bias_size = channels * ch_mult + 1;
        int32_t out_shift[channels * ch_mult];
        int32_t out_mult[channels * ch_mult];

#if IDF_HEAP_CAPS
        int8_t *input_orig = (int8_t *) heap_caps_malloc(in_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        int8_t *out_c_orig = (int8_t *) heap_caps_malloc(out_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        int8_t *out_opt_orig = (int8_t *) heap_caps_malloc(out_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        filter_data = (int8_t *) heap_caps_malloc(filter_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        bias = (int32_t *) heap_caps_malloc(bias_size * 4, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        input = 16 + input_orig - ((uint32_t) input_orig & 0xf);
        out_data_c = 16 + out_c_orig - ((uint32_t) out_c_orig & 0xf);
        out_data_opt = 16 + out_opt_orig - ((uint32_t) out_opt_orig & 0xf);
#else
        input = memalign(16, in_size + 16);
        filter_data = memalign(16, filter_size);
        out_data_c = memalign(16, out_size + 16);
        out_data_opt = memalign(16, out_size + 16);
        bias = memalign(16, bias_size * 4);
        int8_t *input_orig = input;
        int8_t *out_c_orig = out_data_c;
        int8_t *out_opt_orig = out_data_opt;
#endif
        if (bias == NULL || input == NULL || filter_data == NULL ||
                out_data_c == NULL || out_data_opt == NULL || bias == NULL) {
            printf(ANSI_COLOR_RED"%s[%d] allocations failed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);
            goto dc_s8_cleanup;
        }

        /* Generate input data */
        for (int i = 0; i < in_size; ++i) {
            input[i] = rand() % 128;
        }

        /* Generate filter data */
        for (int i = 0; i < filter_size; ++i) {
            filter_data[i] = rand() % 256 - 128;
        }

        /* Generate bias data */
        for (int i = 0; i < channels * ch_mult; ++i) {
            bias[i + 1] = rand() % INT16_MAX; //0th index left for unalignment
            out_shift[i] = -8 + rand() % 3;
            out_mult[i] = 0x7eb0e200 + rand() % 50;
        }

        int scratch_buf_size = esp_nn_get_depthwise_conv_scratch_size(input_wd, input_ht,
                                                                    channels, ch_mult,
                                                                    filter_wd, filter_ht);
        if (scratch_buf_size > 0) {
#if IDF_HEAP_CAPS
            scratch_buf = heap_caps_malloc(scratch_buf_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            int align_sz = 16 - (((int32_t) scratch_buf) & 0xf);
#else
            scratch_buf = memalign(16, scratch_buf_size);
            int align_sz = 0;
#endif
            if (scratch_buf == NULL) {
                printf(ANSI_COLOR_RED"%s[%d] scratch_buf alloc failed size %d\n"ANSI_COLOR_RESET,
                       __FUNCTION__, itr, scratch_buf_size);
                goto dc_s8_cleanup;
            }
            esp_nn_set_depthwise_conv_scratch_buf(scratch_buf + align_sz);
        }
        if (itr == 0) {
            /* enable profiler */
            profile_c_start();
        }

        /* C function */
        esp_nn_depthwise_conv_s8_ansi(input, input_wd, input_ht, channels, input_offset,
                                    pad_wd, pad_ht, stride_wd, stride_ht, ch_mult,
                                    filter_data + 4, filter_wd, filter_ht,
                                    bias + 1, out_data_c, out_wd, out_ht, out_offset, out_shift,
                                    out_mult, activation_min, activation_max);

        if (itr == 0) {
            profile_c_end();
            profile_opt_start();
        }

        /* Optimized function */
        esp_nn_depthwise_conv_s8(input, input_wd, input_ht, channels, input_offset,
                                pad_wd, pad_ht, stride_wd, stride_ht, ch_mult,
                                filter_data + 4, filter_wd, filter_ht,
                                bias + 1, out_data_opt, out_wd, out_ht, out_offset, out_shift,
                                out_mult, activation_min, activation_max);

        if (itr == 0) {
            /* disable profiler */
            profile_opt_end();
        }

        bool ret = CHECK_EQUAL(out_data_c, out_data_opt, out_size);
        if (ret == false) {
            printf(ANSI_COLOR_RED"%s[%d] failed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);
            printf("Output: \n");
            PRINT_ARRAY_HEX(out_data_opt, out_size / out_ht, out_ht);
            printf("Expected: \n");
            PRINT_ARRAY_HEX(out_data_c, out_size / out_ht, out_ht);
            printf("Input:\n");
            PRINT_ARRAY_HEX(input, in_size / input_ht, input_ht);
            printf("Filter data:\n");
            PRINT_ARRAY_HEX(filter_data + 4, (filter_size - 4) / filter_ht, filter_ht);
            printf("bias data:\n");
            PRINT_ARRAY_INT(bias + 1, ch_mult * channels, 1);
            goto dc_s8_cleanup;
        }
        printf(ANSI_COLOR_GREEN"%s[%d] passed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);

    dc_s8_cleanup:
        if (input) {
            free(input_orig);
        }
        if (filter_data) {
            free(filter_data);
        }
        if (out_data_c) {
            free(out_c_orig);
        }
        if (out_data_opt) {
            free(out_opt_orig);
        }
        if (bias) {
            free(bias);
        }
        if (scratch_buf) {
            free(scratch_buf);
        }
    }
}

void esp_nn_conv_s8_test()
{
    const int32_t input_offset = 5; /* some number in [-128, 127] */
    const int32_t activation_min = -125;
    const int32_t activation_max = 122;
    const int32_t out_offset = 3;

    void *scratch_buf = NULL;
    int8_t *input_orig;
    int8_t *out_c_orig;
    int8_t *out_opt_orig;
    int8_t *filter_data;
    int32_t *bias;

    /* independent variable */
    int in_wd, in_ht, in_channels, out_channels;
    uint16_t filter_ht, filter_wd;
    uint16_t pad_wd, pad_ht, stride_wd, stride_ht;

    // run for 10 iterations
    for (int itr = 0; itr < 10; itr++) {
        switch (itr) {
        case 0: // ch % 8 == 0 && filter (1,1), padding (0,0)
            in_wd = 10;
            in_ht = 10;
            in_channels = 64;
            out_channels = 64;
            filter_ht = 1;
            filter_wd = 1;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 1: // ch % 4 == 0 && (in_wd * in_ht) % 16 == 0
            in_wd = 4;
            in_ht = 4;
            in_channels = 20;
            out_channels = 8;
            filter_ht = 1;
            filter_wd = 1;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 2: // ch, filter (3x3x3)
            in_wd = 10;
            in_ht = 10;
            in_channels = 3;
            out_channels = 64;
            filter_ht = 3;
            filter_wd = 3;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 3: // remaining pad (0, 0)
            in_wd = 10;
            in_ht = 10;
            in_channels = 3;
            out_channels = 64;
            filter_ht = 1;
            filter_wd = 1;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 4: // unopt case
            in_wd = 10;
            in_ht = 10;
            in_channels = 12;
            out_channels = 64;
            filter_ht = 3;
            filter_wd = 3;
            pad_wd = 1;
            pad_ht = 1;
            stride_wd = 1;
            stride_ht = 1;
            break;
        case 5: // ch % 8 == 0 & stride (2,2)
            in_wd = 16;
            in_ht = 16;
            in_channels = 16;
            out_channels = 16;
            filter_ht = 1;
            filter_wd = 1;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 2;
            stride_ht = 2;
            break;
        case 6: // ch % 8 == 0 && filter (1,1), padding (0,0)
            in_wd = 2;
            in_ht = 2;
            in_channels = 8;
            out_channels = 8;
            filter_ht = 1;
            filter_wd = 1;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 1;
            stride_ht = 1;
            break;
        default: // ch % 8 == 0
            in_wd = 8;
            in_ht = 8;
            in_channels = 16;
            out_channels = 16;
            filter_ht = 1;
            filter_wd = 1;
            pad_wd = 0;
            pad_ht = 0;
            stride_wd = 1;
            stride_ht = 1;
            break;
        }

        /* prepare data */
        uint16_t out_wd = (in_wd - filter_wd + 1) / stride_wd;
        uint16_t out_ht = (in_ht - filter_ht + 1) / stride_ht;

        int in_size = in_wd * in_ht * in_channels;
        int filter_size = filter_wd * filter_ht * in_channels * out_channels + 2;
        int out_size = out_wd * out_ht * out_channels;

#if IDF_HEAP_CAPS
        input_orig = (int8_t *) heap_caps_malloc(in_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        out_c_orig = (int8_t *) heap_caps_malloc(out_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        out_opt_orig = (int8_t *) heap_caps_malloc(out_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        filter_data = (int8_t *) heap_caps_malloc(filter_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        bias = (int32_t *) heap_caps_malloc(128 + sizeof (int32_t) * out_channels, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        int8_t *input = 16 + input_orig - ((uint32_t) input_orig & 0xf);
        int8_t *out_data_c = 16 + out_c_orig - ((uint32_t) out_c_orig & 0xf);
        int8_t *out_data_opt = 16 + out_opt_orig - ((uint32_t) out_opt_orig & 0xf);
#else
        int8_t *input = memalign(16, in_size);
        int8_t *out_data_c = memalign(16, out_size);
        int8_t *out_data_opt = memalign(16, out_size);
        filter_data = memalign(16, filter_size);
        bias = calloc(1, 128 + sizeof (int32_t) * out_channels);
        input_orig = input;
        out_c_orig = out_data_c;
        out_opt_orig = out_data_opt;
#endif
        int32_t *out_shift = calloc(1, 128 + sizeof (int32_t) * out_channels);
        int32_t *out_mult = calloc(1, 128 + sizeof (int32_t) * out_channels);

        if (input == NULL || filter_data == NULL ||
                out_data_c == NULL || out_data_opt == NULL) {
            printf(ANSI_COLOR_RED"%s allocations failed\n"ANSI_COLOR_RESET, __FUNCTION__);
            goto conv_s8_cleanup;
        }

        if (bias == NULL || out_shift == NULL || out_mult == NULL) {
            printf(ANSI_COLOR_RED"%s allocations failed\n"ANSI_COLOR_RESET, __FUNCTION__);
            goto conv_s8_cleanup;
        }

        /* Generate input data between -128 -> +127 */
        for (int i = 0; i < in_size; ++i) {
            input[i] = rand() % 255 - 128;
        }

        /* Generate filter data between -128 -> +127 */
        for (int i = 0; i < filter_size; ++i) {
            filter_data[i] = rand() % 256 - 128;
        }

        /* Generate bias data */
        for (int i = 0; i < out_channels; ++i) {
            bias[i] = (int32_t)rand() % UINT16_MAX + UINT8_MAX;
        }

        /* Shift and multiplier */
        for (int i = 0; i < out_channels; ++i) {
            out_shift[i] = -10 + rand() % 2;
            out_mult[i] = 0x7f67f4f8 + rand() % 50;
        }

        int scratch_buf_size = esp_nn_get_conv_scratch_size(in_wd, in_ht, in_channels,
                                                            out_channels, filter_wd, filter_ht);
        if (scratch_buf_size > 0) {
#if IDF_HEAP_CAPS
            void *scratch_buf = heap_caps_malloc(scratch_buf_size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            int align_sz = 16 - (((int32_t) scratch_buf) & 0xf);
#else
            void *scratch_buf = memalign(16, scratch_buf_size);
            int align_sz = 0;
#endif
            if (scratch_buf == NULL) {
                printf(ANSI_COLOR_RED"%s scratch_buf alloc failed size %d\n"ANSI_COLOR_RESET, __FUNCTION__, scratch_buf_size);
                goto conv_s8_cleanup;
            }
            esp_nn_set_conv_scratch_buf(scratch_buf + align_sz);
        }

        if (itr == 0) {
            /* enable profiler */
            profile_c_start();
        }

        /* C function */
        esp_nn_conv_s8_ansi(input, in_wd, in_ht, in_channels, input_offset,
                            pad_wd, pad_ht, stride_wd, stride_ht,
                            filter_data + 2, filter_wd, filter_ht, bias,
                            out_data_c, out_wd, out_ht, out_channels, out_offset, out_shift,
                            out_mult, activation_min, activation_max);

        if (itr == 0) {
            profile_c_end();
            profile_opt_start();
        }

        /* Optimized function */
        esp_nn_conv_s8(input, in_wd, in_ht, in_channels, input_offset,
                    pad_wd, pad_ht, stride_wd, stride_ht,
                    filter_data + 2, filter_wd, filter_ht, bias,
                    out_data_opt, out_wd, out_ht, out_channels, out_offset, out_shift,
                    out_mult, activation_min, activation_max);

        if (itr == 0) {
            /* disable profiler */
            profile_opt_end();
        }

        bool ret = CHECK_EQUAL(out_data_c, out_data_opt, out_size);
        if (ret == false) {
            printf(ANSI_COLOR_RED"%s[%d] failed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);
            printf("Output: \n");
            PRINT_ARRAY_HEX(out_data_opt, out_size / out_ht, out_ht);
            printf("Expected: \n");
            PRINT_ARRAY_HEX(out_data_c, out_size / out_ht, out_ht);
            printf("Input:\n");
            PRINT_ARRAY_HEX(input, in_size / in_ht, in_ht);
            printf("Filter data:\n");
            PRINT_ARRAY_HEX(filter_data + 2, (filter_size - 2) / filter_ht, filter_ht);
            printf("bias data:\n");
            PRINT_ARRAY_INT(bias, out_channels, 1);
            goto conv_s8_cleanup;
        }
        printf(ANSI_COLOR_GREEN"%s[%d] passed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);

    conv_s8_cleanup:
        if (input) {
            free(input_orig);
        }
        if (filter_data) {
            free(filter_data);
        }
        if (out_data_c) {
            free(out_c_orig);
        }
        if (out_data_opt) {
            free(out_opt_orig);
        }
        if (bias) {
            free(bias);
        }
        if (out_shift) {
            free(out_shift);
        }
        if (out_mult) {
            free(out_mult);
        }
        if (scratch_buf) {
            free(scratch_buf);
        }
    }
}
