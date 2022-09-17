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

// This file and the associated .cc file is branched from
// tensorflow/lite/kernels/internal/reference/portable_tensor_utils*
// TFLM needs to create its own because the original files are coupled with
// the tensor_utils module, which we cannot reuse due to its use of the
// Eigen library.

#ifndef TENSORFLOW_LITE_MICRO_KERNELS_MICRO_TENSOR_UTILS_H_
#define TENSORFLOW_LITE_MICRO_KERNELS_MICRO_TENSOR_UTILS_H_

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"

#if defined(_MSC_VER)
#define __restrict__ __restrict
#endif

namespace tflite {

// Not all backends support CpuBackendContext usage, so forward declare to avoid
// pulling in its implementation.
// TODO(b/230666277): consider removing this since micro does not utilize it
class CpuBackendContext;

namespace micro_tensor_utils {

template <typename T>
inline bool PortableIsZeroVector(const T* vector, int v_size) {
  for (int i = 0; i < v_size; ++i) {
    if (vector[i] != 0) {
      return false;
    }
  }
  return true;
}

void PortableSymmetricQuantizeFloats(const float* values, const int size,
                                     int8_t* quantized_values, float* min_value,
                                     float* max_value, float* scaling_factor);

void PortableSymmetricQuantizeFloats(const float* values, const int size,
                                     int8_t* quantized_values, float min_value,
                                     float max_value, float* scaling_factor);

void PortableAsymmetricQuantizeFloats(const float* values, const int size,
                                      int8_t* quantized_values,
                                      float* scaling_factor, int32_t* offset);

// Multiply a matrix by a batch vector, and store results in a batch-size
// vector.
void PortableMatrixBatchVectorMultiplyAccumulate(const float* matrix,
                                                 int m_rows, int m_cols,
                                                 const float* vector,
                                                 int n_batch, float* result);

void PortableMatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const int m_rows, const int m_cols,
    const int8_t* __restrict__ vectors, const float* scaling_factors,
    int n_batch, float* __restrict__ result);

void PortableMatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const int m_rows, const int m_cols,
    const int8_t* __restrict__ vectors, const float* scaling_factors,
    int n_batch, float* __restrict__ result, const float* per_channel_scale,
    const int32_t* input_offset, int32_t* scratch, int32_t* row_sums,
    bool* compute_row_sums, CpuBackendContext* context);

void PortableMatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const int m_rows, const int m_cols,
    const int8_t* __restrict__ vector, const float* scaling_factors,
    int n_batch, int32_t* scratch, float* __restrict__ result,
    CpuBackendContext* context);

void PortableSparseMatrixBatchVectorMultiplyAccumulate1x4(
    const float* __restrict__ matrix, const int32_t* __restrict__ segments,
    const int32_t* __restrict__ indices, int m_rows, int m_cols,
    const float* __restrict__ vector, int n_batch, float* __restrict__ result);

void PortableSparseMatrixBatchVectorMultiplyAccumulate(
    const float* __restrict__ matrix, const uint8_t* __restrict__ ledger,
    int m_rows, int m_cols, const float* __restrict__ vector, int n_batch,
    float* __restrict__ result);

void PortableSparseMatrixBatchVectorMultiplyAccumulate1x16(
    const int8_t* __restrict__ matrix, const int32_t* __restrict__ segments,
    const int32_t* __restrict__ indices, int m_rows, int m_cols,
    const int8_t* __restrict__ vector, const int32_t* __restrict__ bias_vector,
    int n_batch, const int32_t input_offset, const int32_t output_multiplier,
    const int32_t output_shift, const int32_t output_offset,
    const int32_t output_activation_min, const int32_t output_activation_max,
    int8_t* __restrict__ result);

void PortableSparseMatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const uint8_t* ledger, const int m_rows,
    const int m_cols, const int8_t* __restrict__ vectors,
    const float* scaling_factors, int n_batch, float* __restrict__ result);

// Dot product of two vectors.
float PortableVectorVectorDotProduct(const float* vector1, const float* vector2,
                                     int v_size);

void PortableBatchVectorBatchVectorDotProduct(const int16_t* vector1,
                                              const int16_t* vector2,
                                              int v_size, int n_batch,
                                              int32_t* result);

void PortableVectorBatchVectorCwiseProductAccumulate(
    const int16_t* vector, int v_size, const int16_t* batch_vector, int n_batch,
    int32_t multiplier, int shift, int16_t* result);

void PortableMatrixBatchVectorMultiplyAccumulate(
    const int8_t* input, const int32_t* bias,
    const int8_t* input_to_gate_weights, int32_t multiplier, int32_t shift,
    int32_t n_batch, int32_t n_input, int32_t n_output, int32_t output_zp,
    int32_t* scratch, int16_t* output, CpuBackendContext* context);

void PortableMatrixBatchVectorMultiplyAccumulate(
    const int8_t* input, const int32_t* bias,
    const int8_t* input_to_gate_weights, int32_t multiplier, int32_t shift,
    int32_t n_batch, int32_t n_input, int32_t n_output, int32_t output_zp,
    int32_t* scratch, int8_t* output, CpuBackendContext* context);

void PortableMatrixBatchVectorMultiply(const int8_t* input,
                                       int32_t input_zeropoint,
                                       const int8_t* input_to_gate_weights,
                                       int32_t input_to_gate_effective_scale_a,
                                       int32_t input_to_gate_effective_scale_b,
                                       int32_t n_batch, int32_t n_input,
                                       int32_t n_cell, int8_t* gate_output,
                                       int8_t gate_output_zp);

void PortableMatrixBatchVectorMultiply(
    const int16_t* hidden, const int8_t* hidden_to_output_weights,
    int32_t proj_effective_scale_a, int32_t proj_effective_scale_b,
    const int32_t* gate_bias, int32_t n_batch, int32_t n_hidden,
    int32_t n_output, int32_t output_zp, int8_t* proj_output);

void PortableMatrixScalarMultiplyAccumulate(const int8_t* matrix,
                                            int32_t scalar, int32_t n_row,
                                            int32_t n_col, int32_t* output);

void PortableApplyLayerNorm(const int16_t* input,
                            const int16_t* layer_norm_weights,
                            const int32_t* bias, int32_t layer_norm_scale_a,
                            int32_t layer_norm_scale_b, int32_t variance_limit,
                            int n_batch, int n_input, int16_t* output);

void PortableApplyLayerNormFloat(const int16_t* input,
                                 const int16_t* layer_norm_weights,
                                 int32_t layer_norm_scale_a,
                                 int32_t layer_norm_scale_b,
                                 const int32_t* bias, int n_batch, int n_input,
                                 int16_t* output);

void PortableApplySigmoid(const int16_t* input, int32_t n_batch,
                          int32_t n_input, int16_t* output);

void PortableApplySigmoidFloat(const int16_t* input, int32_t n_batch,
                               int32_t n_input, int16_t* output);

void PortableApplyTanh(int32_t integer_bits, const int16_t* input,
                       int32_t n_batch, int32_t n_input, int16_t* output);

void PortableApplyTanhFloat(const int16_t* input, int32_t n_batch,
                            int32_t n_input, int32_t integer_bits,
                            int16_t* output);

void PortableCwiseMul(const int16_t* input_1, const int16_t* input_2,
                      int n_batch, int n_input, int shift, int16_t* output);

void PortableCwiseMul(const int16_t* input_1, const int16_t* input_2,
                      int32_t multiplier, int32_t shift, int32_t n_batch,
                      int32_t n_input, int32_t output_zp, int8_t* output);

void PortableCwiseAdd(const int16_t* input_1, const int16_t* input_2,
                      int n_batch, int n_input, int16_t* output);

template <typename T>
inline void PortableCwiseClipping(T* vector, const int v_size,
                                  const T& clipping_value) {
  for (int i = 0; i < v_size; i++) {
    vector[i] = std::max(std::min(clipping_value, vector[i]),
                         static_cast<T>(-clipping_value));
  }
}

// Batch vector initialization with another vector.
void PortableVectorBatchVectorAssign(const float* vector, int v_size,
                                     int n_batch, float* batch_vector);

// Compute "1.0f - elements of vector" (used in CIFG).
void PortableSub1Vector(const float* vector, int v_size, float* result);

void PortableSub1Vector(const int16_t* vector, int v_size, int16_t* result);

// Multiply all elements of vector with a scalar.
void PortableVectorScalarMultiply(const int8_t* vector, int v_size, float scale,
                                  float* result);

// Reduce-sum on a vector:
// input_vector: pointer to input vector.
// output_vector: pointer to vector.
// output_size: output vector size.
// reduction_size: number of consecutive elements from input vector which are
// added to get one element of output.
template <typename INPUT, typename OUTPUT>
inline void PortableReductionSumVector(const INPUT* input_vector,
                                       OUTPUT* output_vector, int output_size,
                                       int reduction_size) {
  for (int o = 0; o < output_size; o++) {
    OUTPUT result = 0;
    for (int r = 0; r < reduction_size; r++) {
      result += input_vector[r];
    }
    output_vector[o] = result;
    input_vector += reduction_size;
  }
}

// Layer norm for each batch.
void PortableMeanStddevNormalization(const float* __restrict__ input_vector,
                                     float* __restrict__ output_vector,
                                     int v_size, int n_batch);

// Saturate Add.
void PortableTwoGateSaturatingAdd(const int8_t* input, int8_t input_zp,
                                  const int8_t* recurrent, int8_t recurrent_zp,
                                  int32_t input_effective_scale_a,
                                  int32_t input_effective_scale_b,
                                  int32_t recurrent_effective_scale_a,
                                  int32_t recurrent_effective_scale_b,
                                  int32_t n_batch, int32_t n_cell,
                                  int16_t* output);

// Add another vector for each batch in the batch vector.
template <typename T>
inline void VectorBatchVectorAdd(const T* vector, int v_size, int n_batch,
                                 T* batch_vector) {
  for (int b = 0; b < n_batch; b++) {
    for (int i = 0; i < v_size; ++i) {
      batch_vector[i] += vector[i];
    }
    batch_vector += v_size;
  }
}

// Cwise product of two vectors.
template <typename T>
inline void VectorVectorCwiseProduct(const T* vector1, const T* vector2,
                                     int v_size, T* result) {
  for (int v = 0; v < v_size; v++) {
    *result++ = *vector1++ * *vector2++;
  }
}

// Cwise product of a vector and a batch-vector.
template <typename T>
inline void VectorBatchVectorCwiseProduct(const T* vector, int v_size,
                                          const T* batch_vector, int n_batch,
                                          T* result) {
  for (int b = 0; b < n_batch; b++) {
    VectorVectorCwiseProduct(vector, batch_vector, v_size, result);
    // Update the pointers.
    result += v_size;
    batch_vector += v_size;
  }
}

// Reduce-sum on a float input vector:
// input_vector: float pointer to input vector.
// output_vector: float pointer to vector.
// output_size: output vector size.
// reduction_size: number of consecutive elements from input vector which are
// added to get one element of output.
inline void ReductionSumVector(const float* input_vector, float* output_vector,
                               int output_size, int reduction_size) {
  PortableReductionSumVector(input_vector, output_vector, output_size,
                             reduction_size);
}

// Same as above but input/output is 32 bit integer.
inline void ReductionSumVector(const int32_t* input_vector,
                               int32_t* output_vector, int output_size,
                               int reduction_size) {
  PortableReductionSumVector(input_vector, output_vector, output_size,
                             reduction_size);
}

// Same as above but input is 8 bit integer.
inline void ReductionSumVector(const int8_t* input_vector,
                               int32_t* output_vector, int output_size,
                               int reduction_size) {
  PortableReductionSumVector(input_vector, output_vector, output_size,
                             reduction_size);
}

// Cwise product and accumulate of two vectors. Since it's a MAC operation, the
// assumption here is that result array is initialized to valid values.
template <typename T>
inline void VectorVectorCwiseProductAccumulate(const T* __restrict__ vector1,
                                               const T* __restrict__ vector2,
                                               int v_size,
                                               T* __restrict__ result) {
  for (int v = 0; v < v_size; v++) {
    *result++ += *vector1++ * *vector2++;
  }
}

// Batch vector initialization with another vector.
template <typename T>
inline void VectorBatchVectorAssign(const T* vector, int v_size, int n_batch,
                                    T* batch_vector) {
  for (int b = 0; b < n_batch; b++) {
    std::copy_n(vector, v_size, batch_vector + b * v_size);
  }
}

inline void SymmetricQuantizeFloats(const float* values, const int size,
                                    int8_t* quantized_values, float* min,
                                    float* max, float* scaling_factor) {
  PortableSymmetricQuantizeFloats(values, size, quantized_values, min, max,
                                  scaling_factor);
}

inline void SymmetricQuantizeFloats(const float* values, const int size,
                                    int8_t* quantized_values, float min_value,
                                    float max_value, float* scaling_factor) {
  PortableSymmetricQuantizeFloats(values, size, quantized_values, min_value,
                                  max_value, scaling_factor);
}

inline void AsymmetricQuantizeFloats(const float* values, const int size,
                                     int8_t* quantized_values,
                                     float* scaling_factor, int32_t* offset) {
  PortableAsymmetricQuantizeFloats(values, size, quantized_values,
                                   scaling_factor, offset);
}

// Helper function to quantize floats.
// float_data_ptr     input float vectors
// n_batch            number of input vectors
// n_data             size of a single input vector
// quantized_data_ptr (out) vector with quantized data
// scaling_factors    (out) scaling factors (one per vector)
// zero_points        (out) zero points (one per vector)
// do_asymmetric      controls if the quantization should be asymmetric.
inline void BatchQuantizeFloats(const float* float_data_ptr, int n_batch,
                                int n_data, int8_t* quantized_data_ptr,
                                float* scaling_factors, int32_t* zero_points,
                                bool do_asymmetric) {
  for (int b = 0; b < n_batch; ++b) {
    const int offset = b * n_data;
    if (do_asymmetric) {
      AsymmetricQuantizeFloats(float_data_ptr + offset, n_data,
                               quantized_data_ptr + offset, &scaling_factors[b],
                               &zero_points[b]);
    } else {
      float unused_min, unused_max;
      SymmetricQuantizeFloats(float_data_ptr + offset, n_data,
                              quantized_data_ptr + offset, &unused_min,
                              &unused_max, &scaling_factors[b]);
    }
  }
}

// Check if all entries of a vector are zero for float.
inline bool IsZeroVector(const float* vector, int v_size) {
  return PortableIsZeroVector(vector, v_size);
}

// Check if all entries of a vector are zero for int8_t.
inline bool IsZeroVector(const int8_t* vector, int v_size) {
  return PortableIsZeroVector(vector, v_size);
}

// Apply Layer Normalization (https://arxiv.org/abs/1607.06450) to a Quantized
// vector.
// Parameters:
//     - input: batch vector of size n_batch * n_input; 16 bit.
//     - layer_norm_weights:  the quantized layer normalization weights.
//     - bias: the bias for the layer normalization.
//     - layer_norm_scale_a: multiplier for scale factor.
//     - layer_norm_scale_b: shift for scale factor.
//     - variance_limit: the guard to make sure the inverse does not overflow.
//     - n_batch: the number of batches.
//     - n_input: the size for input and output.
//     - output:  the 16 bit output
inline void ApplyLayerNorm(const int16_t* input,
                           const int16_t* layer_norm_weights,
                           const int32_t* bias, int32_t layer_norm_scale_a,
                           int32_t layer_norm_scale_b, int32_t variance_limit,
                           int n_batch, int n_input, int16_t* output) {
  PortableApplyLayerNorm(input, layer_norm_weights, bias, layer_norm_scale_a,
                         layer_norm_scale_b, variance_limit, n_batch, n_input,
                         output);
}

// Same as above but the internal calculation is done in float.
inline void ApplyLayerNormFloat(const int16_t* input,
                                const int16_t* layer_norm_weights,
                                int32_t layer_norm_scale_a,
                                int32_t layer_norm_scale_b, const int32_t* bias,
                                int n_batch, int n_input, int16_t* output) {
  PortableApplyLayerNormFloat(input, layer_norm_weights, layer_norm_scale_a,
                              layer_norm_scale_b, bias, n_batch, n_input,
                              output);
}

// Apply Sigmoid to a quantized vector.
// Parameters:
//     - input: batch vector of size n_batch * n_input; 16 bit.
//     - n_batch: the number of batches.
//     - n_input: the size for input and output.
//     - output:  the 16 bit output
// The input is in Q3.12 format and the output is in Q0.15 format.
inline void ApplySigmoid(const int16_t* input, int32_t n_batch, int32_t n_input,
                         int16_t* output) {
  PortableApplySigmoid(input, n_batch, n_input, output);
}

// Same as above but the internal calcualtion is float.
inline void ApplySigmoidFloat(const int16_t* input, int32_t n_batch,
                              int32_t n_input, int16_t* output) {
  PortableApplySigmoidFloat(input, n_batch, n_input, output);
}

// Apply Tanh to a quantized vector.
// Parameters:
//     - integer_bits: the integer bits of the input.
//                     Currently supports 0, 1, 2, 3, 4, 5, 6.
//     - input: batch vector of size n_batch * n_input; 16 bit.
//     - n_batch: the number of batches.
//     - n_input: the size for input and output.
//     - output:  the 16 bit output
// The input is in Qm.15-m format and the output is in Q0.15 format.
inline void ApplyTanh(int32_t integer_bits, const int16_t* input,
                      int32_t n_batch, int32_t n_input, int16_t* output) {
  PortableApplyTanh(integer_bits, input, n_batch, n_input, output);
}

// Apply Tanh to a quantized vector. Tbe internal calculation is in float.
//    - Input has 2^(integer_bits) as scale.
//    - Output has Q0.15 as scale.
inline void ApplyTanhFloat(const int16_t* input, int32_t n_batch,
                           int32_t n_input, int32_t integer_bits,
                           int16_t* output) {
  PortableApplyTanhFloat(input, n_batch, n_input, integer_bits, output);
}

// Element-wise multiplication of two quantized vectors.
// Parameters:
//     - input_1: batch vector of size n_batch * n_input; 16 bit.
//     - input_2: batch vector of size n_batch * n_input; 16 bit.
//     - n_batch: the number of batches.
//     - n_input: the size for input and output.
//     - shift:   the shift needed to produce the output.
//     - output:  the 16 bit output of size n_batch * n_input.
// Output does not need to be initialized.
inline void CwiseMul(const int16_t* input_1, const int16_t* input_2,
                     int n_batch, int n_input, int shift, int16_t* output) {
  PortableCwiseMul(input_1, input_2, n_batch, n_input, shift, output);
}

// Element-wise multiplication of two quantized vectors with rescaling.
// Parameters:
//     - input_1:    batch vector of size n_batch * n_input; 16 bit.
//     - input_2:    batch vector of size n_batch * n_input; 16 bit.
//     - multiplier: the multiplier part of scale.
//     - shift:      the shift part of scale.
//     - n_batch:    the number of batches.
//     - n_input:    the size for input and output.
//     - output:     the 8 bit output of size n_batch * n_input.
//     - output_zp:  the zero point of output.
// Output does not need to be initialized.
// Multiplier ("m") and shift ("s") are connected to scale ("s") with s = m *
// 2^(s - 31).
inline void CwiseMul(const int16_t* input_1, const int16_t* input_2,
                     int32_t multiplier, int32_t shift, int32_t n_batch,
                     int32_t n_input, int32_t output_zp, int8_t* output) {
  PortableCwiseMul(input_1, input_2, multiplier, shift, n_batch, n_input,
                   output_zp, output);
}

// Element-wise in-place clipping of a vector. Overloaded for float, int16_t,
// int8_t. Parameters:
//     - vector:         vector of size v_size.
//     - v_size:         the size of the vector.
//     - clipping_value: the value used for clipping.
inline void CwiseClipping(float* vector, const int v_size,
                          const float clipping_value) {
  PortableCwiseClipping(vector, v_size, clipping_value);
}

inline void CwiseClipping(int16_t* vector, const int v_size,
                          const int16_t clipping_value) {
  PortableCwiseClipping(vector, v_size, clipping_value);
}

inline void CwiseClipping(int8_t* vector, const int v_size,
                          const int8_t clipping_value) {
  PortableCwiseClipping(vector, v_size, clipping_value);
}

// Element-wise saturating addition of two quantized vectors without rescaling.
// Parameters:
//     - input_1:    batch vector of size n_batch * n_input; 16 bit.
//     - input_2:    batch vector of size n_batch * n_input; 16 bit.
//     - n_batch:    the number of batches.
//     - n_input:    the size for input and output.
//     - output:     the 8 bit output of size n_batch * n_input.
// Output does not need to be initialized.
inline void CwiseAdd(const int16_t* input_1, const int16_t* input_2,
                     int n_batch, int n_input, int16_t* output) {
  PortableCwiseAdd(input_1, input_2, n_batch, n_input, output);
}

inline void MeanStddevNormalization(const float* input_vector,
                                    float* output_vector, int v_size,
                                    int n_batch) {
  PortableMeanStddevNormalization(input_vector, output_vector, v_size, n_batch);
}

inline void Sub1Vector(const float* vector, int v_size, float* result) {
  PortableSub1Vector(vector, v_size, result);
}

inline void Sub1Vector(const int16_t* vector, int v_size, int16_t* result) {
  PortableSub1Vector(vector, v_size, result);
}

// Multiply all elements of vector with a scalar.
inline void VectorScalarMultiply(const int8_t* vector, int v_size, float scale,
                                 float* result) {
  PortableVectorScalarMultiply(vector, v_size, scale, result);
}

// Saturate Add with rescale on both inputs.
inline void TwoGateSaturatingAdd(const int8_t* input, int8_t input_zp,
                                 const int8_t* recurrent, int8_t recurrent_zp,
                                 int32_t input_effective_scale_a,
                                 int32_t input_effective_scale_b,
                                 int32_t recurrent_effective_scale_a,
                                 int32_t recurrent_effective_scale_b,
                                 int32_t n_batch, int32_t n_cell,
                                 int16_t* output) {
  PortableTwoGateSaturatingAdd(
      input, input_zp, recurrent, recurrent_zp, input_effective_scale_a,
      input_effective_scale_b, recurrent_effective_scale_a,
      recurrent_effective_scale_b, n_batch, n_cell, output);
}

// Multiplies a matrix by a "batched" vector (i.e. a matrix with a batch
// dimension composed by input vectors independent from each other). The result
// of the multiplication is accumulated to the passed result buffer.
// More specifically, for a matrix M of shape [n, i] and a batched-vector
// of shape [i, batch] it will first compute the product of shape [n, batch].
// This product will be accumulated to the result buffer.
inline void MatrixBatchVectorMultiplyAccumulate(const float* matrix, int m_rows,
                                                int m_cols, const float* vector,
                                                int n_batch, float* result) {
  PortableMatrixBatchVectorMultiplyAccumulate(matrix, m_rows, m_cols, vector,
                                              n_batch, result);
}

// Same as the function above, but the matrix is a sparse tensor with block
// pattern 1x4.
// This function assumes that m_cols is a multiple of the block size (4 in this
// case) so that there's no incomplete block.
inline void MatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const int m_rows, const int m_cols,
    const int8_t* __restrict__ vector, const float* scaling_factors,
    int n_batch, float* __restrict__ result) {
  PortableMatrixBatchVectorMultiplyAccumulate(matrix, m_rows, m_cols, vector,
                                              scaling_factors, n_batch, result);
}

inline void MatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const int m_rows, const int m_cols,
    const int8_t* __restrict__ vectors, const float* scaling_factors,
    int n_batch, float* __restrict__ result, const float* per_channel_scale,
    const int32_t* input_offset, int32_t* scratch, int32_t* row_sums,
    bool* compute_row_sums, CpuBackendContext* context) {
  PortableMatrixBatchVectorMultiplyAccumulate(
      matrix, m_rows, m_cols, vectors, scaling_factors, n_batch, result,
      per_channel_scale, input_offset, scratch, row_sums, compute_row_sums,
      context);
}

inline void MatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const int m_rows, const int m_cols,
    const int8_t* __restrict__ vector, const float* scaling_factors,
    int n_batch, int32_t* scratch, float* __restrict__ result,
    CpuBackendContext* context) {
  PortableMatrixBatchVectorMultiplyAccumulate(matrix, m_rows, m_cols, vector,
                                              scaling_factors, n_batch, result);
}

// Same as the function above, but the matrix is a sparse tensor with block
// pattern 1x4.
// This function assumes that m_cols is a multiple of the block size (4 in this
// case) so that there's no incomplete block.
inline void SparseMatrixBatchVectorMultiplyAccumulate1x4(
    const float* __restrict__ matrix, const int32_t* __restrict__ segments,
    const int32_t* __restrict__ indices, int m_rows, int m_cols,
    const float* __restrict__ vector, int n_batch, float* __restrict__ result) {
  PortableSparseMatrixBatchVectorMultiplyAccumulate1x4(
      matrix, segments, indices, m_rows, m_cols, vector, n_batch, result);
}

// Same as the function above, but the matrix is stored in block compressed
// sparse row format with block pattern 1x16 which consists of two arrays:
//   1. A matrix array stores non-zero blocks of the matrix in row major.
//   2. A ledger array stores nrows groups, one group per row. Each group starts
//      with an integer representing the number of non-zero blocks for the
//      corresponding row and follows with column indexes of the first element
//      of each non-zero block.
// This function assumes that
//   1. m_cols is a multiple of 16 so that all blocks are full blocks.
//   2. m_cols < 254 * 16 so that block index can be represented by uint8.
inline void SparseMatrixBatchVectorMultiplyAccumulate(
    const float* __restrict__ matrix, const uint8_t* __restrict__ ledger,
    int m_rows, int m_cols, const float* __restrict__ vector, int n_batch,
    float* __restrict__ result) {
  PortableSparseMatrixBatchVectorMultiplyAccumulate(
      matrix, ledger, m_rows, m_cols, vector, n_batch, result);
}

// Same as the function above, but the matrix is a sparse tensor with block
// pattern 1x16.
// This function assumes that m_cols is a multiple of the block size (16 in this
// case) so that there's no incomplete block. Also, it assumes all offsets of
// input, output and filter are zero.
inline void SparseMatrixBatchVectorMultiplyAccumulate1x16(
    const int8_t* __restrict__ matrix, const int32_t* __restrict__ segments,
    const int32_t* __restrict__ indices, int m_rows, int m_cols,
    const int8_t* __restrict__ vector, const int32_t* __restrict__ bias_vector,
    int n_batch, const int32_t input_offset, const int32_t output_multiplier,
    const int32_t output_shift, const int32_t output_offset,
    const int32_t output_activation_min, const int32_t output_activation_max,
    int8_t* __restrict__ result) {
  PortableSparseMatrixBatchVectorMultiplyAccumulate1x16(
      matrix, segments, indices, m_rows, m_cols, vector, bias_vector, n_batch,
      input_offset, output_multiplier, output_shift, output_offset,
      output_activation_min, output_activation_max, result);
}

// Same as the function above, but the matrix is stored in block compressed
// sparse row format with block pattern 1x16 which consists of two arrays:
//   1. A matrix array stores non-zero blocks of the matrix in row major.
//   2. A ledger array stores nrows groups, one group per row. Each group starts
//      with an integer representing the number of non-zero blocks for the
//      corresponding row followed by column index of the first element of
//      each non-zero block.
// This function assumes that
//   1. m_cols is a multiple of 16 so that all blocks are full blocks.
//   2. m_cols < 254 * 16 so that block index can be represented by uint8.
inline void SparseMatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const uint8_t* ledger, const int m_rows,
    const int m_cols, const int8_t* __restrict__ vectors,
    const float* scaling_factors, int n_batch, float* __restrict__ result) {
  PortableSparseMatrixBatchVectorMultiplyAccumulate(
      matrix, ledger, m_rows, m_cols, vectors, scaling_factors, n_batch,
      result);
}

// Same as the above 8, 8, 8 integer matmul except for the presence of zero
// point and non-accumulative.
// TODO(b/148688698): remove this function by folding zero point calculation in
// prepare() function.
inline void MatrixBatchVectorMultiplyAccumulate(
    const int8_t* input, const int32_t* bias,
    const int8_t* input_to_gate_weights, int32_t multiplier, int32_t shift,
    int32_t n_batch, int32_t n_input, int32_t n_output, int32_t output_zp,
    int32_t* scratch, int16_t* output, CpuBackendContext* context) {
  PortableMatrixBatchVectorMultiplyAccumulate(
      input, bias, input_to_gate_weights, multiplier, shift, n_batch, n_input,
      n_output, output_zp, scratch, output, context);
}

// Same as above but has 16 bit and 8 bit input and 8 bit output.
// Used in projection when hidden is 16bit.
inline void MatrixBatchVectorMultiplyAccumulate(
    const int8_t* input, const int32_t* bias,
    const int8_t* input_to_gate_weights, int32_t multiplier, int32_t shift,
    int32_t n_batch, int32_t n_input, int32_t n_output, int32_t output_zp,
    int32_t* scratch, int8_t* output, CpuBackendContext* context) {
  PortableMatrixBatchVectorMultiplyAccumulate(
      input, bias, input_to_gate_weights, multiplier, shift, n_batch, n_input,
      n_output, output_zp, scratch, output, context);
}

// Same as the function above, but provides separate scaling factor for the
// matrix and the vectors. The scaling factors are multiplied in the
// scaling_factor_scratch buffer.
inline void MatrixBatchVectorMultiplyAccumulate(
    const int8_t* __restrict__ matrix, const int m_rows, const int m_cols,
    const int8_t* __restrict__ vectors, const float matrix_scaling_factor,
    const float* vector_scaling_factors, int n_batch,
    float* __restrict__ result, const float* per_channel_scale,
    const int32_t* input_offset, int32_t* scratch, int32_t* row_sums,
    bool* compute_row_sums, float* scaling_factor_scratch,
    CpuBackendContext* context) {
  for (int b = 0; b < n_batch; ++b) {
    scaling_factor_scratch[b] =
        vector_scaling_factors[b] * matrix_scaling_factor;
  }
  MatrixBatchVectorMultiplyAccumulate(matrix, m_rows, m_cols, vectors,
                                      scaling_factor_scratch, n_batch, result,
                                      per_channel_scale, input_offset, scratch,
                                      row_sums, compute_row_sums, context);
}

// Multiplies a matrix with a scalar and reduce the result on each row to a
// scalar.
// Parameters:
//     - matrix: matrix of size n_row * n_col
//     - scalar: the scalar that is multiplied to each element in the matrix
//     - n_row:  the row count of the matrix
//     - n_col:  the column count of the matrix
//     - output: the 32bit output
// Note: We do not need saturation because the int8 * int8 is safe from overflow
// in (2^31-1) / (2^14) = 131072, which is bigger than the n_row. Non-zero
// initial output value is not exceptionally large.
inline void MatrixScalarMultiplyAccumulate(const int8_t* matrix, int32_t scalar,
                                           int32_t n_row, int32_t n_col,
                                           int32_t* output) {
  PortableMatrixScalarMultiplyAccumulate(matrix, scalar, n_row, n_col, output);
}

// Same as the above 8, 8, 8 integer matmul except for the presence of zero
// point and non-accumulative.
// TODO(b/148688698): remove this function by folding zero point calculation in
// prepare() function.
inline void MatrixBatchVectorMultiply(const int8_t* input,
                                      int32_t input_zeropoint,
                                      const int8_t* input_to_gate_weights,
                                      int32_t input_to_gate_effective_scale_a,
                                      int32_t input_to_gate_effective_scale_b,
                                      int32_t n_batch, int32_t n_input,
                                      int32_t n_cell, int8_t* gate_output,
                                      int8_t gate_output_zp) {
  PortableMatrixBatchVectorMultiply(
      input, input_zeropoint, input_to_gate_weights,
      input_to_gate_effective_scale_a, input_to_gate_effective_scale_b, n_batch,
      n_input, n_cell, gate_output, gate_output_zp);
}

// Same as above but has 16 bit and 8 bit input and 8 bit output.
// Used in projection when hidden is 16bit.
inline void MatrixBatchVectorMultiply(const int16_t* hidden,
                                      const int8_t* hidden_to_output_weights,
                                      int32_t proj_effective_scale_a,
                                      int32_t proj_effective_scale_b,
                                      const int32_t* gate_bias, int32_t n_batch,
                                      int32_t n_hidden, int32_t n_output,
                                      int32_t output_zp, int8_t* proj_output) {
  PortableMatrixBatchVectorMultiply(hidden, hidden_to_output_weights,
                                    proj_effective_scale_a,
                                    proj_effective_scale_b, gate_bias, n_batch,
                                    n_hidden, n_output, output_zp, proj_output);
}

// Cwise product and accumulate of a vector and a batch-vector. Since it's a MAC
// operation, the assumption here is that result array is initialized to valid
// values.
template <typename T>
inline void VectorBatchVectorCwiseProductAccumulate(const T* vector, int v_size,
                                                    const T* batch_vector,
                                                    int n_batch, T* result) {
  for (int b = 0; b < n_batch; b++) {
    VectorVectorCwiseProductAccumulate(vector, batch_vector, v_size, result);
    // Update the pointers.
    result += v_size;
    batch_vector += v_size;
  }
}

// Same as above, but inputs are 16bit integer and output is 16bit integer.
inline void VectorBatchVectorCwiseProductAccumulate(
    const int16_t* vector, int v_size, const int16_t* batch_vector, int n_batch,
    int32_t multiplier, int shift, int16_t* result) {
  PortableVectorBatchVectorCwiseProductAccumulate(
      vector, v_size, batch_vector, n_batch, multiplier, shift, result);
}

// Apply Rectified Linear to elements of a vector.
inline void ApplyReluToVector(const float* vector, int v_size, float* result) {
  for (int v = 0; v < v_size; v++) {
    result[v] = std::max(0.0f, vector[v]);
  }
}

// Apply Rectified Linear 1 (cap to [-1;1]) to elements of a vector
inline void ApplyRelu1ToVector(const float* vector, int v_size, float* result) {
  for (int v = 0; v < v_size; v++) {
    result[v] = std::max(-1.0f, std::min(vector[v], 1.0f));
  }
}

// Apply Rectified Linear 6 (cap to [0;6]) to elements of a vector
inline void ApplyRelu6ToVector(const float* vector, int v_size, float* result) {
  for (int v = 0; v < v_size; v++) {
    result[v] = std::max(0.0f, std::min(vector[v], 6.0f));
  }
}

// Apply tanh to elements of a vector
inline void ApplyTanhToVector(const float* vector, int v_size, float* result) {
  for (int v = 0; v < v_size; v++) {
    result[v] = std::tanh(vector[v]);
  }
}

// Apply signbit to elements of a vector
inline void ApplySignbitToVector(const float* vector, int v_size,
                                 float* result) {
  for (int v = 0; v < v_size; v++) {
    result[v] = std::signbit(vector[v]);
  }
}

// Apply sigmoid to elements of a vector.
inline void ApplySigmoidToVector(const float* vector, int v_size,
                                 float* result) {
  for (int v = 0; v < v_size; v++) {
    result[v] = 1.0f / (1.0f + std::exp(-vector[v]));
  }
}

// Apply appropriate activation function to elements of a vector.
inline void ApplyActivationToVector(const float* vector, int v_size,
                                    TfLiteFusedActivation activation,
                                    float* result) {
  switch (activation) {
    case kTfLiteActNone:
      return;
    case kTfLiteActRelu:
      return ApplyReluToVector(vector, v_size, result);
    case kTfLiteActReluN1To1:
      return ApplyRelu1ToVector(vector, v_size, result);
    case kTfLiteActRelu6:
      return ApplyRelu6ToVector(vector, v_size, result);
    case kTfLiteActTanh:
      return ApplyTanhToVector(vector, v_size, result);
    case kTfLiteActSignBit:
      return ApplySignbitToVector(vector, v_size, result);
    case kTfLiteActSigmoid:
      return ApplySigmoidToVector(vector, v_size, result);
  }
}

}  // namespace micro_tensor_utils

}  // namespace tflite

#endif  // TENSORFLOW_LITE_MICRO_KERNELS_MICRO_TENSOR_UTILS_H_
