
# Copyright 2019 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This component was generated for the 'hello_world' TF Micro example.
#

# Make sure that the IDF Path environment variable is defined
if(NOT DEFINED ENV{IDF_PATH})
  message(FATAL_ERROR "The IDF_PATH environment variable must point to the location of the ESP-IDF.")
endif()

idf_component_register(
  SRCS tensorflow/lite/micro/simple_memory_allocator.cc tensorflow/lite/micro/micro_error_reporter.cc tensorflow/lite/micro/memory_helpers.cc tensorflow/lite/micro/test_helpers.cc tensorflow/lite/micro/micro_utils.cc tensorflow/lite/micro/micro_time.cc tensorflow/lite/micro/debug_log.cc tensorflow/lite/micro/micro_string.cc tensorflow/lite/micro/micro_optional_debug_tools.cc tensorflow/lite/micro/micro_interpreter.cc tensorflow/lite/micro/micro_allocator.cc tensorflow/lite/micro/kernels/comparisons.cc tensorflow/lite/micro/kernels/fully_connected.cc tensorflow/lite/micro/kernels/depthwise_conv.cc tensorflow/lite/micro/kernels/logistic.cc tensorflow/lite/micro/kernels/pooling.cc tensorflow/lite/micro/kernels/prelu.cc tensorflow/lite/micro/kernels/concatenation.cc tensorflow/lite/micro/kernels/dequantize.cc tensorflow/lite/micro/kernels/pad.cc tensorflow/lite/micro/kernels/l2norm.cc tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc tensorflow/lite/micro/kernels/activations.cc tensorflow/lite/micro/kernels/ceil.cc tensorflow/lite/micro/kernels/arg_min_max.cc tensorflow/lite/micro/kernels/reduce.cc tensorflow/lite/micro/kernels/unpack.cc tensorflow/lite/micro/kernels/add.cc tensorflow/lite/micro/kernels/split.cc tensorflow/lite/micro/kernels/circular_buffer.cc tensorflow/lite/micro/kernels/softmax.cc tensorflow/lite/micro/kernels/floor.cc tensorflow/lite/micro/kernels/sub.cc tensorflow/lite/micro/kernels/mul.cc tensorflow/lite/micro/kernels/conv.cc tensorflow/lite/micro/kernels/neg.cc tensorflow/lite/micro/kernels/quantize.cc tensorflow/lite/micro/kernels/elementwise.cc tensorflow/lite/micro/kernels/all_ops_resolver.cc tensorflow/lite/micro/kernels/svdf.cc tensorflow/lite/micro/kernels/maximum_minimum.cc tensorflow/lite/micro/kernels/reshape.cc tensorflow/lite/micro/kernels/strided_slice.cc tensorflow/lite/micro/kernels/round.cc tensorflow/lite/micro/kernels/pack.cc tensorflow/lite/micro/kernels/logical.cc tensorflow/lite/micro/memory_planner/linear_memory_planner.cc tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc tensorflow/lite/c/common.c tensorflow/lite/core/api/error_reporter.cc tensorflow/lite/core/api/flatbuffer_conversions.cc tensorflow/lite/core/api/op_resolver.cc tensorflow/lite/core/api/tensor_utils.cc tensorflow/lite/kernels/internal/quantization_util.cc tensorflow/lite/kernels/kernel_util.cc tensorflow/lite/micro/testing/test_utils.cc 
  INCLUDE_DIRS . third_party/gemmlowp third_party/flatbuffers/include third_party/ruy third_party/kissfft)

# Reduce the level of paranoia to be able to compile TF sources
target_compile_options(${COMPONENT_LIB} PRIVATE
  -Wno-maybe-uninitialized
  -Wno-missing-field-initializers
  -Wno-type-limits)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=c11   -DTF_LITE_STATIC_MEMORY -O3 -Wno-nonnull -Wno-nonnull -Wno-nonnull -Wno-nonnull)
target_compile_options(${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>: -std=c++11 -DTF_LITE_STATIC_MEMORY -O3 -Wno-return-type -Wno-strict-aliasing -Wno-ignored-qualifiers -Wno-return-type -Wno-strict-aliasing -Wno-ignored-qualifiers -Wno-return-type -Wno-strict-aliasing -Wno-return-type -Wno-strict-aliasing >)
target_link_libraries(${COMPONENT_LIB} PRIVATE -lm)
