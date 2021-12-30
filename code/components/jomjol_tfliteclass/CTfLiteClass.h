
#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x)) {                                                \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
//#include "tensorflow/lite/version.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "esp_err.h"
#include "esp_log.h"

#include "CImageBasis.h"



#define SUPRESS_TFLITE_ERRORS           // use, to avoid error messages from TFLITE

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
        static tflite::AllOpsResolver resolver;

        int kTensorArenaSize;
        uint8_t *tensor_arena;

        unsigned char *modelload = NULL;


        float* input;
        int input_i;
        int im_height, im_width, im_channel;

        long GetFileSize(std::string filename);
        unsigned char* ReadFileToCharArray(std::string _fn);
        
    public:
        CTfLiteClass();
        ~CTfLiteClass();        
        bool LoadModel(std::string _fn);
        void MakeAllocate();
        void GetInputTensorSize();
        bool LoadInputImageBasis(CImageBasis *rs);
        void Invoke();
        int GetAnzOutPut(bool silent = true);        
//        void GetOutPut();
//        int GetOutClassification();
        int GetOutClassification(int _von = -1, int _bis = -1);

        int GetClassFromImageBasis(CImageBasis *rs);
        std::string GetStatusFlow();

        float GetOutputValue(int nr);
        void GetInputDimension(bool silent);
};

