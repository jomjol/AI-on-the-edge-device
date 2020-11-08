#pragma once
#include "ClassFlow.h"
#include "ClassControllCamera.h"

#include <string>

#define BLINK_GPIO GPIO_NUM_4

#define CAMERA_MODEL_AI_THINKER



class ClassFlowMakeImage :
    public ClassFlow
{
protected:
    string LogImageLocation;
    bool isLogImage;
    float waitbeforepicture;
    framesize_t ImageSize;
    bool isImageSize;
    int ImageQuality;
    time_t TimeImageTaken;
    string namerawimage;

    void CopyFile(string input, string output);

    esp_err_t camera_capture();
    void takePictureWithFlash(int flashdauer);   

public:
    ClassFlowMakeImage();
    ClassFlowMakeImage(std::vector<ClassFlow*>* lfc);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    time_t getTimeImageTaken();
    string name(){return "ClassFlowMakeImage";};
};

