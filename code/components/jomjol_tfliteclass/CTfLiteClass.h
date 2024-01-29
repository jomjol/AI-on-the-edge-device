#pragma once

#ifndef CTFLITECLASS_H
#define CTFLITECLASS_H

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"

#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "esp_err.h"
#include "esp_log.h"

#include "CImageBasis.h"



#ifdef SUPRESS_TFLITE_ERRORS
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/micro/compatibility.h"
#include "tensorflow/lite/micro/debug_log.h"
///// OwnErrorReporter to prevent printing of Errors (especially unavoidable in CalculateActivationRangeQuantized@kerne_util.cc)
namespace tflite {
    class OwnMicroErrorReporter : public ErrorReporter {
        public:
           int Report(const char* format, va_list args) override;
    };
}  // namespace tflite
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


class CTfLiteClass
{
    protected:
        tflite::ErrorReporter *error_reporter;
        const tflite::Model* model;
        tflite::MicroInterpreter* interpreter;
        TfLiteTensor* output = nullptr;     

        int kTensorArenaSize;
        uint8_t *tensor_arena;

        unsigned char *modelfile = NULL;


        float* input;
        int input_i;
        int im_height, im_width, im_channel;

        long GetFileSize(std::string filename);
        bool ReadFileToModel(std::string _fn);

    public:
        CTfLiteClass();
        ~CTfLiteClass();        
        bool LoadModel(std::string _fn);
        bool MakeAllocate();
        void GetInputTensorSize();
        bool LoadInputImageBasis(CImageBasis *rs);
        void Invoke();
        int GetAnzOutPut(bool silent = true);        
        int GetOutClassification(int _von = -1, int _bis = -1);

        int GetClassFromImageBasis(CImageBasis *rs);
        std::string GetStatusFlow();

        float GetOutputValue(int nr);
        void GetInputDimension(bool silent);
        int ReadInputDimenstion(int _dim);
};

void MakeStaticResolver();

#endif //CTFLITECLASS_H