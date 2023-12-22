#include "CTfLiteClass.h"
#include "ClassLogFile.h"
#include "Helper.h"
#include "psram.h"
#include "esp_log.h"
#include "../../include/defines.h"

#include <sys/stat.h>

// #define DEBUG_DETAIL_ON


static const char *TAG = "TFLITE";

/// Static Resolver muss mit allen Operatoren geladen Werden, die benöägit werden - ABER nur 1x --> gesonderte Funktion /////////////////////////////
static bool MakeStaticResolverDone = false;
static tflite::MicroMutableOpResolver<15> resolver;

void MakeStaticResolver()
{
  if (MakeStaticResolverDone)
    return;

  MakeStaticResolverDone = true;

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
////////////////////////////////////////////////////////////////////////////////////////


float CTfLiteClass::GetOutputValue(int nr)
{
    TfLiteTensor* output2 = this->interpreter->output(0);

    int numeroutput = output2->dims->data[1];
    if ((nr+1) > numeroutput)
      return -1000;

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

  float zw_max;
  float zw;
  int zw_class;

  if (output2 == NULL)
    return -1;

  int numeroutput = output2->dims->data[1];
  //ESP_LOGD(TAG, "number output neurons: %d", numeroutput);

  if (_bis == -1)
    _bis = numeroutput -1;

  if (_von == -1)
    _von = 0;

  if (_bis >= numeroutput)
  {
    ESP_LOGD(TAG, "NUMBER OF OUTPUT NEURONS does not match required classification!");
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


void CTfLiteClass::GetInputDimension(bool silent = false)
{
  TfLiteTensor* input2 = this->interpreter->input(0);

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
        LogFile.WriteHeapInfo("CTfLiteClass::LoadInputImageBasis - Start");
    #endif

    unsigned int w = rs->width;
    unsigned int h = rs->height;
    unsigned char red, green, blue;
//    ESP_LOGD(TAG, "Image: %s size: %d x %d\n", _fn.c_str(), w, h);

    input_i = 0;
    float* input_data_ptr = (interpreter->input(0))->data.f;

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
        LogFile.WriteHeapInfo("CTfLiteClass::LoadInputImageBasis - done");
    #endif

    return true;
}



bool CTfLiteClass::MakeAllocate()
{

  MakeStaticResolver();


    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CTLiteClass::Alloc start");
    #endif

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CTfLiteClass::MakeAllocate");
    this->interpreter = new tflite::MicroInterpreter(this->model, resolver, this->tensor_arena, this->kTensorArenaSize);
//    this->interpreter = new tflite::MicroInterpreter(this->model, resolver, this->tensor_arena, this->kTensorArenaSize, this->error_reporter);

    if (this->interpreter) 
    {
        TfLiteStatus allocate_status = this->interpreter->AllocateTensors();
        if (allocate_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "AllocateTensors() failed");

            this->GetInputDimension();   
            return false;
        }
    }
    else 
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "new tflite::MicroInterpreter failed");
        LogFile.WriteHeapInfo("CTfLiteClass::MakeAllocate-new tflite::MicroInterpreter failed");
        return false;
    }


    #ifdef DEBUG_DETAIL_ON 
        LogFile.WriteHeapInfo("CTLiteClass::Alloc done");
    #endif

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
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CTfLiteClass::ReadFileToModel: " + _fn);
    
    long size = GetFileSize(_fn);

    if (size == -1)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Model file doesn't exist: " + _fn + "!");
        return false;
    }
    else if(size > MAX_MODEL_SIZE) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Unable to load model '" + _fn + "'! It does not fit in the reserved shared memory in PSRAM!");
        return false;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Loading Model " + _fn + " /size: " + std::to_string(size) + " bytes...");


#ifdef DEBUG_DETAIL_ON      
        LogFile.WriteHeapInfo("CTLiteClass::Alloc modelfile start");
#endif

    modelfile = (unsigned char*)psram_get_shared_model_memory();
  
	  if(modelfile != NULL) 
    {
        FILE* f = fopen(_fn.c_str(), "rb");     // previously only "r
        fread(modelfile, 1, size, f);
        fclose(f);        

        #ifdef DEBUG_DETAIL_ON 
            LogFile.WriteHeapInfo("CTLiteClass::Alloc modelfile successful");
        #endif

        return true;    
	}    
    else 
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "CTfLiteClass::ReadFileToModel: Can't allocate enough memory: " + std::to_string(size));
        LogFile.WriteHeapInfo("CTfLiteClass::ReadFileToModel");

        return false;
    }
}


bool CTfLiteClass::LoadModel(std::string _fn)
{
#ifdef SUPRESS_TFLITE_ERRORS
//    this->error_reporter = new tflite::ErrorReporter;
    this->error_reporter = new tflite::OwnMicroErrorReporter;
#else
    this->error_reporter = new tflite::MicroErrorReporter;
#endif

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CTfLiteClass::LoadModel");

    if (!ReadFileToModel(_fn.c_str())) {
      return false;
    }

    model = tflite::GetModel(modelfile);

    if(model == nullptr)     
      return false;
    
    return true;
}


CTfLiteClass::CTfLiteClass()
{
    this->model = nullptr;
    this->modelfile = NULL;
    this->interpreter = nullptr;
    this->input = nullptr;
    this->output = nullptr;
    this->kTensorArenaSize = TENSOR_ARENA_SIZE;
    this->tensor_arena = (uint8_t*)psram_get_shared_tensor_arena_memory();
}


CTfLiteClass::~CTfLiteClass()
{
  delete this->interpreter;
//  delete this->error_reporter;

  psram_free_shared_tensor_arena_and_model_memory();
}        

#ifdef SUPRESS_TFLITE_ERRORS
namespace tflite 
{
//tflite::ErrorReporter
//  int OwnMicroErrorReporter::Report(const char* format, va_list args) 

  int OwnMicroErrorReporter::Report(const char* format, va_list args) 
  {
    return 0;
  }
} 
#endif
 
