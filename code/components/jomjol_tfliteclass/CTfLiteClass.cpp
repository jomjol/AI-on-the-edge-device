#include "CTfLiteClass.h"
#include "ClassLogFile.h"
#include "Helper.h"
#include "esp_log.h"
#include "../../include/defines.h"

#include <sys/stat.h>

// #define DEBUG_DETAIL_ON


static const char *TAG = "TFLITE";

float CTfLiteClass::GetOutputValue(int nr)
{
    TfLiteTensor* output2 = this->interpreter->output(0);

    if (output2 == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "GetOutputValue failed");
        return -1000;
    }
        
    int numeroutput = output2->dims->data[1];
    if ((nr+1) > numeroutput)
        return -1000;
    else
        return output2->data.f[nr];

}


int CTfLiteClass::GetClassFromImageBasis(CImageBasis *rs)
{
    if (!LoadInputImageBasis(rs))
        return -1000;

    Invoke();

    return GetOutClassification();
}


int CTfLiteClass::GetOutClassification(int _von, int _bis)
{
    TfLiteTensor* output2 = interpreter->output(0);

    if (output2 == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "GetOutClassification failed");
        return -1;
    }

    float zw_max;
    float zw;
    int zw_class;

    int numeroutput = output2->dims->data[1];
    //ESP_LOGD(TAG, "number output neurons: %d", numeroutput);

    if (_bis == -1)
        _bis = numeroutput -1;

    if (_von == -1)
        _von = 0;

    if (_bis >= numeroutput)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "GetOutClassification: NUMBER OF OUTPUT NEURONS does not match required classification!");
        return -1;
    }

    zw_max = output2->data.f[_von];
    zw_class = _von;
    for (int i = _von + 1; i <= _bis; ++i)
    {
        zw = output2->data.f[i];
        if (zw > zw_max)
        {
            zw_max = zw;
            zw_class = i;
        }
    }
    return (zw_class - _von);
}


bool CTfLiteClass::GetInputDimension(bool silent = false)
{
  TfLiteTensor* input2 = this->interpreter->input(0);

    if (input2 == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "GetInputDimension failed");
        return false;
    }

    int numdim = input2->dims->size;
    if (!silent)  ESP_LOGD(TAG, "NumDimension: %d", numdim);

    int sizeofdim;
    for (int j = 0; j < numdim; ++j)
    {
      sizeofdim = input2->dims->data[j];
      if (!silent) ESP_LOGD(TAG, "SizeOfDimension %d: %d", j, sizeofdim);
      if (j == 1) im_height = sizeofdim;
      if (j == 2) im_width = sizeofdim;
      if (j == 3) im_channel = sizeofdim;
    }

    return true;
}


int CTfLiteClass::ReadInputDimenstion(int _dim)
{
    if (_dim == 0)
        return im_width;
    if (_dim == 1)
        return im_height;
    if (_dim == 2)
        return im_channel;

    return -1;
}


int CTfLiteClass::GetAnzOutPut(bool silent)
{
  TfLiteTensor* output2 = this->interpreter->output(0);

    if (output2 == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "GetAnzOutPut failed");
        return -1;
    }

    int numdim = output2->dims->size;
    if (!silent) ESP_LOGD(TAG, "NumDimension: %d", numdim);

    int sizeofdim;
    for (int j = 0; j < numdim; ++j)
    {
        sizeofdim = output2->dims->data[j];
        if (!silent) ESP_LOGD(TAG, "SizeOfDimension %d: %d", j, sizeofdim);
    }

    float fo;
    // Process the inference results.
    int numeroutput = output2->dims->data[1];
    for (int i = 0; i < numeroutput; ++i)
    {
        fo = output2->data.f[i];
        if (!silent) ESP_LOGD(TAG, "Result %d: %f", i, fo);
    }
    return numeroutput;
}


void CTfLiteClass::Invoke()
{
    if (interpreter != nullptr)
      interpreter->Invoke();
}


bool CTfLiteClass::LoadInputImageBasis(CImageBasis *rs)
{
    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("LoadInputImageBasis - Start");
    #endif

    if (rs == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadInputImageBasis: No image data");
        return false;
    }

    unsigned int w = rs->width;
    unsigned int h = rs->height;
    unsigned char red, green, blue;
//    ESP_LOGD(TAG, "Image: %s size: %d x %d\n", _fn.c_str(), w, h);

    input_i = 0;
    float* input_data_ptr = (interpreter->input(0))->data.f;

    if (input_data_ptr == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadInputImageBasis: No input data");
        return false;
    }

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            {
                red = rs->GetPixelColor(x, y, 0);
                green = rs->GetPixelColor(x, y, 1);
                blue = rs->GetPixelColor(x, y, 2);
                *(input_data_ptr) = (float) red;
                input_data_ptr++;
                *(input_data_ptr) = (float) green;
                input_data_ptr++;
                *(input_data_ptr) = (float) blue;
                input_data_ptr++;
            }

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("LoadInputImageBasis - done");
    #endif

    return true;
}


bool CTfLiteClass::MakeAllocate()
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating tensors");
    static tflite::AllOpsResolver resolver;

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CTLiteClass::Alloc start");
    #endif

    this->interpreter = new tflite::MicroInterpreter(this->model, resolver, this->tensor_arena, this->kTensorArenaSize, this->error_reporter);

    if (this->interpreter == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "new tflite::MicroInterpreter failed");
        LogFile.WriteHeapInfo("MakeAllocate-new tflite::MicroInterpreter failed");
        return false;
    }

    TfLiteStatus allocate_status = this->interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        //TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed");
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Allocate tensors failed");

        //this->GetInputDimension();   // TODO: Why read again when already failed state?
        return false;
    }

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CTLiteClass::Alloc done");
    #endif

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Tensors successfully allocated");
    return true;
}


void CTfLiteClass::GetInputTensorSize()
{
#ifdef DEBUG_DETAIL_ON    
    float *zw = this->input;
    int test = sizeof(zw);
    ESP_LOGD(TAG, "Input Tensor Dimension: %d", test);
#endif
}


long CTfLiteClass::GetFileSize(std::string filename)
{
    struct stat stat_buf;
    long rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


bool CTfLiteClass::ReadFileToModel(std::string _fn)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Read TFLITE model file: " + _fn);
    
    #ifdef DEBUG_DETAIL_ON      
            LogFile.WriteHeapInfo("ReadFileToModel: start");
    #endif

    long size = GetFileSize(_fn);
    if (size <= 0) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "File not existing or zero size: " + _fn);
        return false;
    }

    modelfile = (unsigned char*)GET_MEMORY(size);
  
	  if(modelfile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ReadFileToModel: Can't allocate enough memory: " + std::to_string(size));
        LogFile.WriteHeapInfo("ReadFileToModel: Allocation failed");
        return false;
    }

    FILE* f = fopen(_fn.c_str(), "rb");     // previously only "r
    if (fread(modelfile, 1, size, f) != size) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ReadFileToModel: Reading error: Size differs!");
        free(modelfile);
        fclose(f);
        return false;
    }
    fclose(f);     

    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("ReadFileToModel: done");
    #endif

    return true;   
}


bool CTfLiteClass::LoadModel(std::string _fn)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Loading TFLITE model");
    
    #ifdef SUPRESS_TFLITE_ERRORS
        this->error_reporter = new tflite::OwnMicroErrorReporter;
    #else
        this->error_reporter = new tflite::MicroErrorReporter;
    #endif

    if (!ReadFileToModel(_fn)) {
      LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadModel: TFLITE model file reading failed!");
      return false;
    }

    model = tflite::GetModel(modelfile);

    if(model == nullptr) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadModel: GetModel failed!");
        return false;
    }

    /*if (model->version() != TFLITE_SCHEMA_VERSION) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "LoadModel: Model provided is schema version " + std::to_string(model->version()) +
                                                " not euqal to supported version " + std::to_string(TFLITE_SCHEMA_VERSION));
        return false;
    }*/

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "TFLITE model successfully loaded");
    return true;
}


CTfLiteClass::CTfLiteClass()
{
    this->model = nullptr;
    this->modelfile = NULL;
    this->interpreter = nullptr;
    this->input = nullptr;
    this->output = nullptr;  
    this->kTensorArenaSize = 800 * 1024;   /// according to testfile: 108000 - so far 600;; 2021-09-11: 200 * 1024
    this->tensor_arena = (uint8_t*)GET_MEMORY(kTensorArenaSize);
}


CTfLiteClass::~CTfLiteClass()
{
  free(modelfile);

  free(this->tensor_arena);
  delete this->interpreter;
  delete this->error_reporter;
}        


namespace tflite 
{
  int OwnMicroErrorReporter::Report(const char* format, va_list args) 
  {
    return 0;
  }
}  
