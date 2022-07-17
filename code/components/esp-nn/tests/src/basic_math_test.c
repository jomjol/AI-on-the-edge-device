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

#include <common_functions.h>
#include <esp_nn.h>
#include "test_utils.h"

#if CONFIG_IDF_CMAKE
#if (CONFIG_SPIRAM_SUPPORT && (CONFIG_SPIRAM_USE_CAPS_ALLOC || CONFIG_SPIRAM_USE_MALLOC))
#define IDF_HEAP_CAPS 1
#endif

#if IDF_HEAP_CAPS
#include "esp_heap_caps.h"
#endif
#endif

void esp_nn_add_elementwise_s8_test()
{
    /* prepare data */
    const int size = 1600 + 8 + 7; /* odd len to test leftover */
    int8_t *input1;
    int8_t *input2;
    int8_t *out_data_c;
    int8_t *out_data_opt;
    int8_t *input1_orig = NULL;
    int8_t *input2_orig = NULL;
    int8_t *out_c_orig = NULL;
    int8_t *out_opt_orig = NULL;
    int32_t input1_offset = 34;
    int32_t input2_offset = 35;
    int32_t output_offset = 36;
    int32_t input1_shift = -8; // right_shift amt always <= 0
    int32_t input2_shift = -8; // right_shift amt always <= 0
    int32_t output_shift = -9; // right_shift amt always <= 0
    int32_t left_shift = 15; // always +ve
    int32_t input1_mult = INT32_MAX;
    int32_t input2_mult = INT32_MAX;
    int32_t output_mult = INT32_MAX;
    int32_t activation_min = -128;
    int32_t activation_max = 127;

    for (int itr = 0; itr < 10; itr++) {
        switch (itr) {
        case 0: // all zeros
            input1_offset = 0;
            input2_offset = 0;
            output_offset = 0;
            input1_mult = 0;
            input2_mult = 0;
            output_mult = 0;
            input1_shift = 0;
            input2_shift = 0;
            output_shift = 0;
            left_shift = 0;
        break;
        case 1: // hit min
            input1_offset = -127;
            input2_offset = -127;
            output_offset = -128;
            input1_mult = MULT_MIN;
            input2_mult = MULT_MIN;
            output_mult = MULT_MIN;
            input1_shift = 0;
            input2_shift = 0;
            output_shift = 0;
            left_shift = 0;
        break;
        case 2: // hit max
            input1_offset = 128;
            input2_offset = 128;
            output_offset = -127;
            input1_mult = MULT_MAX;
            input2_mult = MULT_MAX;
            output_mult = MULT_MAX;
            input1_shift = SHIFT_MIN;
            input2_shift = SHIFT_MIN;
            output_shift = SHIFT_MIN;
            left_shift = 30 - 8; // since input is 8 bits
        break;
        case 3: // hit extreme max
            input1_offset = 128;
            input2_offset = 128;
            output_offset = -127;
            input1_mult = MULT_MAX;
            input2_mult = MULT_MAX;
            output_mult = MULT_MAX;
            input1_shift = 0;
            input2_shift = 0;
            output_shift = 0;
            left_shift = 30 - 8; // -8 since input is 8 bit
        break;
        default:  // practical random input
            input1_offset = rand() % 256 - 127; // range [-127, 128]
            input2_offset = rand() % 256 - 127; // range [-127, 128]
            output_offset = rand() % 256 - 128; // range [-128, 127]
            input1_mult = MULT_MAX / 2 + rand() % INT16_MAX;
            input2_mult = MULT_MAX / 2 + rand() % INT16_MAX;
            output_mult = MULT_MAX / 2 + rand() % INT16_MAX;
            input1_shift = -8 + rand() % 4;
            input2_shift = -8 + rand() % 4;
            output_shift = -8 + rand() % 4;
            left_shift = rand() % 15;
        }
#if IDF_HEAP_CAPS
        input1_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        input2_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        out_c_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        out_opt_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        input1 = 16 + input1_orig - ((uint32_t) input1_orig & 0xf);
        input2 = 16 + input2_orig - ((uint32_t) input2_orig & 0xf);
        out_data_c = 16 + out_c_orig - ((uint32_t) out_c_orig & 0xf);
        out_data_opt = 16 + out_opt_orig - ((uint32_t) out_opt_orig & 0xf);
#else
        input1 = memalign(16, size);
        input2 = memalign(16, size);
        out_data_c = memalign(16, size);
        out_data_opt = memalign(16, size);

        input1_orig = input1;
        input2_orig = input2;
        out_c_orig = out_data_c;
        out_opt_orig = out_data_opt;
#endif
        if (input1_orig == NULL || input2_orig == NULL || out_c_orig == NULL ||
                out_opt_orig == NULL) {
            printf(ANSI_COLOR_RED"%s error allocating buffers\n"ANSI_COLOR_RESET, __FUNCTION__);
            goto elementwise_add_test_cleanup;
        }

        for (int i = 0; i < size; ++i) {
            input1[i] = rand() % 256 - 128;
            input2[i] = rand() % 256 - 128;
        }

        if (itr == 0) {
            /* enable profiler */
            profile_c_start();
        }
        /* C function */
        esp_nn_add_elementwise_s8_ansi(input1, input2, input1_offset, input2_offset,
                                       input1_mult, input2_mult, input1_shift, input2_shift,
                                       left_shift, out_data_c, output_offset, output_mult,
                                       output_shift, activation_min, activation_max, size);

        if (itr == 0) {
            profile_c_end();
            profile_opt_start();
        }

        /* Optimized function */
        esp_nn_add_elementwise_s8(input1, input2, input1_offset, input2_offset,
                                  input1_mult, input2_mult, input1_shift, input2_shift,
                                  left_shift, out_data_opt, output_offset, output_mult,
                                  output_shift, activation_min, activation_max, size);
        if (itr == 0) {
            /* disable profiler */
            profile_opt_end();
        }

        bool ret = CHECK_EQUAL(out_data_c, out_data_opt, size);
        if (ret == false) {
            printf(ANSI_COLOR_RED"%s[%d] failed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);
            printf("Output: \n");
            PRINT_ARRAY_HEX(out_data_opt, size, 1);
            printf("Expected: \n");
            PRINT_ARRAY_HEX(out_data_c, size, 1);
            printf("Input1:\n");
            PRINT_ARRAY_HEX(input1, size, 1);
            printf("Input2:\n");
            PRINT_ARRAY_HEX(input2, size, 1);
            printf("in1_shift %d, in2_shift %d, left_shift %d, out_shift %d\n",
                   input1_shift, input2_shift, left_shift, output_shift);
            printf("in1_mult %d, in2_mult %d, out_mult %d\n", input1_mult, input2_mult, output_mult);
            goto elementwise_add_test_cleanup;
        }
        printf(ANSI_COLOR_GREEN"%s[%d] passed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);

elementwise_add_test_cleanup:
        if (input1_orig) {
            free(input1_orig);
        }
        if (input2_orig) {
            free(input2_orig);
        }
        if (out_c_orig) {
            free(out_c_orig);
        }
        if (out_opt_orig) {
            free(out_opt_orig);
        }
    }
}

void esp_nn_mul_elementwise_s8_test()
{
    /* prepare data */
    const int size = 1600 + 8 + 7; /* odd len to test leftover */
    int8_t *input1;
    int8_t *input2;
    int8_t *out_data_c;
    int8_t *out_data_opt;
    int32_t input1_offset = 34;
    int32_t input2_offset = 35;
    int32_t output_offset = 36;
    int32_t output_shift = -7;
    int32_t output_mult = MULT_MAX; // max out_mult
    int32_t activation_min = -128;
    int32_t activation_max = 127;
    int8_t *input1_orig = NULL;
    int8_t *input2_orig = NULL;
    int8_t *out_c_orig = NULL;
    int8_t *out_opt_orig = NULL;

    for (int itr = 0; itr < 10; itr++) {
        switch (itr) {
        case 0: // all zeros
            input1_offset = 0;
            input2_offset = 0;
            output_offset = 0;
            output_mult = 0;
            output_shift = 0;
        break;
        case 1: // hit min
            input1_offset = -127;
            input2_offset = -127;
            output_offset = -128;
            output_mult = MULT_MIN;
            output_shift = 0;
        break;
        case 2: // hit max
            input1_offset = 128;
            input2_offset = 128;
            output_offset = -127;
            output_mult = MULT_MAX;
            output_shift = SHIFT_MIN;
        break;
        case 3: // hit extreme max
            input1_offset = 128;
            input2_offset = 128;
            output_offset = -127;
            output_mult = MULT_MAX;
            output_shift = 0;
        break;
        default:  // practical random input
            input1_offset = rand() % 256 - 127; // range [-127, 128]
            input2_offset = rand() % 256 - 127; // range [-127, 128]
            output_offset = rand() % 256 - 128; // range [-128, 127]
            output_mult = MULT_MAX / 2 + rand() % INT16_MAX;
            output_shift = -8 + rand() % 4;
        }

#if IDF_HEAP_CAPS
        input1_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        input2_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        out_c_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        out_opt_orig = (int8_t *) heap_caps_malloc(size + 32, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        input1 = 16 + input1_orig - ((uint32_t) input1_orig & 0xf);
        input2 = 16 + input2_orig - ((uint32_t) input2_orig & 0xf);
        out_data_c = 16 + out_c_orig - ((uint32_t) out_c_orig & 0xf);
        out_data_opt = 16 + out_opt_orig - ((uint32_t) out_opt_orig & 0xf);
#else
        input1 = memalign(16, size);
        input2 = memalign(16, size);
        out_data_c = memalign(16, size);
        out_data_opt = memalign(16, size);

        input1_orig = input1;
        input2_orig = input2;
        out_c_orig = out_data_c;
        out_opt_orig = out_data_opt;
#endif
        if (input1_orig == NULL || input2_orig == NULL || out_c_orig == NULL ||
                out_opt_orig == NULL) {
            printf(ANSI_COLOR_RED"%s error allocating buffers\n"ANSI_COLOR_RESET, __FUNCTION__);
            goto elementwise_mult_test_cleanup;
        }

        for (int i = 0; i < size; ++i) {
            input1[i] = rand() % 256 - 128;
            input2[i] = rand() % 256 - 128;
        }

        if (itr == 0) {
            /* enable profiler */
            profile_c_start();
        }
        /* C function */
        esp_nn_mul_elementwise_s8_ansi(input1, input2, input1_offset, input2_offset,
                                       out_data_c, output_offset, output_mult, output_shift,
                                       activation_min, activation_max, size);

        if (itr == 0) {
            profile_c_end();
            profile_opt_start();
        }
        /* Optimized function */
        esp_nn_mul_elementwise_s8(input1, input2, input1_offset, input2_offset,
                                  out_data_opt, output_offset, output_mult, output_shift,
                                  activation_min, activation_max, size);

        if (itr == 0) {
            /* disable profiler */
            profile_opt_end();
        }

        bool ret = CHECK_EQUAL(out_data_c, out_data_opt, size);
        if (ret == false) {
            printf(ANSI_COLOR_RED"%s[%d] failed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);
            printf("Output: \n");
            PRINT_ARRAY_HEX(out_data_opt, size, 1);
            printf("Expected: \n");
            PRINT_ARRAY_HEX(out_data_c, size, 1);
            printf("Input1:\n");
            PRINT_ARRAY_HEX(input1, size, 1);
            printf("Input2:\n");
            PRINT_ARRAY_HEX(input2, size, 1);
            goto elementwise_mult_test_cleanup;
        }
        printf(ANSI_COLOR_GREEN"%s[%d] passed\n"ANSI_COLOR_RESET, __FUNCTION__, itr);

elementwise_mult_test_cleanup:
        if (input1_orig) {
            free(input1_orig);
        }
        if (input2_orig) {
            free(input2_orig);
        }
        if (out_c_orig) {
            free(out_c_orig);
        }
        if (out_opt_orig) {
            free(out_opt_orig);
        }
    }
}
