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
    rawImage->width = CCstatus.CamConfig.ImageWidth;
    rawImage->height = CCstatus.CamConfig.ImageHeight;

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
            if (isStringNumeric(splitted[1]))
            {
                this->imagesRetention = std::stod(splitted[1]);
            }          
        }

        else if ((toUpper(splitted[0]) == "SAVEALLFILES") && (splitted.size() > 1))
        {
            CCstatus.SaveAllFiles = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "WAITBEFORETAKINGPICTURE") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
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
        }

        else if ((toUpper(splitted[0]) == "CAMGAINCEILING") && (splitted.size() > 1))
        {
            std::string _ImageGainceiling = toUpper(splitted[1]);

            if (isStringNumeric(_ImageGainceiling))
            {
                int _ImageGainceiling_ = std::stoi(_ImageGainceiling);
                switch (_ImageGainceiling_)
                {
                case 1:
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_4X;
                    break;
                case 2:
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_8X;
                    break;
                case 3:
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_16X;
                    break;
                case 4:
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_32X;
                    break;
                case 5:
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_64X;
                    break;
                case 6:
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_128X;
                    break;
                default:
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_2X;
                }
            }
            else
            {
                if (_ImageGainceiling == "X4")
                {
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_4X;
                }
                else if (_ImageGainceiling == "X8")
                {
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_8X;
                }
                else if (_ImageGainceiling == "X16")
                {
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_16X;
                }
                else if (_ImageGainceiling == "X32")
                {
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_32X;
                }
                else if (_ImageGainceiling == "X64")
                {
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_64X;
                }
                else if (_ImageGainceiling == "X128")
                {
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_128X;
                }
                else
                {
                    CCstatus.CamConfig.ImageGainceiling = GAINCEILING_2X;
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMQUALITY") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageQuality = std::stoi(splitted[1]);
                CCstatus.CamConfig.ImageQuality = clipInt(_ImageQuality, 63, 6);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMBRIGHTNESS") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageBrightness = std::stoi(splitted[1]);
                CCstatus.CamConfig.ImageBrightness = clipInt(_ImageBrightness, 2, -2);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMCONTRAST") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageContrast = std::stoi(splitted[1]);
                CCstatus.CamConfig.ImageContrast = clipInt(_ImageContrast, 2, -2);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMSATURATION") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageSaturation = std::stoi(splitted[1]);
                CCstatus.CamConfig.ImageSaturation = clipInt(_ImageSaturation, 2, -2);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMSHARPNESS") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageSharpness = std::stoi(splitted[1]);
                if (CCstatus.CamSensor_id == OV2640_PID)
                {
                    CCstatus.CamConfig.ImageSharpness = clipInt(_ImageSharpness, 2, -2);
                }
                else
                {
                    CCstatus.CamConfig.ImageSharpness = clipInt(_ImageSharpness, 3, -3);
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAUTOSHARPNESS") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageAutoSharpness = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMSPECIALEFFECT") && (splitted.size() > 1))
        {
            std::string _ImageSpecialEffect = toUpper(splitted[1]);

            if (isStringNumeric(_ImageSpecialEffect))
            {
                int _ImageSpecialEffect_ = std::stoi(_ImageSpecialEffect);
                CCstatus.CamConfig.ImageSpecialEffect = clipInt(_ImageSpecialEffect_, 6, 0);
            }
            else
            {
                if (_ImageSpecialEffect == "NEGATIVE")
                {
                    CCstatus.CamConfig.ImageSpecialEffect = 1;
                }
                else if (_ImageSpecialEffect == "GRAYSCALE")
                {
                    CCstatus.CamConfig.ImageSpecialEffect = 2;
                }
                else if (_ImageSpecialEffect == "RED")
                {
                    CCstatus.CamConfig.ImageSpecialEffect = 3;
                }
                else if (_ImageSpecialEffect == "GREEN")
                {
                    CCstatus.CamConfig.ImageSpecialEffect = 4;
                }
                else if (_ImageSpecialEffect == "BLUE")
                {
                    CCstatus.CamConfig.ImageSpecialEffect = 5;
                }
                else if (_ImageSpecialEffect == "RETRO")
                {
                    CCstatus.CamConfig.ImageSpecialEffect = 6;
                }
                else
                {
                    CCstatus.CamConfig.ImageSpecialEffect = 0;
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMWBMODE") && (splitted.size() > 1))
        {
            std::string _ImageWbMode = toUpper(splitted[1]);

            if (isStringNumeric(_ImageWbMode))
            {
                int _ImageWbMode_ = std::stoi(_ImageWbMode);
                CCstatus.CamConfig.ImageWbMode = clipInt(_ImageWbMode_, 4, 0);
            }
            else
            {
                if (_ImageWbMode == "SUNNY")
                {
                    CCstatus.CamConfig.ImageWbMode = 1;
                }
                else if (_ImageWbMode == "CLOUDY")
                {
                    CCstatus.CamConfig.ImageWbMode = 2;
                }
                else if (_ImageWbMode == "OFFICE")
                {
                    CCstatus.CamConfig.ImageWbMode = 3;
                }
                else if (_ImageWbMode == "HOME")
                {
                    CCstatus.CamConfig.ImageWbMode = 4;
                }
                else
                {
                    CCstatus.CamConfig.ImageWbMode = 0;
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAWB") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageAwb = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMAWBGAIN") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageAwbGain = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMAEC") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageAec = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMAEC2") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageAec2 = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMAELEVEL") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageAeLevel = std::stoi(splitted[1]);
                if (CCstatus.CamSensor_id == OV2640_PID)
                {
                    CCstatus.CamConfig.ImageAeLevel = clipInt(_ImageAeLevel, 2, -2);
                }
                else
                {
                    CCstatus.CamConfig.ImageAeLevel = clipInt(_ImageAeLevel, 5, -5);
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAECVALUE") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageAecValue = std::stoi(splitted[1]);
                CCstatus.CamConfig.ImageAecValue = clipInt(_ImageAecValue, 1200, 0);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMAGC") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageAgc = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMAGCGAIN") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageAgcGain = std::stoi(splitted[1]);
                CCstatus.CamConfig.ImageAgcGain = clipInt(_ImageAgcGain, 30, 0);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMBPC") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageBpc = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMWPC") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageWpc = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMRAWGMA") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageRawGma = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMLENC") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageLenc = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMHMIRROR") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageHmirror = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMVFLIP") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageVflip = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMDCW") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageDcw = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMDENOISE") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageDenoiseLevel = std::stoi(splitted[1]);
                if (CCstatus.CamSensor_id == OV2640_PID)
                {
                    CCstatus.CamConfig.ImageDenoiseLevel = 0;
                }
                else
                {
                    CCstatus.CamConfig.ImageDenoiseLevel = clipInt(_ImageDenoiseLevel, 8, 0);
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMZOOM") && (splitted.size() > 1))
        {
            CCstatus.CamConfig.ImageZoomEnabled = alphanumericToBoolean(splitted[1]);
        }

        else if ((toUpper(splitted[0]) == "CAMZOOMOFFSETX") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageZoomOffsetX = std::stoi(splitted[1]);
                if (CCstatus.CamSensor_id == OV2640_PID)
                {
                    CCstatus.CamConfig.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 480, -480);
                }
                else if (CCstatus.CamSensor_id == OV3660_PID)
                {
                    CCstatus.CamConfig.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 704, -704);
                }
                else if (CCstatus.CamSensor_id == OV5640_PID)
                {
                    CCstatus.CamConfig.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 960, -960);
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMZOOMOFFSETY") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageZoomOffsetY = std::stoi(splitted[1]);
                if (CCstatus.CamSensor_id == OV2640_PID)
                {
                    CCstatus.CamConfig.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 360, -360);
                }
                else if (CCstatus.CamSensor_id == OV3660_PID)
                {
                    CCstatus.CamConfig.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 528, -528);
                }
                else if (CCstatus.CamSensor_id == OV5640_PID)
                {
                    CCstatus.CamConfig.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 720, -720);
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMZOOMSIZE") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _ImageZoomSize = std::stoi(splitted[1]);
                if (CCstatus.CamSensor_id == OV2640_PID)
                {
                    CCstatus.CamConfig.ImageZoomSize = clipInt(_ImageZoomSize, 29, 0);
                }
                else if (CCstatus.CamSensor_id == OV3660_PID)
                {
                    CCstatus.CamConfig.ImageZoomSize = clipInt(_ImageZoomSize, 43, 0);
                }
                else if (CCstatus.CamSensor_id == OV5640_PID)
                {
                    CCstatus.CamConfig.ImageZoomSize = clipInt(_ImageZoomSize, 59, 0);
                }
            }
        }

        else if ((toUpper(splitted[0]) == "CAMFOCUS") && (splitted.size() > 1))
        {
            if (CCstatus.CamSensor_id == OV5640_PID)
            {
                CCstatus.CamConfig.CameraFocusEnabled = alphanumericToBoolean(splitted[1]);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMFOCUSAUTO") && (splitted.size() > 1))
        {
            if (CCstatus.CamSensor_id == OV5640_PID)
            {
                CCstatus.CamConfig.CameraManualFocus = !alphanumericToBoolean(splitted[1]);
            }
        }

        else if ((toUpper(splitted[0]) == "CAMFOCUSMANUALLEVEL") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int _CameraManualFocusLevel = std::stoi(splitted[1]);
                if (CCstatus.CamSensor_id == OV5640_PID)
                {
                    CCstatus.CamConfig.CameraManualFocusLevel = clipInt(_CameraManualFocusLevel, 1023, 0);
                }
            }
        }

        else if ((toUpper(splitted[0]) == "LEDINTENSITY") && (splitted.size() > 1))
        {
            if (isStringNumeric(splitted[1]))
            {
                int ledintensity = std::stoi(splitted[1]);
                CCstatus.CamConfig.ImageLedIntensity = Camera.CalculateLEDIntensity(ledintensity);
            }
        }

        else if ((toUpper(splitted[0]) == "DEMO") && (splitted.size() > 1))
        {
            CCstatus.DemoMode = alphanumericToBoolean(splitted[1]);
            if (CCstatus.DemoMode == true)
            {
                Camera.useDemoMode();
            }
        }
    }

    Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera

    rawImage = new CImageBasis("rawImage");
    rawImage->CreateEmptyImage(CCstatus.CamConfig.ImageWidth, CCstatus.CamConfig.ImageHeight, 3);

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
