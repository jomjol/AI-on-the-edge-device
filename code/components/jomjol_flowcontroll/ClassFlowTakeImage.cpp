#include "ClassFlowTakeImage.h"
#include "Helper.h"
#include "ClassLogFile.h"

#include "CImageBasis.h"
#include "ClassControllCamera.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "../../include/defines.h"
#include "psram.h"

#include <time.h>

// #define DEBUG_DETAIL_ON 

// #define WIFITURNOFF

static const char* TAG = "TAKEIMAGE";

esp_err_t ClassFlowTakeImage::camera_capture(){
    string nm =  namerawimage;
    Camera.CaptureToFile(nm);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return ESP_OK;
}

void ClassFlowTakeImage::takePictureWithFlash(int flash_duration)
{
    // in case the image is flipped, it must be reset here //
    rawImage->width = image_width;          
    rawImage->height = image_height;
    /////////////////////////////////////////////////////////////////////////////////////
    ESP_LOGD(TAG, "flash_duration: %d", flash_duration);
    Camera.CaptureToBasisImage(rawImage, flash_duration);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    if (SaveAllFiles) rawImage->SaveToFile(namerawimage);
}

void ClassFlowTakeImage::SetInitialParameter(void)
{
    waitbeforepicture = 5;
    isImageSize = false;
    ImageQuality = -1;    
    TimeImageTaken = 0;
    ImageQuality = 5;
    rawImage = NULL;
    ImageSize = FRAMESIZE_VGA;
    ZoomEnabled = false;
    ZoomMode = 0;
    zoomOffsetX = 0;
    zoomOffsetY = 0;
    ImageNegative = false;
    ImageAec2 = false;
#ifdef GRAYSCALE_AS_DEFAULT
    ImageGrayscale = true;
#else
    ImageGrayscale = false;
#endif
    SaveAllFiles = false;
    disabled = false;
    FixedExposure = false;
    namerawimage = "/sdcard/img_tmp/raw.jpg";
}     


ClassFlowTakeImage::ClassFlowTakeImage(std::vector<ClassFlow*>* lfc) : ClassFlowImage(lfc, TAG)
{
    imagesLocation = "/log/source";
    imagesRetention = 5;
    SetInitialParameter();
}


bool ClassFlowTakeImage::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);
    int _brightness = 0;
    int _contrast = 0;
    int _saturation = 0;
    int _sharpness = 0;
    int _autoExposureLevel = 0;

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

        if ((toUpper(splitted[0]) == "ZOOM") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                ZoomEnabled = true;
            else if (toUpper(splitted[1]) == "FALSE")
                ZoomEnabled = false;
        }
        if ((toUpper(splitted[0]) == "ZOOMMODE") && (splitted.size() > 1))
            ZoomMode = std::stod(splitted[1]);
        if ((toUpper(splitted[0]) == "ZOOMOFFSETX") && (splitted.size() > 1))
            zoomOffsetX = std::stod(splitted[1]);
        if ((toUpper(splitted[0]) == "ZOOMOFFSETY") && (splitted.size() > 1))
            zoomOffsetY = std::stod(splitted[1]);
        if ((toUpper(splitted[0]) == "GRAYSCALE") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                ImageGrayscale = true;
            else if (toUpper(splitted[1]) == "FALSE")
                ImageGrayscale = false;
        }
        if ((toUpper(splitted[0]) == "NEGATIVE") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                ImageNegative = true;
            else if (toUpper(splitted[1]) == "FALSE")
                ImageNegative = false;
        }
        if ((toUpper(splitted[0]) == "AEC2") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                ImageAec2 = true;
            else if (toUpper(splitted[1]) == "FALSE")
                ImageAec2 = false;
        }
        if ((toUpper(splitted[0]) == "AUTOEXPOSURELEVEL") && (splitted.size() > 1))
            _autoExposureLevel = std::stod(splitted[1]);

        if ((toUpper(splitted[0]) == "IMAGESIZE") && (splitted.size() > 1))
        {
            ImageSize = Camera.TextToFramesize(splitted[1].c_str());
            isImageSize = true;
        }

        if ((toUpper(splitted[0]) == "SAVEALLFILES") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                SaveAllFiles = true;
        }
        
        if ((toUpper(splitted[0]) == "WAITBEFORETAKINGPICTURE") && (splitted.size() > 1))
        {
            waitbeforepicture = stoi(splitted[1]);
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

        if ((toUpper(splitted[0]) == "SHARPNESS") && (splitted.size() > 1))
        {
            _sharpness = stoi(splitted[1]);
        }

        if ((toUpper(splitted[0]) == "FIXEDEXPOSURE") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                FixedExposure = true;  
        }

        if ((toUpper(splitted[0]) == "LEDINTENSITY") && (splitted.size() > 1))
        {
            float ledintensity = stof(splitted[1]);
            ledintensity = min((float) 100, ledintensity);
            ledintensity = max((float) 0, ledintensity);
            Camera.SetLEDIntensity(ledintensity);
        }

        if ((toUpper(splitted[0]) == "DEMO") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                Camera.useDemoMode();
        }
    }

    Camera.SetBrightnessContrastSaturation(_brightness, _contrast, _saturation, _autoExposureLevel, ImageGrayscale, ImageNegative, ImageAec2, _sharpness);
    Camera.SetQualitySize(ImageQuality, ImageSize, ZoomEnabled, ZoomMode, zoomOffsetX, zoomOffsetY);

    image_width = Camera.image_width;
    image_height = Camera.image_height;
    rawImage = new CImageBasis("rawImage");
    rawImage->CreateEmptyImage(image_width, image_height, 3);

    waitbeforepicture_store = waitbeforepicture;
    if (FixedExposure && (waitbeforepicture > 0))
    {
//        ESP_LOGD(TAG, "Fixed Exposure enabled!");
        int flash_duration = (int) (waitbeforepicture * 1000);
        Camera.EnableAutoExposure(flash_duration);
        waitbeforepicture = 0.2;
//        flash_duration = (int) (waitbeforepicture * 1000);
//        takePictureWithFlash(flash_duration);
//        rawImage->SaveToFile("/sdcard/init2.jpg");
    }

    return true;
}


string ClassFlowTakeImage::getHTMLSingleStep(string host)
{
    string result;
    result = "Raw Image: <br>\n<img src=\"" + host + "/img_tmp/raw.jpg\">\n";
    return result;
}


bool ClassFlowTakeImage::doFlow(string zwtime)
{
    psram_init_shared_memory_for_take_image_step();

    string logPath = CreateLogFolder(zwtime);

    int flash_duration = (int) (waitbeforepicture * 1000);
 
    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("ClassFlowTakeImage::doFlow - Before takePictureWithFlash");
    #endif


    #ifdef WIFITURNOFF
        esp_wifi_stop();        // to save power usage and 
    #endif

    takePictureWithFlash(flash_duration);

    #ifdef WIFITURNOFF
        esp_wifi_start();
    #endif


    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("ClassFlowTakeImage::doFlow - After takePictureWithFlash");
    #endif

    LogImage(logPath, "raw", NULL, NULL, zwtime, rawImage);

    RemoveOldLogs();

    #ifdef DEBUG_DETAIL_ON  
        LogFile.WriteHeapInfo("ClassFlowTakeImage::doFlow - After RemoveOldLogs");
    #endif

    psram_deinit_shared_memory_for_take_image_step();

    return true;
}


esp_err_t ClassFlowTakeImage::SendRawJPG(httpd_req_t *req)
{
    int flash_duration = (int) (waitbeforepicture * 1000);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return Camera.CaptureToHTTP(req, flash_duration);
}


ImageData* ClassFlowTakeImage::SendRawImage()
{
    CImageBasis *zw = new CImageBasis("SendRawImage", rawImage);
    ImageData *id;
    int flash_duration = (int) (waitbeforepicture * 1000);
    Camera.CaptureToBasisImage(zw, flash_duration);
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

ClassFlowTakeImage::~ClassFlowTakeImage(void)
{
    delete rawImage;
}

