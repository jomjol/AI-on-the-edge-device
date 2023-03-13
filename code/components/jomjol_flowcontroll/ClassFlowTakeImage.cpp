#include "ClassFlowTakeImage.h"
#include "Helper.h"
#include "ClassLogFile.h"

#include "CImageBasis.h"
#include "ClassControllCamera.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "../../include/defines.h"

#include <time.h>

// #define DEBUG_DETAIL_ON 

static const char* TAG = "TAKEIMAGE";


void ClassFlowTakeImage::SetInitialParameter(void)
{
    PresetFlowStateHandler(true);
    waitbeforepicture = 5.0; // Flash duration in s
    flash_duration = (int)(waitbeforepicture * 1000);   // Flash duration in ms
    isImageSize = false;
    ImageSize = FRAMESIZE_VGA;
    TimeImageTaken = 0;
    ImageQuality = 12;
    SaveAllFiles = false;
    disabled = false;
    FixedExposure = false;
    rawImage = NULL;
    namerawimage = "/sdcard/img_tmp/raw.jpg";
}     


ClassFlowTakeImage::ClassFlowTakeImage(std::vector<ClassFlow*>* lfc) : ClassFlowImage(lfc, TAG)
{
    SetInitialParameter();
    // Init of ClassFlowImage variables --> ClassFlowImage.cpp
}


bool ClassFlowTakeImage::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;
    int _brightness = 0;
    int _contrast = 0;
    int _saturation = 0;
    int ledintensity = 50;

    aktparamgraph = trim(aktparamgraph);
    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (aktparamgraph.compare("[TakeImage]") != 0)       // Paragraph does not fit TakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);
        if ((toUpper(splitted[0]) ==  "RAWIMAGESLOCATION") && (splitted.size() > 1))
        {
            imagesLocation = "/sdcard" + splitted[1];
            isLogImage = true;
        }
        if ((toUpper(splitted[0]) == "IMAGEQUALITY") && (splitted.size() > 1))
            ImageQuality = std::stod(splitted[1]);

        if ((toUpper(splitted[0]) == "IMAGESIZE") && (splitted.size() > 1))
        {
            ImageSize = Camera.TextToFramesize(splitted[1].c_str());
            isImageSize = true;
        }

        if ((toUpper(splitted[0]) == "SAVEALLFILES") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                SaveAllFiles = true;
            else
                SaveAllFiles = false;
        }
        
        if ((toUpper(splitted[0]) == "WAITBEFORETAKINGPICTURE") && (splitted.size() > 1))
        {
            waitbeforepicture = stof(splitted[1]);
            flash_duration = (int)(waitbeforepicture * 1000);
        }

        if ((toUpper(splitted[0]) == "RAWIMAGESRETENTION") && (splitted.size() > 1))
        {
            this->imagesRetention = std::stoi(splitted[1]);
        }

        if ((toUpper(splitted[0]) == "BRIGHTNESS") && (splitted.size() > 1))
        {
            _brightness = stoi(splitted[1]);
        }

        if ((toUpper(splitted[0]) == "CONTRAST") && (splitted.size() > 1))
        {
            _contrast = stoi(splitted[1]);
        }

        if ((toUpper(splitted[0]) == "SATURATION") && (splitted.size() > 1))
        {
            _saturation = stoi(splitted[1]);
        }

        if ((toUpper(splitted[0]) == "FIXEDEXPOSURE") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                FixedExposure = true;
            else
                FixedExposure = false;
        }

        if ((toUpper(splitted[0]) == "LEDINTENSITY") && (splitted.size() > 1))
        {
            ledintensity = stoi(splitted[1]);
            //checkMinMax(&ledintensity, 0, 100);
            //ESP_LOGI(TAG, "ledintensity: %d", ledintensity);
            ledintensity = min(100, ledintensity);
            ledintensity = max(0, ledintensity);
            //Camera.SetLEDIntensity(ledintensity);
        }

        if ((toUpper(splitted[0]) == "DEMO") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                Camera.useDemoMode();
        }
    }

    Camera.ledc_init(); // PWM init needs to be repeated after online config update
    Camera.SetLEDIntensity(ledintensity);
    Camera.SetBrightnessContrastSaturation(_brightness, _contrast, _saturation);
    Camera.SetQualitySize(ImageQuality, ImageSize);

    image_width = Camera.image_width;
    image_height = Camera.image_height;
    rawImage = new CImageBasis();
    if (rawImage) {
        if(!rawImage->CreateEmptyImage(image_width, image_height, 3)) {
            return false;
        }
    }
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "ReadParameter: Can't create CImageBasis for rawImage");
        return false;
    }

    if (FixedExposure && (waitbeforepicture > 0)) {
//      ESP_LOGD(TAG, "Fixed Exposure enabled!");
        Camera.EnableAutoExposure(flash_duration);
    }

    return true;
}


bool ClassFlowTakeImage::doFlow(string zwtime)
{

    PresetFlowStateHandler();
    std::string logPath = CreateLogFolder(zwtime);
 
    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("ClassFlowTakeImage::doFlow - Start");
    #endif

    if (takePictureWithFlash(flash_duration)) {
        FlowStateHandlerSetError(-1);       // Error cluster: -1
        return false;
    }

    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("ClassFlowTakeImage::doFlow - After takePictureWithFlash");
    #endif

    LogImage(logPath, "raw", NULL, NULL, zwtime, rawImage);

    RemoveOldLogs();

    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("ClassFlowTakeImage::doFlow - Done");
    #endif

    return true;
}


void ClassFlowTakeImage::doAutoErrorHandling()
{
    // Error handling can be included here. Function is called after round is completed.
    //ESP_LOGI(TAG, "ClassFlowTakeImage::doAutoErrorHandling() - TEST");
}


string ClassFlowTakeImage::getHTMLSingleStep(string host)
{
    string result = "Raw Image: <br>\n<img src=\"" + host + "/img_tmp/raw.jpg\">\n";
    return result;
}


esp_err_t ClassFlowTakeImage::camera_capture()
{
    std::string nm =  namerawimage;

    if (!Camera.CaptureToFile(nm))
        return ESP_FAIL;

    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return ESP_OK;
}


bool ClassFlowTakeImage::takePictureWithFlash(int _flash_duration)
{
    if (rawImage == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "takePictureWithFlash: rawImage not initialized");
        return false;
    }

    // in case the image is flipped, it must be reset here //
    rawImage->width = image_width;          
    rawImage->height = image_height;

    ESP_LOGD(TAG, "flash_duration: %d", _flash_duration);
    if (!Camera.CaptureToBasisImage(rawImage, _flash_duration))
        return false;

    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    if (SaveAllFiles)
        rawImage->SaveToFile(namerawimage);

    return true;
}


esp_err_t ClassFlowTakeImage::SendRawJPG(httpd_req_t *req)
{
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return Camera.CaptureToHTTP(req, flash_duration);
}


ImageData* ClassFlowTakeImage::SendRawImage()
{
    CImageBasis *zw = new CImageBasis(rawImage);
    ImageData *id;

    if (!Camera.CaptureToBasisImage(rawImage, flash_duration))
        return NULL;

    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    id = zw->writeToMemoryAsJPG();    
    delete zw;
    return id;  
}


time_t ClassFlowTakeImage::getTimeImageTaken()
{
    return TimeImageTaken;
}


std::string ClassFlowTakeImage::getFileNameRawImage(void)
{
    return namerawimage;
}


ClassFlowTakeImage::~ClassFlowTakeImage()
{
    delete rawImage;
}
