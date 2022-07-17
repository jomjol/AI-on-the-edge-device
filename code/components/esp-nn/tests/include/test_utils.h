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
#include <common_functions.h>
#include <stdio.h>

/* mult value range */
#define MULT_MAX    INT32_MAX
#define MULT_MIN    0

/* shift value range */
#define SHIFT_MIN   -31
#define SHIFT_MAX   30

/**
 * @brief callback function to run before C function
 */
void profile_c_start();

/**
 * @brief callback function to run after C function
 */
void profile_c_end();

/**
 * @brief callback function to run before optimized function
 */
void profile_opt_start();

/**
 * @brief callback function to run after optimized function
 */
void profile_opt_end();

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define CHECK_EQUAL(ARRAY1, ARRAY2, size) ({    \
    bool res = true;                            \
    for (int _i = 0; _i < size; _i++) {         \
        if (ARRAY1[_i] != ARRAY2[_i]) {         \
            res = false;                        \
            break;                              \
        }                                       \
    }                                           \
    res;                                        \
})

#define PRINT_ARRAY_INT(ARRAY, width, height) ({        \
    int *_array = (int *) ARRAY;                        \
    for (int _j = 0; _j < height; _j++) {               \
        for (int _i = 0; _i < width; _i++) {            \
            printf("%d\t", _array[width * _j + _i]);    \
        }                                               \
        printf("\n");                                   \
    }                                                   \
    printf("\n");                                       \
})

#define PRINT_ARRAY_HEX(ARRAY, width, height) ({        \
    uint8_t *_array = (uint8_t *) ARRAY;                \
    for (int _j = 0; _j < height; _j++) {               \
        for (int _i = 0; _i < width; _i++) {            \
            printf("%02x\t", _array[width * _j + _i]);  \
        }                                               \
        printf("\n");                                   \
    }                                                   \
    printf("\n");                                       \
})
