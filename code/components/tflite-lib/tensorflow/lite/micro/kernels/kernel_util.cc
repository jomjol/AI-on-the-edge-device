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

#include "tensorflow/lite/micro/kernels/kernel_util.h"

#include "tensorflow/lite/c/common.h"

namespace tflite {
namespace micro {

namespace {

int ValidateTensorIndexing(const TfLiteContext* context, int index,
                           int max_size, const int* tensor_indices) {
  if (index >= 0 && index < max_size) {
    const int tensor_index = tensor_indices[index];
    if (tensor_index != kTfLiteOptionalTensor) {
      return tensor_index;
    }
  }
  return -1;
}

}  // namespace

// Returns a mutable tensor for a given input index. is_variable must be checked
// during prepare when the full TfLiteTensor is available.
TfLiteEvalTensor* GetMutableEvalInput(const TfLiteContext* context,
                                      const TfLiteNode* node, int index) {
  TFLITE_DCHECK(context != nullptr);
  TFLITE_DCHECK(node != nullptr);
  const int tensor_index = ValidateTensorIndexing(
      context, index, node->inputs->size, node->inputs->data);

  if (tensor_index < 0) {
    return nullptr;
  }

  return context->GetEvalTensor(context, node->inputs->data[index]);
}

// Returns the TfLiteEvalTensor struct for a given input index in a node.
const TfLiteEvalTensor* GetEvalInput(const TfLiteContext* context,
                                     const TfLiteNode* node, int index) {
  return GetMutableEvalInput(context, node, index);
}

// Returns the TfLiteEvalTensor struct for a given output index in a node.
TfLiteEvalTensor* GetEvalOutput(const TfLiteContext* context,
                                const TfLiteNode* node, int index) {
  TFLITE_DCHECK(context != nullptr);
  TFLITE_DCHECK(node != nullptr);
  return context->GetEvalTensor(context, node->outputs->data[index]);
}

bool HaveSameShapes(const TfLiteEvalTensor* input1,
                    const TfLiteEvalTensor* input2) {
  TFLITE_DCHECK(input1 != nullptr);
  TFLITE_DCHECK(input2 != nullptr);
  return TfLiteIntArrayEqual(input1->dims, input2->dims);
}

const RuntimeShape GetTensorShape(const TfLiteEvalTensor* tensor) {
  if (tensor == nullptr || tensor->dims == nullptr) {
    return RuntimeShape();
  }
  TfLiteIntArray* dims = tensor->dims;
  const int dims_size = dims->size;
  const int32_t* dims_data = reinterpret_cast<const int32_t*>(dims->data);
  return RuntimeShape(dims_size, dims_data);
}

PaddingType RuntimePaddingType(TfLitePadding padding) {
  switch (padding) {
    case TfLitePadding::kTfLitePaddingSame:
      return PaddingType::kSame;
    case TfLitePadding::kTfLitePaddingValid:
      return PaddingType::kValid;
    case TfLitePadding::kTfLitePaddingUnknown:
    default:
      return PaddingType::kNone;
  }
}

// Relocate tensor dims from FlatBuffer to the persistent storage arena.
// The old dims data is copied to the new storage area.
// The tensor and eval_tensor must be the same tensor.
// Only use during Prepare phase.
TfLiteStatus CreateWritableTensorDimsWithCopy(TfLiteContext* context,
                                              TfLiteTensor* tensor,
                                              TfLiteEvalTensor* eval_tensor) {
  TF_LITE_ENSURE(context, tensor != nullptr);
  TF_LITE_ENSURE(context, eval_tensor != nullptr);
  TF_LITE_ENSURE(context, context->AllocatePersistentBuffer != nullptr);
  int ranks = tensor->dims->size;
  size_t alloc_size = TfLiteIntArrayGetSizeInBytes(ranks);
  TfLiteIntArray* new_dims = static_cast<TfLiteIntArray*>(
      context->AllocatePersistentBuffer(context, alloc_size));
  TfLiteIntArray* old_dims = tensor->dims;
  new_dims->size = ranks;
  tensor->dims = new_dims;
  eval_tensor->dims = new_dims;
  for (int i = 0; i < ranks; i++) {
    new_dims->data[i] = old_dims->data[i];
  }

  return kTfLiteOk;
}

// Returns a blob of payload data. The payload is subjected to interpretation by
// the OP. This is the recommended API for an OP to get an external context. OP
// should use this instead of directly calling GetExternalContext function in
// context.
void* GetExternalContext(TfLiteContext* context) {
  return reinterpret_cast<void*>(
      context->GetExternalContext(context, kTfLiteMaxExternalContexts));
}

}  // namespace micro
}  // namespace tflite
