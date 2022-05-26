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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <test_functions.h>
#include <esp_timer.h>

static const char *TAG = "test_app";
static uint32_t start_c, start_opt, total_c, total_opt;

void profile_c_start()
{
    /* initiate profiling */
    start_c = esp_cpu_get_ccount();
}

void profile_c_end()
{
    /* record profile number */
    total_c = esp_cpu_get_ccount() - start_c;
}

void profile_opt_start()
{
    /* initiate profiling */
    start_opt = esp_cpu_get_ccount();
}

void profile_opt_end()
{
    /* record profile number */
    total_opt = esp_cpu_get_ccount() - start_opt;
}

void app_main()
{
    /* s8 tests */
    ESP_LOGI(TAG, "Running s8 tests...");
    esp_nn_add_elementwise_s8_test();
    printf("add, c %u opt %u\n", total_c, total_opt);
    esp_nn_mul_elementwise_s8_test();
    printf("mul, c %u opt %u\n", total_c, total_opt);
    esp_nn_depthwise_conv_s8_test();
    printf("depthwise, c %u opt %u\n", total_c, total_opt);
    esp_nn_conv_s8_test();
    printf("conv2d, c %u opt %u\n", total_c, total_opt);

    esp_nn_relu6_s8_test();
    printf("relu, c %u opt %u\n", total_c, total_opt);
    esp_nn_avg_pool_s8_test();
    printf("avg_pool, c %u opt %u\n", total_c, total_opt);
    esp_nn_max_pool_s8_test();
    printf("max_pool, c %u opt %u\n", total_c, total_opt);
    esp_nn_fully_connected_s8_test();
    printf("fully_connected, c %u opt %u\n", total_c, total_opt);
    esp_nn_softmax_s8_test();
    printf("softmax, c %u opt %u\n", total_c, total_opt);
    ESP_LOGI(TAG, "s8 tests done!\n");

    /* u8 tests */
    //ESP_LOGI(TAG, "Running u8 tests...");
    //esp_nn_add_elementwise_u8_test();
    //esp_nn_depthwise_conv_u8_test();
    //esp_nn_conv_u8_test();
    //esp_nn_avg_pool_u8_test();
    //esp_nn_max_pool_u8_test();
    //esp_nn_fully_connected_u8_test();
    //ESP_LOGI(TAG, "u8 tests done!\n");
}
