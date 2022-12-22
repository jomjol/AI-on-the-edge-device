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
    std::string zw = "ClassFlowCNNGeneral::doNeuralNetwork after LoadInputResizeImage: ";

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
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "After loading in input");
#endif

    return true;
}


void CTfLiteClass::MakeAllocate()
{
    static tflite::AllOpsResolver resolver;

//    ESP_LOGD(TAG, "%s", LogFile.getESPHeapInfo().c_str());

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Make Allocate");
    this->interpreter = new tflite::MicroInterpreter(this->model, resolver, this->tensor_arena, this->kTensorArenaSize, this->error_reporter);
//    ESP_LOGD(TAG, "%s", LogFile.getESPHeapInfo().c_str());

    TfLiteStatus allocate_status = this->interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "AllocateTensors() failed");

    this->GetInputDimension();   
    return;
  }
//    ESP_LOGD(TAG, "Allocate Done");
}

void CTfLiteClass::GetInputTensorSize(){
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


unsigned char* CTfLiteClass::ReadFileToCharArray(std::string _fn)
{
    long size;
    
    size = GetFileSize(_fn);

    if (size == -1)
    {
      ESP_LOGD(TAG, "File doesn't exist");
      return NULL;
    }

    unsigned char *result = (unsigned char*) malloc(size);
    int anz = 1;
    while (!result && (anz < 6))    // Try a maximum of 5x (= 5s)
    {
#ifdef DEBUG_DETAIL_ON      
		    ESP_LOGD(TAG, "Speicher ist voll - Versuche es erneut: %d", anz);
#endif
        result = (unsigned char*) malloc(size);
        anz++;
    }

  
	  if(result != NULL) {
        FILE* f = fopen(_fn.c_str(), "rb");     // previously only "r
        fread(result, 1, size, f);
        fclose(f);        
	  }else {
		  ESP_LOGD(TAG, "No free memory available");
	}    


    return result;
}

bool CTfLiteClass::LoadModel(std::string _fn){

#ifdef SUPRESS_TFLITE_ERRORS
    this->error_reporter = new tflite::OwnMicroErrorReporter;
#else
    this->error_reporter = new tflite::MicroErrorReporter;
#endif

    modelload = ReadFileToCharArray(_fn.c_str());

    if (modelload == NULL) 
      return false;

    model = tflite::GetModel(modelload);
//    free(rd);
    TFLITE_MINIMAL_CHECK(model != nullptr); 
    
    return true;
}



CTfLiteClass::CTfLiteClass()
{
    this->model = nullptr;
    this->interpreter = nullptr;
    this->input = nullptr;
    this->output = nullptr;  
    this->kTensorArenaSize = 800 * 1024;   /// according to testfile: 108000 - so far 600;; 2021-09-11: 200 * 1024
    this->tensor_arena = new uint8_t[kTensorArenaSize]; 
}

CTfLiteClass::~CTfLiteClass()
{
  delete this->tensor_arena;
  delete this->interpreter;
  delete this->error_reporter;
  
  free(modelload);
}        


namespace tflite {

  int OwnMicroErrorReporter::Report(const char* format, va_list args) {
    return 0;
  }

}  


