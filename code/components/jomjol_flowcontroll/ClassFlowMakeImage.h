#pragma once

#ifndef CLASSFFLOWMAKEIMAGE_H
#define CLASSFFLOWMAKEIMAGE_H

#include "ClassFlowImage.h"
#include "ClassControllCamera.h"
#include "../../include/defines.h"

#include <string>

class ClassFlowMakeImage :
    public ClassFlowImage
{
protected:
    float waitbeforepicture;
    float waitbeforepicture_store;
    framesize_t ImageSize;
    bool isImageSize;
    int ImageQuality;
    time_t TimeImageTaken;
    string namerawimage;
    int image_height, image_width;
    bool SaveAllFiles;
    bool FixedExposure;



    void CopyFile(string input, string output);

    esp_err_t camera_capture();
    void takePictureWithFlash(int flash_duration);


    void SetInitialParameter(void);       

public:
    CImageBasis *rawImage;

    ClassFlowMakeImage(std::vector<ClassFlow*>* lfc);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    time_t getTimeImageTaken();
    string name(){return "ClassFlowMakeImage";};

    ImageData* SendRawImage();
    esp_err_t SendRawJPG(httpd_req_t *req);

    ~ClassFlowMakeImage(void);
};


#endif //CLASSFFLOWMAKEIMAGE_H