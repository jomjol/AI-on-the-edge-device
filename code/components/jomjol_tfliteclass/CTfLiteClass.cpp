#include "CTfLiteClass.h"
#include "ClassLogFile.h"
#include "Helper.h"

#include <sys/stat.h>

// #define DEBUG_DETAIL_ON

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

int CTfLiteClass::GetOutClassification()
{
  TfLiteTensor* output2 = interpreter->output(0);

  float zw_max = 0;
  float zw;
  int zw_class = -1;

  if (output2 == NULL)
    return -1;

  int numeroutput = output2->dims->data[1];
  for (int i = 0; i < numeroutput; ++i)
  {
    zw = output2->data.f[i];
    if (zw > zw_max)
    {
        zw_max = zw;
        zw_class = i;
    }
  }
  return zw_class;
}

void CTfLiteClass::GetInputDimension(bool silent = false)
{
  TfLiteTensor* input2 = this->interpreter->input(0);

  int numdim = input2->dims->size;
  if (!silent)  printf("NumDimension: %d\n", numdim);  

  int sizeofdim;
  for (int j = 0; j < numdim; ++j)
  {
    sizeofdim = input2->dims->data[j];
    if (!silent) printf("SizeOfDimension %d: %d\n", j, sizeofdim);  
    if (j == 1) im_height = sizeofdim;
    if (j == 2) im_width = sizeofdim;
    if (j == 3) im_channel = sizeofdim;
  }
}


void CTfLiteClass::GetOutPut()
{
  TfLiteTensor* output2 = this->interpreter->output(0);

  int numdim = output2->dims->size;
  printf("NumDimension: %d\n", numdim);  

  int sizeofdim;
  for (int j = 0; j < numdim; ++j)
  {
    sizeofdim = output2->dims->data[j];
    printf("SizeOfDimension %d: %d\n", j, sizeofdim);  
  }


  float fo;

  // Process the inference results.
  int numeroutput = output2->dims->data[1];
  for (int i = 0; i < numeroutput; ++i)
  {
   fo = output2->data.f[i];
    printf("Result %d: %f\n", i, fo);  
  }
}

void CTfLiteClass::Invoke()
{
    if (interpreter != nullptr)
      interpreter->Invoke();
}



bool CTfLiteClass::LoadInputImageBasis(CImageBasis *rs)
{
    std::string zw = "ClassFlowAnalog::doNeuralNetwork nach LoadInputResizeImage: ";

    unsigned int w = rs->width;
    unsigned int h = rs->height;
    unsigned char red, green, blue;
//    printf("Image: %s size: %d x %d\n", _fn.c_str(), w, h);

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
    LogFile.WriteToFile("Nach dem Laden in input");
#endif

    return true;
}


void CTfLiteClass::MakeAllocate()
{
    static tflite::AllOpsResolver resolver;

//    printf(LogFile.getESPHeapInfo().c_str()); printf("\n");
    this->interpreter = new tflite::MicroInterpreter(this->model, resolver, this->tensor_arena, this->kTensorArenaSize, this->error_reporter);
//    printf(LogFile.getESPHeapInfo().c_str()); printf("\n");

    TfLiteStatus allocate_status = this->interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    this->GetInputDimension();   
    return;
  }
//    printf("Allocate Done.\n");
}

void CTfLiteClass::GetInputTensorSize(){
#ifdef DEBUG_DETAIL_ON    
    float *zw = this->input;
    int test = sizeof(zw);
    printf("Input Tensor Dimension: %d\n", test);       
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
  		printf("\nFile existiert nicht.\n");
      return NULL;
    }

    unsigned char *result = (unsigned char*) malloc(size);
    int anz = 1;
    while (!result && (anz < 6))    // maximal 5x versuchen (= 5s)
    {
#ifdef DEBUG_DETAIL_ON      
		    printf("Speicher ist voll - Versuche es erneut: %d.\n", anz);
#endif
        result = (unsigned char*) malloc(size);
        anz++;
    }

  
	  if(result != NULL) {
        FILE* f = OpenFileAndWait(_fn.c_str(), "rb");     // vorher  nur "r"
        fread(result, 1, size, f);
        fclose(f);        
	  }else {
		  printf("\nKein freier Speicher vorhanden.\n");
	}    


    return result;
}

bool CTfLiteClass::LoadModel(std::string _fn){

#ifdef SUPRESS_TFLITE_ERRORS
    this->error_reporter = new tflite::OwnMicroErrorReporter;
#else
    this->error_reporter = new tflite::MicroErrorReporter;
#endif

    unsigned char *rd;
    rd = ReadFileToCharArray(_fn.c_str());

    if (rd == NULL) 
      return false;

    this->model = tflite::GetModel(rd);
    free(rd);
    TFLITE_MINIMAL_CHECK(model != nullptr); 
    
    return true;
}



CTfLiteClass::CTfLiteClass()
{
    this->model = nullptr;
    this->interpreter = nullptr;
    this->input = nullptr;
    this->output = nullptr;  
    this->kTensorArenaSize = 200 * 1024;   /// laut testfile: 108000 - bisher 600
    this->tensor_arena = new uint8_t[kTensorArenaSize]; 
}

CTfLiteClass::~CTfLiteClass()
{
  delete this->tensor_arena;
  delete this->interpreter;
  delete this->error_reporter;
}        


namespace tflite {

  int OwnMicroErrorReporter::Report(const char* format, va_list args) {
    return 0;
  }

}  


