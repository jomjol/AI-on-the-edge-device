#pragma once

#ifndef CLASSFFLOWTAKEIMAGE_H
#define CLASSFFLOWTAKEIMAGE_H

#include "ClassFlowImage.h"
#include "ClassControllCamera.h"
#include "../../include/defines.h"

#include <string>

class ClassFlowTakeImage :
    public ClassFlowImage
{
protected:
    float waitbeforepicture;
    float waitbeforepicture_store;
    framesize_t ImageSize;
    bool isImageSize;
    bool ZoomEnabled = false;
    int ZoomMode = 0;
    int zoomOffsetX = 0;
    int zoomOffsetY = 0;
    bool ImageGrayscale;
    bool ImageNegative;
    bool ImageAec2;
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

    ClassFlowTakeImage(std::vector<ClassFlow*>* lfc);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    time_t getTimeImageTaken();
    string name(){return "ClassFlowTakeImage";};

    ImageData* SendRawImage();
    esp_err_t SendRawJPG(httpd_req_t *req);

    ~ClassFlowTakeImage(void);
};


#endif //CLASSFFLOWTAKEIMAGE_H