#pragma once

#ifndef __CFINDTEMPLATE
#define __CFINGTEMPLATE

#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x)) {                                                \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }

//#include "CAccessSD.h"
#include "CFindTemplate.h"

#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "esp_err.h"
#include "esp_log.h"

//extern CAccessSDClass accessSD;

class CTfLiteClass
{
    protected:
//        CAccessSDClass *accessSD;

        tflite::ErrorReporter* error_reporter;
        
        const tflite::Model* model;
        tflite::MicroInterpreter* interpreter;
//        TfLiteTensor* input = nullptr;
        TfLiteTensor* output = nullptr;     
        static tflite::ops::micro::AllOpsResolver *resolver; 

        tflite::MicroOpResolver<5> micro_op_resolver;
  

        int kTensorArenaSize;
        uint8_t *tensor_arena;

        float* input;
        int input_i;

        int im_height, im_width, im_channel;

        long GetFileSize(std::string filename);
        unsigned char* ReadFileToCharArray(std::string _fn);
        
    public:
//        CTfLiteClass(CAccessSDClass *_accessSD);
        CTfLiteClass();
        ~CTfLiteClass();        
        void LoadModel(std::string _fn);
        void MakeAllocate();
        void GetInputTensorSize();
        bool LoadInputImage(std::string _fn);
        void Invoke();
        void GetOutPut();
        int GetOutClassification();
        int GetClassFromImage(std::string _fn);

        float GetOutputValue(int nr);
        void GetInputDimension(bool silent);

};


#endif