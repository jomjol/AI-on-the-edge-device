
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
  SRCS tensorflow/lite/micro/simple_memory_allocator.cc tensorflow/lite/micro/debug_log.cc tensorflow/lite/micro/micro_error_reporter.cc tensorflow/lite/micro/memory_helpers.cc tensorflow/lite/micro/test_helpers.cc tensorflow/lite/micro/recording_micro_allocator.cc tensorflow/lite/micro/micro_time.cc tensorflow/lite/micro/recording_simple_memory_allocator.cc tensorflow/lite/micro/micro_string.cc tensorflow/lite/micro/micro_profiler.cc tensorflow/lite/micro/flatbuffer_utils.cc tensorflow/lite/micro/micro_graph.cc tensorflow/lite/micro/mock_micro_graph.cc tensorflow/lite/micro/all_ops_resolver.cc tensorflow/lite/micro/micro_utils.cc tensorflow/lite/micro/micro_interpreter.cc tensorflow/lite/micro/micro_allocator.cc tensorflow/lite/micro/system_setup.cc tensorflow/lite/micro/memory_planner/linear_memory_planner.cc tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc tensorflow/lite/schema/schema_utils.cc tensorflow/lite/c/common.c tensorflow/lite/core/api/tensor_utils.cc tensorflow/lite/core/api/error_reporter.cc tensorflow/lite/core/api/flatbuffer_conversions.cc tensorflow/lite/core/api/op_resolver.cc tensorflow/lite/kernels/kernel_util.cc tensorflow/lite/kernels/internal/quantization_util.cc tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc  tensorflow/lite/micro/kernels/activations.cc tensorflow/lite/micro/kernels/activations_common.cc tensorflow/lite/micro/kernels/add.cc tensorflow/lite/micro/kernels/add_n.cc tensorflow/lite/micro/kernels/arg_min_max.cc tensorflow/lite/micro/kernels/batch_to_space_nd.cc tensorflow/lite/micro/kernels/cast.cc tensorflow/lite/micro/kernels/ceil.cc tensorflow/lite/micro/kernels/circular_buffer.cc tensorflow/lite/micro/kernels/comparisons.cc tensorflow/lite/micro/kernels/concatenation.cc tensorflow/lite/micro/kernels/conv.cc tensorflow/lite/micro/kernels/conv_common.cc tensorflow/lite/micro/kernels/cumsum.cc tensorflow/lite/micro/kernels/depth_to_space.cc tensorflow/lite/micro/kernels/depthwise_conv.cc tensorflow/lite/micro/kernels/depthwise_conv_common.cc tensorflow/lite/micro/kernels/dequantize.cc tensorflow/lite/micro/kernels/detection_postprocess.cc tensorflow/lite/micro/kernels/elementwise.cc tensorflow/lite/micro/kernels/elu.cc tensorflow/lite/micro/kernels/ethosu.cc tensorflow/lite/micro/kernels/exp.cc tensorflow/lite/micro/kernels/expand_dims.cc tensorflow/lite/micro/kernels/fill.cc tensorflow/lite/micro/kernels/floor.cc tensorflow/lite/micro/kernels/floor_div.cc tensorflow/lite/micro/kernels/floor_mod.cc tensorflow/lite/micro/kernels/fully_connected.cc tensorflow/lite/micro/kernels/fully_connected_common.cc tensorflow/lite/micro/kernels/gather.cc tensorflow/lite/micro/kernels/gather_nd.cc tensorflow/lite/micro/kernels/hard_swish.cc tensorflow/lite/micro/kernels/hard_swish_common.cc tensorflow/lite/micro/kernels/if.cc tensorflow/lite/micro/kernels/kernel_runner.cc tensorflow/lite/micro/kernels/kernel_util.cc tensorflow/lite/micro/kernels/l2norm.cc tensorflow/lite/micro/kernels/l2_pool_2d.cc tensorflow/lite/micro/kernels/leaky_relu.cc tensorflow/lite/micro/kernels/logical.cc tensorflow/lite/micro/kernels/logical_common.cc tensorflow/lite/micro/kernels/logistic.cc tensorflow/lite/micro/kernels/logistic_common.cc tensorflow/lite/micro/kernels/log_softmax.cc tensorflow/lite/micro/kernels/maximum_minimum.cc tensorflow/lite/micro/kernels/mul.cc tensorflow/lite/micro/kernels/neg.cc tensorflow/lite/micro/kernels/pack.cc tensorflow/lite/micro/kernels/pad.cc tensorflow/lite/micro/kernels/pooling.cc tensorflow/lite/micro/kernels/pooling_common.cc tensorflow/lite/micro/kernels/prelu.cc tensorflow/lite/micro/kernels/quantize.cc tensorflow/lite/micro/kernels/quantize_common.cc tensorflow/lite/micro/kernels/reduce.cc tensorflow/lite/micro/kernels/reshape.cc tensorflow/lite/micro/kernels/resize_bilinear.cc tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc tensorflow/lite/micro/kernels/round.cc tensorflow/lite/micro/kernels/shape.cc tensorflow/lite/micro/kernels/softmax.cc tensorflow/lite/micro/kernels/softmax_common.cc tensorflow/lite/micro/kernels/space_to_batch_nd.cc tensorflow/lite/micro/kernels/space_to_depth.cc tensorflow/lite/micro/kernels/split.cc tensorflow/lite/micro/kernels/split_v.cc tensorflow/lite/micro/kernels/squeeze.cc tensorflow/lite/micro/kernels/strided_slice.cc tensorflow/lite/micro/kernels/sub.cc tensorflow/lite/micro/kernels/svdf.cc tensorflow/lite/micro/kernels/svdf_common.cc tensorflow/lite/micro/kernels/tanh.cc tensorflow/lite/micro/kernels/transpose.cc tensorflow/lite/micro/kernels/transpose_conv.cc tensorflow/lite/micro/kernels/unpack.cc tensorflow/lite/micro/kernels/zeros_like.cc
  INCLUDE_DIRS . third_party/gemmlowp third_party/flatbuffers/include third_party/ruy)

# Reduce the level of paranoia to be able to compile TF sources
target_compile_options(${COMPONENT_LIB} PRIVATE
  -Wno-maybe-uninitialized
  -Wno-missing-field-initializers
  -Wno-type-limits)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wimplicit-function-declaration -Werror -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -DESP)
target_compile_options(${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>: -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -Werror -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -DESP >)
target_compile_options(${COMPONENT_LIB} INTERFACE $<$<IN_LIST:-DTF_LITE_STATIC_MEMORY,$<TARGET_PROPERTY:${COMPONENT_LIB},COMPILE_OPTIONS>>:-DTF_LITE_STATIC_MEMORY>)
target_link_libraries(${COMPONENT_LIB} PRIVATE -lm)
