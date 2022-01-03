/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

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

#include "tensorflow/lite/kernels/internal/reference/slice.h"

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

namespace tflite {

namespace {

constexpr int kInputTensor = 0;
constexpr int kBeginTensor = 1;
constexpr int kSizeTensor = 2;
constexpr int kOutputTensor = 0;

const int kMaxDim = 5;

template <typename T>
void GetBeginAndSizeVectors(int dimensions, const TfLiteEvalTensor* begin,
                            const TfLiteEvalTensor* size, int32_t* begins,
                            int32_t* sizes) {
  int offset = kMaxDim - dimensions;
  for (int idx = 0; idx < dimensions; ++idx) {
    begins[offset + idx] = tflite::micro::GetTensorData<T>(begin)[idx];
    sizes[offset + idx] = tflite::micro::GetTensorData<T>(size)[idx];
  }
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 3);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TFLITE_DCHECK(input != nullptr);
  const TfLiteTensor* begin = GetInput(context, node, kBeginTensor);
  TFLITE_DCHECK(begin != nullptr);
  const TfLiteTensor* size = GetInput(context, node, kSizeTensor);
  TFLITE_DCHECK(size != nullptr);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  TFLITE_DCHECK(output != nullptr);

  // Ensure validity of input tensor and its dimension.
  TFLITE_DCHECK(input->type == output->type);
  TFLITE_DCHECK(begin->type == size->type);
  TFLITE_DCHECK(begin->type == kTfLiteInt32 || begin->type == kTfLiteInt64);
  TFLITE_DCHECK(size->type == kTfLiteInt32 || size->type == kTfLiteInt64);
  TFLITE_DCHECK(NumDimensions(begin) == 1);
  TFLITE_DCHECK(NumDimensions(size) == 1);
  TFLITE_DCHECK(NumElements(begin) == NumElements(size));
  TFLITE_DCHECK(NumDimensions(input) <= kMaxDim);
  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  const TfLiteEvalTensor* begin =
      tflite::micro::GetEvalInput(context, node, kBeginTensor);
  const TfLiteEvalTensor* size =
      tflite::micro::GetEvalInput(context, node, kSizeTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  tflite::SliceParams op_params;
  op_params.begin_count = kMaxDim;
  op_params.size_count = kMaxDim;
  for (int i = 0; i < kMaxDim; ++i) {
    op_params.begin[i] = 0;
    op_params.size[i] = 1;
  }

  if (begin->type == kTfLiteInt32) {
    GetBeginAndSizeVectors<int32_t>(input->dims->size, begin, size,
                                    op_params.begin, op_params.size);
  } else if (begin->type == kTfLiteInt64) {
    GetBeginAndSizeVectors<int64_t>(input->dims->size, begin, size,
                                    op_params.begin, op_params.size);
  } else {
    TF_LITE_KERNEL_LOG(context, "Begin tensor type %s (%d) not supported.",
                       TfLiteTypeGetName(input->type), input->type);
    return kTfLiteError;
  }

  switch (input->type) {
    case kTfLiteFloat32:
      reference_ops::Slice<float>(op_params,
                                  tflite::micro::GetTensorShape(input),
                                  tflite::micro::GetTensorData<float>(input),
                                  tflite::micro::GetTensorShape(output),
                                  tflite::micro::GetTensorData<float>(output));
      break;
    case kTfLiteInt32:
      reference_ops::Slice<int32_t>(
          op_params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int32_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int32_t>(output));
      break;
    case kTfLiteInt8:
      reference_ops::Slice<int8_t>(
          op_params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int8_t>(output));
      break;
    case kTfLiteInt16:
      reference_ops::Slice<int16_t>(
          op_params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int16_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int16_t>(output));
      break;
    default:
      MicroPrintf("Input tensor type %s (%d) not supported.",
                  TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration Register_SLICE() {
  return {/*init=*/nullptr,
          /*free=*/nullptr,
          /*prepare=*/Prepare,
          /*invoke=*/Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace tflite
