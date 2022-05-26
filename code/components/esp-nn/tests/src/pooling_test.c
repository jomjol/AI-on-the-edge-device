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


void esp_nn_avg_pool_s8_test()
{
    /* prepare data */
    const uint16_t input_wd = 16;
    const uint16_t input_ht = 16;
    const uint16_t channels = 16; /* With TFLite example, I have seen it 256 */
    const int size = input_wd * input_ht * channels;
    int8_t *input, *output_c, *output_opt;
    const int32_t activation_min = -128;
    const int32_t activation_max = 127;
    const uint16_t pad_wd = 1;
    const uint16_t pad_ht = 1;
    const uint16_t stride_wd = 1;
    const uint16_t stride_ht = 1;
    const uint16_t filter_ht = 3;
    const uint16_t filter_wd = 3;
    const uint16_t out_wd = input_wd / stride_wd;
    const uint16_t out_ht = input_ht / stride_ht;
    const int out_size = out_wd * out_ht * channels;

    input = memalign(16, size);
    output_c = memalign(16, out_size);
    output_opt = memalign(16, out_size);

    if (input == NULL || output_c == NULL || output_opt == NULL) {
        printf(ANSI_COLOR_RED"%s allocations failed\n"ANSI_COLOR_RESET, __FUNCTION__);
        goto avg_pool_s8_cleanup;
    }
    /**
     * width/height, channels etc look suspicious but it it true.
     * It actually depends upon where in model this is actually placed.
     * If at the end wd/ht tends to be smaller and depth larger.
     */

    for (int i = 0; i < size; ++i) {
        input[i] = rand() % 256 - 128;
    }

    /* enable profiler */
    profile_c_start();

    /* C function */
    esp_nn_avg_pool_s8_ansi(input, input_wd, input_ht, output_c, out_wd, out_ht,
                              stride_wd, stride_ht, filter_wd, filter_ht, pad_wd, pad_ht,
                              activation_min, activation_max, channels);

    profile_c_end();
    profile_opt_start();

    /* Optimized function */
    esp_nn_avg_pool_s8(input, input_wd, input_ht, output_opt, out_wd, out_ht,
                         stride_wd, stride_ht, filter_wd, filter_ht, pad_wd, pad_ht,
                         activation_min, activation_max, channels);

    /* disable profiler */
    profile_opt_end();


    bool ret = CHECK_EQUAL(output_c, output_opt, out_size);
    if (ret == false) {
        printf(ANSI_COLOR_RED"%s failed\n"ANSI_COLOR_RESET, __FUNCTION__);
        printf("Output: \n");
        PRINT_ARRAY_HEX(output_opt, out_wd * channels, out_ht);
        printf("Expected: \n");
        PRINT_ARRAY_HEX(output_c, out_wd * channels, out_ht);
        printf("Input:\n");
        PRINT_ARRAY_HEX(input, input_wd * channels, input_ht);
        goto avg_pool_s8_cleanup;
    }
    printf(ANSI_COLOR_GREEN"%s passed\n"ANSI_COLOR_RESET, __FUNCTION__);

avg_pool_s8_cleanup:
    if (input) {
        free(input);
    }
    if (output_c) {
        free(output_c);
    }
    if (output_opt) {
        free(output_opt);
    }
}

void esp_nn_max_pool_s8_test()
{
    /* prepare data */
    const uint16_t input_wd = 16;
    const uint16_t input_ht = 16;
    const uint16_t channels = 16; /* With TFLite example, I have seen it 256 */
    int8_t *input, *output_c, *output_opt;
    const int size = input_wd * input_ht * channels;
    const int32_t activation_min = -128;
    const int32_t activation_max = 127;
    const uint16_t pad_wd = 1;
    const uint16_t pad_ht = 1;
    const uint16_t stride_wd = 1;
    const uint16_t stride_ht = 1;
    const uint16_t filter_ht = 3;
    const uint16_t filter_wd = 3;
    const uint16_t out_wd = input_wd / stride_wd;
    const uint16_t out_ht = input_ht / stride_ht;
    const int out_size = out_wd * out_ht * channels;

    input = memalign(16, size);
    output_c = memalign(16, out_size);
    output_opt = memalign(16, out_size);

    if (input == NULL || output_c == NULL || output_opt == NULL) {
        printf(ANSI_COLOR_RED"%s allocations failed\n"ANSI_COLOR_RESET, __FUNCTION__);
        goto max_pool_s8_cleanup;
    }

    for (int i = 0; i < size; ++i) {
        input[i] = rand() % 256 - 128;
    }

    /* enable profiler */
    profile_c_start();

    /* C function */
    esp_nn_max_pool_s8_ansi(input, input_wd, input_ht, output_c, out_wd, out_ht,
                            stride_wd, stride_ht, filter_wd, filter_ht, pad_wd, pad_ht,
                            activation_min, activation_max, channels);

    profile_c_end();
    profile_opt_start();

    /* Optimized function */
    esp_nn_max_pool_s8(input, input_wd, input_ht, output_opt, out_wd, out_ht,
                       stride_wd, stride_ht, filter_wd, filter_ht, pad_wd, pad_ht,
                       activation_min, activation_max, channels);

    /* disable profiler */
    profile_opt_end();


    bool ret = CHECK_EQUAL(output_c, output_opt, out_wd * out_ht * channels);
    if (ret == false) {
        printf(ANSI_COLOR_RED"%s failed\n"ANSI_COLOR_RESET, __FUNCTION__);
        printf("Output: \n");
        PRINT_ARRAY_HEX(output_opt, out_wd * out_ht * channels, 1);
        printf("Expected: \n");
        PRINT_ARRAY_HEX(output_c, out_wd * out_ht * channels, 1);
        printf("Input:\n");
        PRINT_ARRAY_HEX(input, 8, size / 8);
        goto max_pool_s8_cleanup;
    }
    printf(ANSI_COLOR_GREEN"%s passed\n"ANSI_COLOR_RESET, __FUNCTION__);

max_pool_s8_cleanup:
    if (input) {
        free(input);
    }
    if (output_c) {
        free(output_c);
    }
    if (output_opt) {
        free(output_opt);
    }
}
