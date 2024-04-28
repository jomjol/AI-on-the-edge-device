#include <iostream>
#include <string>
#include <vector>
#include <regex>

#include "ClassFlowTakeImage.h"
#include "Helper.h"
#include "ClassLogFile.h"

#include "CImageBasis.h"
#include "ClassControllCamera.h"
#include "MainFlowControl.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "../../include/defines.h"
#include "psram.h"

#include <time.h>

// #define DEBUG_DETAIL_ON
// #define WIFITURNOFF

static const char *TAG = "TAKEIMAGE";

esp_err_t ClassFlowTakeImage::camera_capture(void)
{
    string nm = namerawimage;
    Camera.CaptureToFile(nm);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return ESP_OK;
}

void ClassFlowTakeImage::takePictureWithFlash(int flash_duration)
{
    // in case the image is flipped, it must be reset here //
    rawImage->width = CCstatus.ImageWidth;
    rawImage->height = CCstatus.ImageHeight;

    ESP_LOGD(TAG, "flash_duration: %d", flash_duration);

    Camera.CaptureToBasisImage(rawImage, flash_duration);

    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    if (CCstatus.SaveAllFiles)
    {
        rawImage->SaveToFile(namerawimage);
    }
}

void ClassFlowTakeImage::SetInitialParameter(void)
{
    TimeImageTaken = 0;
    rawImage = NULL;
    disabled = false;
    namerawimage = "/sdcard/img_tmp/raw.jpg";
}

// auslesen der Kameraeinstellungen aus der config.ini
// wird beim Start aufgerufen
bool ClassFlowTakeImage::ReadParameter(FILE *pfile, string &aktparamgraph)
{
    Camera.getSensorDatenToCCstatus(); // Kamera >>> CCstatus

    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
    {
        if (!this->GetNextParagraph(pfile, aktparamgraph))
        {
            return false;
        }
    }

    if (aktparamgraph.compare("[TakeImage]") != 0)
    {
        // Paragraph does not fit TakeImage
        return false;
    }

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);

        if ((toUpper(splitted[0]) == "RAWIMAGESLOCATION") && (splitted.size() > 1))
        {
            imagesLocation = "/sdcard" + splitted[1];
            isLogImage = true;
        }

        else if ((toUpper(splitted[0]) == "RAWIMAGESRETENTION") && (splitted.size() > 1))
        {
            this->imagesRetention = std::stod(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "SAVEALLFILES") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.SaveAllFiles = 1;
            }
            else
            {
                CCstatus.SaveAllFiles = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "WAITBEFORETAKINGPICTURE") && (splitted.size() > 1))
        {
            int _WaitBeforePicture = std::stoi(splitted[1]);
            if (_WaitBeforePicture != 0)
            {
                CCstatus.WaitBeforePicture = _WaitBeforePicture;
            }
            else
            {
                CCstatus.WaitBeforePicture = 2;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMGAINCEILING") && (splitted.size() > 1))
        {
            std::string _ImageGainceiling = toUpper(splitted[1]);
            if (_ImageGainceiling == "X4")
            {
                CCstatus.ImageGainceiling = GAINCEILING_4X;
            }
            else if (_ImageGainceiling == "X8")
            {
                CCstatus.ImageGainceiling = GAINCEILING_8X;
            }
            else if (_ImageGainceiling == "X16")
            {
                CCstatus.ImageGainceiling = GAINCEILING_16X;
            }
            else if (_ImageGainceiling == "X32")
            {
                CCstatus.ImageGainceiling = GAINCEILING_32X;
            }
            else if (_ImageGainceiling == "X64")
            {
                CCstatus.ImageGainceiling = GAINCEILING_64X;
            }
            else if (_ImageGainceiling == "X128")
            {
                CCstatus.ImageGainceiling = GAINCEILING_128X;
            }
            else
            {
                CCstatus.ImageGainceiling = GAINCEILING_2X;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMQUALITY") && (splitted.size() > 1))
        {
            int _ImageQuality = std::stoi(splitted[1]);
            if ((_ImageQuality >= 0) && (_ImageQuality <= 63))
            {
                CCstatus.ImageQuality = _ImageQuality;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMBRIGHTNESS") && (splitted.size() > 1))
        {
            int _ImageBrightness = std::stoi(splitted[1]);
            if ((_ImageBrightness >= -2) && (_ImageBrightness <= 2))
            {
                CCstatus.ImageBrightness = _ImageBrightness;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMCONTRAST") && (splitted.size() > 1))
        {
            int _ImageContrast = std::stoi(splitted[1]);
            if ((_ImageContrast >= -2) && (_ImageContrast <= 2))
            {
                CCstatus.ImageContrast = _ImageContrast;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMSATURATION") && (splitted.size() > 1))
        {
            int _ImageSaturation = std::stoi(splitted[1]);
            if ((_ImageSaturation >= -2) && (_ImageSaturation <= 2))
            {
                CCstatus.ImageSaturation = _ImageSaturation;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMSHARPNESS") && (splitted.size() > 1))
        {
            int _ImageSharpness = std::stoi(splitted[1]);
            if ((_ImageSharpness >= -2) && (_ImageSharpness <= 2))
            {
                CCstatus.ImageSharpness = _ImageSharpness;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAUTOSHARPNESS") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageAutoSharpness = 1;
            }
            else
            {
                CCstatus.ImageAutoSharpness = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMSPECIALEFFECT") && (splitted.size() > 1))
        {
            std::string _ImageSpecialEffect = toUpper(splitted[1]);
            if (_ImageSpecialEffect == "NEGATIVE")
            {
                CCstatus.ImageSpecialEffect = 1;
            }
            else if (_ImageSpecialEffect == "GRAYSCALE")
            {
                CCstatus.ImageSpecialEffect = 2;
            }
            else if (_ImageSpecialEffect == "RED")
            {
                CCstatus.ImageSpecialEffect = 3;
            }
            else if (_ImageSpecialEffect == "GREEN")
            {
                CCstatus.ImageSpecialEffect = 4;
            }
            else if (_ImageSpecialEffect == "BLUE")
            {
                CCstatus.ImageSpecialEffect = 5;
            }
            else if (_ImageSpecialEffect == "RETRO")
            {
                CCstatus.ImageSpecialEffect = 6;
            }
            else
            {
                CCstatus.ImageSpecialEffect = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMWBMODE") && (splitted.size() > 1))
        {
            std::string _ImageWbMode = toUpper(splitted[1]);
            if (_ImageWbMode == "SUNNY")
            {
                CCstatus.ImageWbMode = 1;
            }
            else if (_ImageWbMode == "CLOUDY")
            {
                CCstatus.ImageWbMode = 2;
            }
            else if (_ImageWbMode == "OFFICE")
            {
                CCstatus.ImageWbMode = 3;
            }
            else if (_ImageWbMode == "HOME")
            {
                CCstatus.ImageWbMode = 4;
            }
            else
            {
                CCstatus.ImageWbMode = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAWB") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageAwb = 1;
            }
            else
            {
                CCstatus.ImageAwb = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAWBGAIN") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageAwbGain = 1;
            }
            else
            {
                CCstatus.ImageAwbGain = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAEC") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageAec = 1;
            }
            else
            {
                CCstatus.ImageAec = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAEC2") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageAec2 = 1;
            }
            else
            {
                CCstatus.ImageAec2 = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAELEVEL") && (splitted.size() > 1))
        {
            int _ImageAeLevel = std::stoi(splitted[1]);
            if ((_ImageAeLevel >= -2) && (_ImageAeLevel <= 2))
            {
                CCstatus.ImageAeLevel = _ImageAeLevel;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAECVALUE") && (splitted.size() > 1))
        {
            int _ImageAecValue = std::stoi(splitted[1]);
            if ((_ImageAecValue >= 0) && (_ImageAecValue <= 1200))
            {
                CCstatus.ImageAecValue = _ImageAecValue;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAGC") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageAgc = 1;
            }
            else
            {
                CCstatus.ImageAgc = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAGCGAIN") && (splitted.size() > 1))
        {
            int _ImageAgcGain = std::stoi(splitted[1]);
            if ((_ImageAgcGain >= 0) && (_ImageAgcGain <= 30))
            {
                CCstatus.ImageAgcGain = _ImageAgcGain;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMBPC") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageBpc = 1;
            }
            else
            {
                CCstatus.ImageBpc = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMWPC") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageWpc = 1;
            }
            else
            {
                CCstatus.ImageWpc = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMRAWGMA") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageRawGma = 1;
            }
            else
            {
                CCstatus.ImageRawGma = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMLENC") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageLenc = 1;
            }
            else
            {
                CCstatus.ImageLenc = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMHMIRROR") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageHmirror = 1;
            }
            else
            {
                CCstatus.ImageHmirror = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMVFLIP") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageVflip = 1;
            }
            else
            {
                CCstatus.ImageVflip = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMDCW") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageDcw = 1;
            }
            else
            {
                CCstatus.ImageDcw = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMZOOM") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.ImageZoomEnabled = 1;
            }
            else
            {
                CCstatus.ImageZoomEnabled = 0;
            }
        }

        else if ((toUpper(splitted[0]) == "CAMZOOMOFFSETX") && (splitted.size() > 1))
        {
            CCstatus.ImageZoomOffsetX = std::stoi(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMZOOMOFFSETY") && (splitted.size() > 1))
        {
            CCstatus.ImageZoomOffsetY = std::stoi(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMZOOMSIZE") && (splitted.size() > 1))
        {
            int _ImageZoomSize = std::stoi(splitted[1]);
            if (_ImageZoomSize >= 0)
            {
                CCstatus.ImageZoomSize = _ImageZoomSize;
            }
        }

        else if ((toUpper(splitted[0]) == "LEDINTENSITY") && (splitted.size() > 1))
        {
            float ledintensity = std::stof(splitted[1]);
            Camera.SetLEDIntensity(ledintensity);
        }

        else if ((toUpper(splitted[0]) == "DEMO") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
            {
                CCstatus.DemoMode = true;
                Camera.useDemoMode();
            }
            else
            {
                CCstatus.DemoMode = false;
            }
        }
    }

    Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
    Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize);

    rawImage = new CImageBasis("rawImage");
    rawImage->CreateEmptyImage(CCstatus.ImageWidth, CCstatus.ImageHeight, 3);

    return true;
}

ClassFlowTakeImage::ClassFlowTakeImage(std::vector<ClassFlow *> *lfc) : ClassFlowImage(lfc, TAG)
{
    imagesLocation = "/log/source";
    imagesRetention = 5;
    SetInitialParameter();
}

string ClassFlowTakeImage::getHTMLSingleStep(string host)
{
    string result;
    result = "Raw Image: <br>\n<img src=\"" + host + "/img_tmp/raw.jpg\">\n";
    return result;
}

// wird bei jeder Auswertrunde aufgerufen
bool ClassFlowTakeImage::doFlow(string zwtime)
{
    psram_init_shared_memory_for_take_image_step();

    string logPath = CreateLogFolder(zwtime);

    int flash_duration = (int)(CCstatus.WaitBeforePicture * 1000);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("ClassFlowTakeImage::doFlow - Before takePictureWithFlash");
#endif

#ifdef WIFITURNOFF
    esp_wifi_stop(); // to save power usage and
#endif

    // wenn die Kameraeinstellungen durch Erstellen eines neuen Referenzbildes verändert wurden, müssen sie neu gesetzt werden
    if (CFstatus.changedCameraSettings)
    {
        Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
        Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize);
        CFstatus.changedCameraSettings = false;
    }

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
    int flash_duration = (int)(CCstatus.WaitBeforePicture * 1000);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return Camera.CaptureToHTTP(req, flash_duration);
}

ImageData *ClassFlowTakeImage::SendRawImage(void)
{
    CImageBasis *zw = new CImageBasis("SendRawImage", rawImage);
    ImageData *id;
    int flash_duration = (int)(CCstatus.WaitBeforePicture * 1000);
    Camera.CaptureToBasisImage(zw, flash_duration);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    id = zw->writeToMemoryAsJPG();
    delete zw;
    return id;
}

time_t ClassFlowTakeImage::getTimeImageTaken(void)
{
    return TimeImageTaken;
}

ClassFlowTakeImage::~ClassFlowTakeImage(void)
{
    delete rawImage;
}
