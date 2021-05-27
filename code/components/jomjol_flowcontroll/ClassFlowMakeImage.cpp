#include "ClassFlowMakeImage.h"
#include "Helper.h"
#include "ClassLogFile.h"

#include "CImageBasis.h"
#include "ClassControllCamera.h"

#include <time.h>

// #define DEBUG_DETAIL_ON 

static const char* TAG = "flow_make_image";

esp_err_t ClassFlowMakeImage::camera_capture(){
    string nm =  namerawimage;
    Camera.CaptureToFile(nm);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return ESP_OK;
}

void ClassFlowMakeImage::takePictureWithFlash(int flashdauer)
{
    // für den Fall, dass das Bild geflippt wird, muss es hier zurück gesetzt werden ////
    rawImage->width = image_width;          
    rawImage->height = image_height;
    /////////////////////////////////////////////////////////////////////////////////////
    printf("Flashdauer: %d\n", flashdauer);
    Camera.CaptureToBasisImage(rawImage, flashdauer);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    if (SaveAllFiles) rawImage->SaveToFile(namerawimage);
}

void ClassFlowMakeImage::SetInitialParameter(void)
{
    waitbeforepicture = 5;
    isImageSize = false;
    ImageQuality = -1;    
    TimeImageTaken = 0;
    ImageQuality = 5;
    rawImage = NULL;
    ImageSize = FRAMESIZE_VGA;
    SaveAllFiles = false;
    disabled = false;
    FixedExposure = false;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
}     


ClassFlowMakeImage::ClassFlowMakeImage(std::vector<ClassFlow*>* lfc) : ClassFlowImage(lfc, TAG)
{
    SetInitialParameter();
}

bool ClassFlowMakeImage::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);
    int _brightness = -100;
    int _contrast = -100;
    int _saturation = -100;

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
            ImageQuality = std::stod(zerlegt[1]);

        if ((zerlegt[0] == "ImageSize") && (zerlegt.size() > 1))
        {
            ImageSize = Camera.TextToFramesize(zerlegt[1].c_str());
            isImageSize = true;
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }
        
        if ((toUpper(zerlegt[0]) == "WAITBEFORETAKINGPICTURE") && (zerlegt.size() > 1))
        {
            waitbeforepicture = stoi(zerlegt[1]);
        }


        if ((toUpper(zerlegt[0]) == "BRIGHTNESS") && (zerlegt.size() > 1))
        {
            _brightness = stoi(zerlegt[1]);
        }

        if ((toUpper(zerlegt[0]) == "CONTRAST") && (zerlegt.size() > 1))
        {
            _contrast = stoi(zerlegt[1]);
        }

        if ((toUpper(zerlegt[0]) == "SATURATION") && (zerlegt.size() > 1))
        {
            _saturation = stoi(zerlegt[1]);
        }

        if ((toUpper(zerlegt[0]) == "FIXEDEXPOSURE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                FixedExposure = true;
        }
    }

    Camera.SetBrightnessContrastSaturation(_brightness, _contrast, _saturation);
    Camera.SetQualitySize(ImageQuality, ImageSize);

    image_width = Camera.image_width;
    image_height = Camera.image_height;
    rawImage = new CImageBasis();
    rawImage->CreateEmptyImage(image_width, image_height, 3);

    waitbeforepicture_store = waitbeforepicture;
    if (FixedExposure && (waitbeforepicture > 0))
    {
//        printf("Fixed Exposure enabled!\n");
        int flashdauer = (int) (waitbeforepicture * 1000);
        Camera.EnableAutoExposure(flashdauer);
        waitbeforepicture = 0.2;
//        flashdauer = (int) (waitbeforepicture * 1000);
//        takePictureWithFlash(flashdauer);
//        rawImage->SaveToFile("/sdcard/init2.jpg");
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
    string logPath = CreateLogFolder(zwtime);

    int flashdauer = (int) (waitbeforepicture * 1000);
 
 #ifdef DEBUG_DETAIL_ON  
    LogFile.WriteHeapInfo("ClassFlowMakeImage::doFlow - Before takePictureWithFlash");
#endif

    takePictureWithFlash(flashdauer);

#ifdef DEBUG_DETAIL_ON  
    LogFile.WriteHeapInfo("ClassFlowMakeImage::doFlow - After takePictureWithFlash");
#endif

    LogImage(logPath, "raw", NULL, NULL, zwtime, rawImage);

    RemoveOldLogs();

#ifdef DEBUG_DETAIL_ON  
    LogFile.WriteHeapInfo("ClassFlowMakeImage::doFlow - After RemoveOldLogs");
#endif

    return true;
}

esp_err_t ClassFlowMakeImage::SendRawJPG(httpd_req_t *req)
{
    int flashdauer = (int) (waitbeforepicture * 1000);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return Camera.CaptureToHTTP(req, flashdauer);
}


ImageData* ClassFlowMakeImage::SendRawImage()
{
    CImageBasis *zw = new CImageBasis(rawImage);
    ImageData *id;
    int flashdauer = (int) (waitbeforepicture * 1000);
    Camera.CaptureToBasisImage(zw, flashdauer);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    id = zw->writeToMemoryAsJPG();    
    delete zw;
    return id;  
}

time_t ClassFlowMakeImage::getTimeImageTaken()
{
    return TimeImageTaken;
}

ClassFlowMakeImage::~ClassFlowMakeImage(void)
{
    delete rawImage;
}

