#include "CTfLiteClass.h"

#include "bitmap_image.hpp"

#include <sys/stat.h>

float CTfLiteClass::GetOutputValue(int nr)
{
    TfLiteTensor* output2 = this->interpreter->output(0);

    int numeroutput = output2->dims->data[1];
    if ((nr+1) > numeroutput)
      return -1000;

    return output2->data.f[nr];
}


int CTfLiteClass::GetClassFromImage(std::string _fn)
{
//  printf("Before Load image %s\n", _fn.c_str());
    if (!LoadInputImage(_fn))
      return -1000;
//  printf("After Load image %s\n", _fn.c_str());

    Invoke();
  printf("After Invoke %s\n", _fn.c_str());

    return GetOutClassification();
//    return 0;
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
//  printf("Result Ziffer: %d\n", zw_class);       
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
    interpreter->Invoke();
//    printf("Invoke Done.\n");
}


bool CTfLiteClass::LoadInputImage(std::string _fn)
{
    bitmap_image image(_fn);
    unsigned int w = image.width();
    unsigned int h = image.height();
    unsigned char red, green, blue;

    input_i = 0;
    float* input_data_ptr = (interpreter->input(0))->data.f;

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            {
                red = image.red_channel(x, y);
                green = image.green_channel(x, y);
                blue = image.blue_channel(x, y);
                *(input_data_ptr) = (float) red;
                input_data_ptr++;
                *(input_data_ptr) = (float) green;
                input_data_ptr++;
                *(input_data_ptr) = (float) blue;
                input_data_ptr++;

//                printf("BMP: %f %f %f\n", (float) red, (float) green, (float) blue);

            }
    return true;
}


void CTfLiteClass::MakeAllocate()
{
/*    
  this->micro_op_resolver.AddBuiltin(
                                tflite::BuiltinOperator_RESHAPE,
                                tflite::ops::micro::Register_RESHAPE());
  this->micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
  this->micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());
  this->micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());
  this->micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
                               tflite::ops::micro::Register_DEPTHWISE_CONV_2D());


    this->interpreter = new tflite::MicroInterpreter(this->model, this->micro_op_resolver, this->tensor_arena, this->kTensorArenaSize, this->error_reporter);
*/


    static tflite::ops::micro::AllOpsResolver resolver;
    this->interpreter = new tflite::MicroInterpreter(this->model, resolver, this->tensor_arena, this->kTensorArenaSize, this->error_reporter);

    TfLiteStatus allocate_status = this->interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    this->GetInputDimension();   
    return;
  }

    printf("Allocate Done.\n");
}

void CTfLiteClass::GetInputTensorSize(){
    float *zw = this->input;
    int test = sizeof(zw);
    printf("Input Tensor Dimension: %d\n", test);       

    printf("Input Tensor Dimension: %d\n", test);   
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
    
    size = this->GetFileSize(_fn);

    if (size == -1)
    {
		printf("\nFile existiert nicht.\n");
        return NULL;
    }


    unsigned char *result = (unsigned char*) malloc(size);
  
	if(result != NULL) {
//		printf("\nSpeicher ist reserviert\n");
        FILE* f = fopen(_fn.c_str(), "rb");     // vorher  nur "r"
        fread(result, 1, size, f);
        fclose(f);        
	}else {
		printf("\nKein freier Speicher vorhanden.\n");
	}    


    return result;
}

void CTfLiteClass::LoadModel(std::string _fn){


    this->error_reporter = new tflite::MicroErrorReporter;

    unsigned char *rd;
    rd = this->ReadFileToCharArray(_fn.c_str());
//    printf("loadedfile: %d", (int) rd);

    this->model = tflite::GetModel(rd);
    free(rd);
    TFLITE_MINIMAL_CHECK(model != nullptr); 
    printf("tfile Loaded.\n");  

}



CTfLiteClass::CTfLiteClass()
{
//    this->accessSD = _accessSD;
    this->model = nullptr;
    this->interpreter = nullptr;
    this->input = nullptr;
    this->output = nullptr;  
    this->kTensorArenaSize = 600 * 1024;
    this->tensor_arena = new uint8_t[kTensorArenaSize];    

//      micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
//                               tflite::ops::micro::Register_CONV_2D());
}

CTfLiteClass::~CTfLiteClass()
{
  delete this->tensor_arena;
}        


