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

#include <esp_nn.h>
#include "test_utils.h"


void esp_nn_fully_connected_s8_test()
{
    /* prepare data */
    static uint16_t row_len = 256 + 8 + 7; /* odd len to test unaligned+left-over */
    static uint16_t out_channels = 3;
    int8_t input[row_len];
    int8_t filter_data[row_len * out_channels];
    int8_t output_c[out_channels], output_opt[out_channels];
    static int32_t activation_min = -128;
    static int32_t activation_max = 127;
    static int32_t input_offset = 0;
    static int32_t filter_offset = 0;
    int32_t out_shift = -10;
    static int32_t out_offset = 127;
    int32_t out_mult = 0x59e492c4;
    for (int itr = 0; itr < 5; itr++) {
        out_mult = INT32_MAX / row_len + rand() % INT16_MAX;
        switch (itr) {
        case 0:
            out_shift = -10;
            break;
        case 1:
            out_shift = SHIFT_MIN;
            break;
        case 2:
            out_shift = SHIFT_MAX;
            break;
        case 3:
            out_shift = 0;
            break;
        default:
            out_shift = -10 + rand() % 5;
            break;
        }
        if (itr == 0) {
            out_shift = SHIFT_MAX;
        }
        /* Generate input and filter data */
        for (int i = 0; i < row_len; ++i) {
            input[i] = rand() % 256 - 128;
        }
        for (int i = 0; i < row_len * out_channels; ++i) {
            filter_data[i] = rand() % 256 - 128;
        }

        if (itr == 0) {
            /* enable profiler */
            profile_c_start();
        }

        /* C function */
        esp_nn_fully_connected_s8_ansi(input, input_offset, row_len, filter_data, filter_offset,
                                    NULL, output_c, out_channels, out_offset, out_shift, out_mult,
                                    activation_min, activation_max);

        if (itr == 0) {
            profile_c_end();
            profile_opt_start();
        }

        /* Optimized function */
        esp_nn_fully_connected_s8(input, input_offset, row_len, filter_data, filter_offset,
                                NULL, output_opt, out_channels, out_offset, out_shift, out_mult,
                                activation_min, activation_max);

        if (itr == 0) {
            /* disable profiler */
            profile_opt_end();
        }

        bool ret = CHECK_EQUAL(output_c, output_opt, out_channels);
        if (ret == false) {
            printf(ANSI_COLOR_RED"%s[%d] failed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);
            printf("Output: \n");
            PRINT_ARRAY_HEX(output_opt, out_channels, 1);
            printf("Expected: \n");
            PRINT_ARRAY_HEX(output_c, out_channels, 1);
            printf("Input:\n");
            PRINT_ARRAY_HEX(input, row_len, 1);
            printf("Filter data:\n");
            PRINT_ARRAY_HEX(filter_data, row_len, out_channels);
            printf("Out shift: %d\n", out_shift);
            printf("Out mult: %x\n", out_mult);
            return;
        }
        printf(ANSI_COLOR_GREEN"%s[%d] passed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);
    }
}
