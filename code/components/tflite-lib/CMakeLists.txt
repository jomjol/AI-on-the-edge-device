## TODO: GLOB is not a good way to collect files. Use explicit file list instead

cmake_minimum_required(VERSION 3.5)

set(tflite_dir "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow/lite")
set(tfmicro_dir "${tflite_dir}/micro")
set(tfmicro_frontend_dir "${tflite_dir}/experimental/microfrontend/lib")
set(tfmicro_kernels_dir "${tfmicro_dir}/kernels")

file(GLOB srcs_micro
          "${tfmicro_dir}/*.cc"
          "${tfmicro_dir}/*.c")

file(GLOB src_micro_frontend
          "${tfmicro_frontend_dir}/*.c"
          "${tfmicro_frontend_dir}/*.cc")
file(GLOB srcs_kernels
          "${tfmicro_kernels_dir}/*.c"
          "${tfmicro_kernels_dir}/*.cc")

# remove sources which will be provided by esp_nn
list(REMOVE_ITEM srcs_kernels
          "${tfmicro_kernels_dir}/add.cc"
          "${tfmicro_kernels_dir}/conv.cc"
          "${tfmicro_kernels_dir}/depthwise_conv.cc"
          "${tfmicro_kernels_dir}/fully_connected.cc"
          "${tfmicro_kernels_dir}/mul.cc"
          "${tfmicro_kernels_dir}/pooling.cc"
          "${tfmicro_kernels_dir}/softmax.cc")

FILE(GLOB esp_nn_kernels
          "${tfmicro_kernels_dir}/esp_nn/*.cc")

set(lib_srcs
          "${srcs_micro}"
          "${srcs_kernels}"
          "${esp_nn_kernels}"
          "${src_micro_frontend}"
          "${tflite_dir}/kernels/kernel_util.cc"
          "${tflite_dir}/micro/memory_planner/greedy_memory_planner.cc"
          "${tflite_dir}/micro/memory_planner/linear_memory_planner.cc"
          "${tflite_dir}/micro/arena_allocator/recording_simple_memory_allocator.cc"
          "${tflite_dir}/micro/arena_allocator/simple_memory_allocator.cc"
          "${tflite_dir}/c/common.cc"
          "${tflite_dir}/core/api/error_reporter.cc"
          "${tflite_dir}/core/api/flatbuffer_conversions.cc"
          "${tflite_dir}/core/api/op_resolver.cc"
          "${tflite_dir}/core/api/tensor_utils.cc"
          "${tflite_dir}/kernels/internal/quantization_util.cc"
          "${tflite_dir}/schema/schema_utils.cc")

idf_component_register(
            SRCS "${lib_srcs}"
            INCLUDE_DIRS "." "third_party/gemmlowp"
                         "third_party/flatbuffers/include"
                         "third_party/ruy"
                         "third_party/kissfft"
            REQUIRES "esp-nn")

# Reduce the level of paranoia to be able to compile TF sources
target_compile_options(${COMPONENT_LIB} PRIVATE
  -Wno-maybe-uninitialized
  -Wno-missing-field-initializers
  -DESP_NN # enables ESP-NN optimizations by Espressif
  -Wno-type-limits)

target_compile_options(${COMPONENT_LIB} PRIVATE -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -O3 -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -Wno-nonnull)
target_compile_options(${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>: -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -O3 -Werror -Wsign-compare -Wdouble-promotion -Wshadow -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wvla -Wall -Wextra -Wstrict-aliasing -Wno-unused-parameter -Wno-return-type -Wno-strict-aliasing -std=gnu++14 >)
target_compile_options(${COMPONENT_LIB} INTERFACE $<$<IN_LIST:-DTF_LITE_STATIC_MEMORY,$<TARGET_PROPERTY:${COMPONENT_LIB},COMPILE_OPTIONS>>:-DTF_LITE_STATIC_MEMORY>)
target_link_libraries(${COMPONENT_LIB} PRIVATE -lm)
