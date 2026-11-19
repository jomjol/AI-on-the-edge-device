#pragma once

#ifndef CTFLITECLASS_H
#define CTFLITECLASS_H

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "esp_err.h"
#include "esp_log.h"

#include "CImageBasis.h"

class CTfLiteClass
{
protected:
    tflite::MicroMutableOpResolver<10> resolver;
    const tflite::Model *model;
    tflite::MicroInterpreter *interpreter;
    TfLiteTensor *output = nullptr;

    int kTensorArenaSize;
    uint8_t *tensor_arena;

    unsigned char *modelfile = NULL;

    float *input;

    int input_i = 0;

    int im_height = 0;
    int im_width = 0;
    int im_channel = 0;

    long GetFileSize(std::string filename);
    bool ReadFileToModel(std::string _model);
    void MakeStaticResolver(void);

public:
    CTfLiteClass();
    ~CTfLiteClass();

    bool LoadModel(std::string _model);
    bool MakeAllocate(void);
    void GetInputTensorSize(void);
    bool LoadInputImageBasis(CImageBasis *rs);
    bool Invoke(void);
    int GetAnzOutPut(bool silent = true);
    int GetOutClassification(int _von = -1, int _bis = -1);

    int GetClassFromImageBasis(CImageBasis *rs);

    float GetOutputValue(int nr);
    void GetInputDimension(bool silent);
    int ReadInputDimenstion(int _dim);
};

#endif // CTFLITECLASS_H