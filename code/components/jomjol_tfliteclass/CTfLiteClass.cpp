#include "CTfLiteClass.h"
#include "ClassLogFile.h"
#include "Helper.h"
#include "psram.h"
#include "esp_log.h"
#include "defines.h"

#include <sys/stat.h>
#include <stdint.h>
#include <stddef.h>

static const char *TAG = "TFLITE";

CTfLiteClass::CTfLiteClass()
{
    model = nullptr;
    modelfile = NULL;
    interpreter = nullptr;
    input = nullptr;
    output = nullptr;
    kTensorArenaSize = TENSOR_ARENA_SIZE;
    tensor_arena = (uint8_t *)psram_get_shared_tensor_arena_memory();
    im_height = 0;
    im_width = 0;
    im_channel = 0;
    input_i = 0;
}

CTfLiteClass::~CTfLiteClass()
{
    delete interpreter;

    psram_free_shared_tensor_arena_and_model_memory();
}

void CTfLiteClass::MakeStaticResolver()
{
    resolver.AddFullyConnected();
    resolver.AddReshape();
    resolver.AddSoftmax();
    resolver.AddConv2D();
    resolver.AddMaxPool2D();
    resolver.AddQuantize();
    resolver.AddMul();
    resolver.AddAdd();
    resolver.AddLeakyRelu();
    resolver.AddDequantize();
}

float CTfLiteClass::GetOutputValue(int nr)
{
    if (interpreter == nullptr || nr < 0)
    {
        return -1000.0f;
    }

    TfLiteTensor *output2 = interpreter->output(0);
    if (output2 == nullptr || output2->type != kTfLiteFloat32 || output2->dims == nullptr || output2->dims->size < 2 || output2->data.f == nullptr)
    {
        return -1000.0f;
    }

    const int outputCount = output2->dims->data[1];

    if (outputCount <= nr || static_cast<size_t>(outputCount) > SIZE_MAX / sizeof(float) || output2->bytes < static_cast<size_t>(outputCount) * sizeof(float))
    {
        return -1000.0f;
    }

    return output2->data.f[nr];
}

int CTfLiteClass::GetClassFromImageBasis(CImageBasis *rs)
{
    if (!LoadInputImageBasis(rs))
    {
        return -1000;
    }

    if (!Invoke())
    {
        return -1000;
    }

    return GetOutClassification();
}

int CTfLiteClass::GetOutClassification(int _von, int _bis)
{
    if (interpreter == nullptr)
    {
        return -1;
    }

    TfLiteTensor *output2 = interpreter->output(0);

    if (output2 == nullptr || output2->type != kTfLiteFloat32 || output2->dims == nullptr || output2->dims->size < 2 || output2->data.f == nullptr)
    {
        return -1;
    }

    const int outputCount = output2->dims->data[1];

    if (outputCount <= 0 || static_cast<size_t>(outputCount) > SIZE_MAX / sizeof(float) || output2->bytes < static_cast<size_t>(outputCount) * sizeof(float))
    {
        return -1;
    }

    if (_von == -1)
    {
        _von = 0;
    }

    if (_bis == -1)
    {
        _bis = outputCount - 1;
    }

    if (_von < 0 || _bis < _von || _bis >= outputCount)
    {
        ESP_LOGE(TAG, "Invalid output range [%d, %d] for %d outputs", _von, _bis, outputCount);
        return -1;
    }

    float maxValue = output2->data.f[_von];
    int maxClass = _von;

    for (int i = _von + 1; i <= _bis; ++i)
    {
        const float value = output2->data.f[i];

        if (value > maxValue)
        {
            maxValue = value;
            maxClass = i;
        }
    }

    return maxClass - _von;
}

void CTfLiteClass::GetInputDimension(bool silent)
{
    im_height = im_width = im_channel = 0;

    if (interpreter == nullptr)
    {
        return;
    }

    TfLiteTensor *inputTensor = interpreter->input(0);

    if (inputTensor == nullptr || inputTensor->dims == nullptr || inputTensor->dims->size < 4)
    {
        if (!silent)
        {
            ESP_LOGE(TAG, "Input tensor or dimensions are invalid");
        }

        return;
    }

    const int batch = inputTensor->dims->data[0];
    im_height = inputTensor->dims->data[1];
    im_width = inputTensor->dims->data[2];
    im_channel = inputTensor->dims->data[3];

    if (!silent)
    {
        ESP_LOGD(TAG, "Input tensor: batch=%d height=%d width=%d channels=%d bytes=%u", batch, im_height, im_width, im_channel, static_cast<unsigned>(inputTensor->bytes));
    }

    if (batch != 1 || im_height <= 0 || im_width <= 0 || im_channel <= 0)
    {
        im_height = im_width = im_channel = 0;

        if (!silent)
        {
            ESP_LOGE(TAG, "Unsupported input tensor dimensions");
        }
    }
}

int CTfLiteClass::ReadInputDimenstion(int _dim)
{
    if (_dim == 0)
    {
        return im_width;
    }

    if (_dim == 1)
    {
        return im_height;
    }

    if (_dim == 2)
    {
        return im_channel;
    }

    return -1;
}

int CTfLiteClass::GetAnzOutPut(bool silent)
{
    if (interpreter == nullptr)
    {
        return 0;
    }

    TfLiteTensor *output2 = interpreter->output(0);

    if (output2 == nullptr || output2->type != kTfLiteFloat32 || output2->dims == nullptr || output2->dims->size < 2 || output2->data.f == nullptr)
    {
        return 0;
    }

    const int outputCount = output2->dims->data[1];

    if (outputCount <= 0 || static_cast<size_t>(outputCount) > SIZE_MAX / sizeof(float) || output2->bytes < static_cast<size_t>(outputCount) * sizeof(float))
    {
        return 0;
    }

    if (!silent)
    {
        ESP_LOGD(TAG, "Output dimension: %d, output count: %d", output2->dims->size, outputCount);

        for (int i = 0; i < outputCount; ++i)
        {
            ESP_LOGD(TAG, "Result %d: %f", i, output2->data.f[i]);
        }
    }
    return outputCount;
}

bool CTfLiteClass::Invoke()
{
    if (interpreter == nullptr)
    {
        ESP_LOGE(TAG, "Invoke called without an interpreter");
        return false;
    }

    if (interpreter->Invoke() != kTfLiteOk)
    {
        ESP_LOGE(TAG, "TensorFlow Lite inference failed");
        return false;
    }

    return true;
}

bool CTfLiteClass::LoadInputImageBasis(CImageBasis *rs)
{
    if (interpreter == nullptr || rs == nullptr || !rs->ImageOkay())
    {
        ESP_LOGE(TAG, "Cannot load image: interpreter or source image is invalid");
        return false;
    }

    TfLiteTensor *inputTensor = interpreter->input(0);

    if (inputTensor == nullptr || inputTensor->dims == nullptr || inputTensor->dims->size < 4 || inputTensor->type != kTfLiteFloat32 || inputTensor->data.f == nullptr)
    {
        ESP_LOGE(TAG, "Input tensor is not a valid float32 image tensor");
        return false;
    }

    const int batch = inputTensor->dims->data[0];
    const int expectedHeight = inputTensor->dims->data[1];
    const int expectedWidth = inputTensor->dims->data[2];
    const int expectedChannels = inputTensor->dims->data[3];

    if (batch != 1 || expectedHeight <= 0 || expectedWidth <= 0 || expectedChannels != 3 || rs->width != expectedWidth || rs->height != expectedHeight || rs->channels < 3)
    {
        ESP_LOGE(TAG, "Image/tensor dimensions do not match (image=%dx%dx%d tensor=%dx%dx%d batch=%d)", rs->width, rs->height, rs->channels, expectedWidth, expectedHeight, expectedChannels, batch);
        return false;
    }

    if (static_cast<size_t>(expectedWidth) > SIZE_MAX / static_cast<size_t>(expectedHeight))
    {
        ESP_LOGE(TAG, "Input tensor dimensions overflow addressable memory");
        return false;
    }

    const size_t pixelCount = static_cast<size_t>(expectedWidth) * static_cast<size_t>(expectedHeight);

    if (pixelCount > SIZE_MAX / (3 * sizeof(float)) || inputTensor->bytes < pixelCount * 3 * sizeof(float))
    {
        ESP_LOGE(TAG, "Input tensor buffer is too small for the requested image");
        return false;
    }

    uint8_t *imageData = rs->RGBImageLock();

    if (imageData == nullptr)
    {
        rs->RGBImageRelease();
        ESP_LOGE(TAG, "Source image buffer is unavailable");
        return false;
    }

    float *inputData = inputTensor->data.f;

    for (size_t pixel = 0; pixel < pixelCount; ++pixel)
    {
        inputData[pixel * 3] = static_cast<float>(imageData[pixel * rs->channels]);
        inputData[pixel * 3 + 1] = static_cast<float>(imageData[pixel * rs->channels + 1]);
        inputData[pixel * 3 + 2] = static_cast<float>(imageData[pixel * rs->channels + 2]);
    }

    rs->RGBImageRelease();

    return true;
}

bool CTfLiteClass::MakeAllocate()
{
    if (model == nullptr || tensor_arena == nullptr || kTensorArenaSize <= 0)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Cannot allocate interpreter: model or tensor arena is unavailable");
        return false;
    }

    if (interpreter != nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Interpreter already allocated");
        return false;
    }

    MakeStaticResolver();

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CTfLiteClass::MakeAllocate");
    interpreter = new (std::nothrow) tflite::MicroInterpreter(model, resolver, tensor_arena, kTensorArenaSize);
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Trying to load the model. If it crashes here, it ist most likely due to a corrupted model!");

    if (interpreter)
    {
        TfLiteStatus allocate_status = interpreter->AllocateTensors();

        if (allocate_status != kTfLiteOk)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "AllocateTensors() failed");
            delete interpreter;
            interpreter = nullptr;
            return false;
        }

        GetInputDimension(true);

        if (im_width <= 0 || im_height <= 0 || im_channel != 3)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Unsupported input tensor dimensions");
            delete interpreter;
            interpreter = nullptr;
            return false;
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "new tflite::MicroInterpreter failed");
        LogFile.WriteHeapInfo("CTfLiteClass::MakeAllocate-new tflite::MicroInterpreter failed");
        return false;
    }

    return true;
}

void CTfLiteClass::GetInputTensorSize()
{
    ESP_LOGD(TAG, "Input Tensor Dimension: %dl", sizeof(input));
}

long CTfLiteClass::GetFileSize(std::string filename)
{
    struct stat stat_buf;
    long rc = -1;

    FILE *pFile = fopen(filename.c_str(), "rb"); // previously only "rb

    if (pFile != NULL)
    {
        rc = stat(filename.c_str(), &stat_buf);
        fclose(pFile);
    }

    return rc == 0 ? stat_buf.st_size : -1;
}

bool CTfLiteClass::ReadFileToModel(std::string _model)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CTfLiteClass::ReadFileToModel: " + _model);

    long size = GetFileSize(_model);

    if (size == -1)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Model file doesn't exist: " + _model + "!");
        return false;
    }
    else if (size <= 0)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Model file is empty: " + _model);
        return false;
    }
    else if (size > MAX_MODEL_SIZE)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Unable to load model '" + _model + "'! It does not fit in the reserved shared memory in PSRAM!");
        return false;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Loading Model " + _model + " /size: " + std::to_string(size) + " bytes...");

    modelfile = (unsigned char *)psram_get_shared_model_memory();

    if (modelfile != NULL)
    {
        FILE *pFile = fopen(_model.c_str(), "rb"); // previously only "rb

        if (pFile != NULL)
        {
            const size_t bytesRead = fread(modelfile, 1, static_cast<size_t>(size), pFile);
            const bool readOk = (bytesRead == static_cast<size_t>(size)) && (ferror(pFile) == 0);

            fclose(pFile);

            if (!readOk)
            {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Short read while loading model file: " + _model);
                return false;
            }

            return true;
        }
        else
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CTfLiteClass::ReadFileToModel: Model does not exist");
            return false;
        }
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CTfLiteClass::ReadFileToModel: Can't allocate enough memory: " + std::to_string(size));
        LogFile.WriteHeapInfo("CTfLiteClass::ReadFileToModel");

        return false;
    }
}

bool CTfLiteClass::LoadModel(std::string _model)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CTfLiteClass::LoadModel");

    if (!ReadFileToModel(_model.c_str()))
    {
        return false;
    }

    model = tflite::GetModel(modelfile);

    if (model == nullptr)
    {
        return false;
    }

    return true;
}
