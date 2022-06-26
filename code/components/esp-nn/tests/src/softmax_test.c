// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
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

void esp_nn_softmax_s8_test()
{
    const int32_t height = 8;
    const int32_t width = 32;
    const int32_t diff_min = -128;
    const int32_t mult = INT32_MAX / 2;
    const int32_t shift = 7;
    void *scratch_buf = NULL;
    const int size = width * height;
    int8_t *input, *out_ansi, *out_opt;

    input = memalign(16, size);
    out_ansi = memalign(16, size);
    out_opt = memalign(16, size);

    if (input == NULL || out_ansi == NULL || out_opt == NULL) {
        printf(ANSI_COLOR_RED"%s buffer allocations failed\n"ANSI_COLOR_RESET, __FUNCTION__);
        goto softmax_s8_cleanup;
    }

    /* Generate input data between -128 -> +127 */
    for (int i = 0; i < size; ++i) {
        input[i] = rand() % 255 - 128;
    }

    /* enable profiler */
    profile_c_start();

    /* C function */
    esp_nn_softmax_s8_ansi(input, height, width, mult, shift, diff_min, out_ansi);

    profile_c_end();

    int32_t scratch_buf_size = esp_nn_get_softmax_scratch_size(width, height);
    if (scratch_buf_size) {
        scratch_buf = memalign(4, scratch_buf_size);
        if (scratch_buf == NULL) {
            printf(ANSI_COLOR_RED"%s scratch_buf alloc failed size %d\n"ANSI_COLOR_RESET, __FUNCTION__, scratch_buf_size);
            goto softmax_s8_cleanup;
        }
        esp_nn_set_softmax_scratch_buf(scratch_buf);
    }

    profile_opt_start();

    /* Optimized function */
    esp_nn_softmax_s8(input, height, width, mult, shift, diff_min, out_opt);

    /* disable profiler */
    profile_opt_end();

    bool ret = CHECK_EQUAL(out_ansi, out_opt, size);
    if (ret == false) {
        printf(ANSI_COLOR_RED"%s failed\n"ANSI_COLOR_RESET, __FUNCTION__);
        printf("Output: \n");
        PRINT_ARRAY_HEX(out_opt, width, height);
        printf("Expected: \n");
        PRINT_ARRAY_HEX(out_ansi, width, height);
        printf("Input:\n");
        PRINT_ARRAY_HEX(input, width, height);
        goto softmax_s8_cleanup;
    }
    printf(ANSI_COLOR_GREEN"%s passed\n"ANSI_COLOR_RESET, __FUNCTION__);

softmax_s8_cleanup:
    if (input) {
        free (input);
    }
    if (out_ansi) {
        free (out_ansi);
    }
    if (out_opt) {
        free (out_opt);
    }
    if (scratch_buf) {
        free (scratch_buf);
    }
}
