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


/* int8_t ops tests */
void esp_nn_add_elementwise_s8_test();
void esp_nn_mul_elementwise_s8_test();

void esp_nn_depthwise_conv_s8_test();
void esp_nn_conv_s8_test();

void esp_nn_avg_pool_s8_test();
void esp_nn_max_pool_s8_test();

void esp_nn_fully_connected_s8_test();

void esp_nn_relu6_s8_test();

void esp_nn_softmax_s8_test();

/* uint8_t ops tests */
void esp_nn_add_elementwise_u8_test();

void esp_nn_depthwise_conv_u8_test();
void esp_nn_conv_u8_test();

void esp_nn_avg_pool_u8_test();
void esp_nn_max_pool_u8_test();

void esp_nn_fully_connected_u8_test();

/* instructions test functions */
void compare_instructions_test();
void arith_instructions_test();
void min_max_instructions_test();
void bitwise_instructions_test();
void load_store_instructions_test();
