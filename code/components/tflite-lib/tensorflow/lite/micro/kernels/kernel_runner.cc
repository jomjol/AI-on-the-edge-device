/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/micro/kernels/kernel_runner.h"

#include "tensorflow/lite/micro/arena_allocator/simple_memory_allocator.h"
#include "tensorflow/lite/micro/micro_arena_constants.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/test_helpers.h"

namespace tflite {
namespace micro {

// TODO(b/161841696): Consider moving away from global arena buffers:
constexpr int KernelRunner::kKernelRunnerBufferSize_;
uint8_t KernelRunner::kKernelRunnerBuffer_[];

KernelRunner::KernelRunner(const TfLiteRegistration& registration,
                           TfLiteTensor* tensors, int tensors_size,
                           TfLiteIntArray* inputs, TfLiteIntArray* outputs,
                           void* builtin_data, TfLiteIntArray* intermediates)
    : registration_(registration),
      allocator_(SimpleMemoryAllocator::Create(GetMicroErrorReporter(),
                                               kKernelRunnerBuffer_,
                                               kKernelRunnerBufferSize_)),
      mock_micro_graph_(allocator_),
      fake_micro_context_(tensors, allocator_, &mock_micro_graph_) {
  // Prepare TfLiteContext:
  context_.impl_ = static_cast<void*>(&fake_micro_context_);
  context_.ReportError = MicroContextReportOpError;
  context_.recommended_num_threads = 1;
  context_.GetTensor = MicroContextGetTensor;
  context_.GetEvalTensor = MicroContextGetEvalTensor;
  context_.AllocatePersistentBuffer = MicroContextAllocatePersistentBuffer;
  context_.RequestScratchBufferInArena =
      MicroContextRequestScratchBufferInArena;
  context_.GetScratchBuffer = MicroContextGetScratchBuffer;

  context_.recommended_num_threads = 0;

  // Prepare TfLiteNode:
  node_.inputs = inputs;
  node_.outputs = outputs;
  node_.builtin_data = builtin_data;
  node_.intermediates = intermediates;
}

bool KernelRunner::ValidateTempBufferDeallocated() {
  return fake_micro_context_.IsAllTempTfLiteTensorDeallocated();
}

TfLiteStatus KernelRunner::InitAndPrepare(const char* init_data,
                                          size_t length) {
  if (registration_.init) {
    node_.user_data = registration_.init(&context_, init_data, length);
  }

  TF_LITE_ENSURE(&context_, ValidateTempBufferDeallocated());

  if (registration_.prepare) {
    TF_LITE_ENSURE_STATUS(registration_.prepare(&context_, &node_));
  }

  TF_LITE_ENSURE(&context_, ValidateTempBufferDeallocated());

  return kTfLiteOk;
}

TfLiteStatus KernelRunner::Invoke() {
  if (registration_.invoke == nullptr) {
    MicroPrintf("TfLiteRegistration missing invoke function pointer!");
    return kTfLiteError;
  }

  TF_LITE_ENSURE_STATUS(registration_.invoke(&context_, &node_));

  TF_LITE_ENSURE(&context_, ValidateTempBufferDeallocated());

  return kTfLiteOk;
}

}  // namespace micro
}  // namespace tflite
