#include "ClassFlowMakeImage.h"
#include "Helper.h"

#include "CFindTemplate.h"
#include "ClassControllCamera.h"

#include <time.h>

static const char* TAG = "flow_make_image";

esp_err_t ClassFlowMakeImage::camera_capture(){
    string nm =  namerawimage;
    Camera.CaptureToFile(nm);
    return ESP_OK;
}

void ClassFlowMakeImage::takePictureWithFlash(int flashdauer)
{
    string nm = namerawimage;
    if (isImageSize && (ImageQuality > 0))
        Camera.SetQualitySize(ImageQuality, ImageSize);
    printf("Start CaptureFile\n");
    Camera.CaptureToFile(nm, flashdauer);
}


ClassFlowMakeImage::ClassFlowMakeImage() : ClassFlowImage(TAG)
{
    waitbeforepicture = 5;
    isImageSize = false;
    ImageQuality = -1;    
    TimeImageTaken = 0;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
}

ClassFlowMakeImage::ClassFlowMakeImage(std::vector<ClassFlow*>* lfc) : ClassFlowImage(lfc, TAG)
{
    waitbeforepicture = 5;
    isImageSize = false;
    ImageQuality = -1;
    TimeImageTaken = 0;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
}

bool ClassFlowMakeImage::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (aktparamgraph.compare("[MakeImage]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] ==  "LogImageLocation") && (zerlegt.size() > 1))
        {
            LogImageLocation = "/sdcard" + zerlegt[1];
            isLogImage = true;
        }
        if ((zerlegt[0] == "ImageQuality") && (zerlegt.size() > 1))
            this->ImageQuality = std::stod(zerlegt[1]);
        if ((zerlegt[0] == "ImageSize") && (zerlegt.size() > 1))
        {
            ImageSize = Camera.TextToFramesize(zerlegt[1].c_str());
            isImageSize = true;
        }
    }
   
    return true;
}

string ClassFlowMakeImage::getHTMLSingleStep(string host)
{
    string result;
    result = "Raw Image: <br>\n<img src=\"" + host + "/img_tmp/raw.jpg\">\n";
    return result;
}

bool ClassFlowMakeImage::doFlow(string zwtime)
{
    ////////////////////////////////////////////////////////////////////
    // TakeImage and Store into /image_tmp/raw.jpg  TO BE DONE
    ////////////////////////////////////////////////////////////////////

    string logPath = CreateLogFolder(zwtime);

    int flashdauer = (int) waitbeforepicture * 1000;
    

    takePictureWithFlash(flashdauer);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    LogImage(logPath, "raw", NULL, NULL, zwtime);

    RemoveOldLogs();

    return true;
}

time_t ClassFlowMakeImage::getTimeImageTaken()
{
    return TimeImageTaken;
}
