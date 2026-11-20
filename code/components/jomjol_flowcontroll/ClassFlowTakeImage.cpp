#include <iostream>
#include <string>
#include <vector>
#include <regex>

#include "ClassFlowTakeImage.h"
#include "Helper.h"
#include "ClassLogFile.h"

#include "CImageBasis.h"
#include "ClassControlCamera.h"
#include "MainFlowControl.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "defines.h"
#include "psram.h"

#include <time.h>

static const char *TAG = "TAKEIMAGE";

ClassFlowTakeImage::ClassFlowTakeImage(std::vector<ClassFlow *> *lfc) : ClassFlowImage(lfc, TAG)
{
    imagesLocation = "/log/source";
    imagesRetention = 5;
    set_initial_parameter();
}

ClassFlowTakeImage::~ClassFlowTakeImage(void)
{
    delete rawImage;
}

void ClassFlowTakeImage::set_initial_parameter(void)
{
    TimeImageTaken = 0;
    rawImage = NULL;
    disabled = false;
    namerawimage = "/sdcard/img_tmp/raw.jpg";
}

bool ClassFlowTakeImage::ReadParameter(FILE *pfile, std::string &aktparamgraph)
{
    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
    {
        if (!GetNextParagraph(pfile, aktparamgraph))
        {
            return false;
        }
    }

    const std::string _aktparamgraph = toUpper(aktparamgraph);
    if (_aktparamgraph.compare("[TAKEIMAGE]") != 0)
    {
        return false;
    }

    Camera.get_sensor_control_config(&CCstatus); // Kamera >>> CCstatus

    std::vector<std::string> splitted;

    while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);

        if (splitted.size() > 1)
        {
            const std::string _param = toUpper(splitted[0]);

            if (_param == "RAWIMAGESLOCATION")
            {
                imagesLocation = "/sdcard" + splitted[1];
                isLogImage = true;
            }
            else if (_param == "RAWIMAGESRETENTION")
            {
                if (isStringNumeric(splitted[1]))
                {
                    imagesRetention = std::stod(splitted[1]);
                }
            }
            else if (_param == "SAVEALLFILES")
            {
                Camera.SaveAllFiles = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "WAITBEFORETAKINGPICTURE")
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
            else if (_param == "CAMXCLKFREQMHZ")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _CamXclkFreqMhz = std::stoi(splitted[1]);
                    CCstatus.CamXclkFreqMhz = clipInt(_CamXclkFreqMhz, 20, 4);
                }
            }
            else if (_param == "CAMGAINCEILING")
            {
                std::string _ImageGainceiling = toUpper(splitted[1]);

                if (isStringNumeric(_ImageGainceiling))
                {
                    int _ImageGainceiling_ = std::stoi(_ImageGainceiling);
                    switch (_ImageGainceiling_)
                    {
                    case 1:
                        CCstatus.ImageGainceiling = GAINCEILING_4X;
                        break;
                    case 2:
                        CCstatus.ImageGainceiling = GAINCEILING_8X;
                        break;
                    case 3:
                        CCstatus.ImageGainceiling = GAINCEILING_16X;
                        break;
                    case 4:
                        CCstatus.ImageGainceiling = GAINCEILING_32X;
                        break;
                    case 5:
                        CCstatus.ImageGainceiling = GAINCEILING_64X;
                        break;
                    case 6:
                        CCstatus.ImageGainceiling = GAINCEILING_128X;
                        break;
                    default:
                        CCstatus.ImageGainceiling = GAINCEILING_2X;
                    }
                }
                else
                {
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
            }
            else if (_param == "CAMQUALITY")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageQuality = std::stoi(splitted[1]);
                    CCstatus.ImageQuality = clipInt(_ImageQuality, 63, 6);
                }
            }
            else if (_param == "CAMBRIGHTNESS")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageBrightness = std::stoi(splitted[1]);
                    CCstatus.ImageBrightness = clipInt(_ImageBrightness, 2, -2);
                }
            }
            else if (_param == "CAMCONTRAST")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageContrast = std::stoi(splitted[1]);
                    CCstatus.ImageContrast = clipInt(_ImageContrast, 2, -2);
                }
            }
            else if (_param == "CAMSATURATION")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageSaturation = std::stoi(splitted[1]);
                    CCstatus.ImageSaturation = clipInt(_ImageSaturation, 2, -2);
                }
            }
            else if (_param == "CAMSHARPNESS")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageSharpness = std::stoi(splitted[1]);
                    if (Camera.CamSensor_id == OV2640_PID)
                    {
                        CCstatus.ImageSharpness = clipInt(_ImageSharpness, 2, -2);
                    }
                    else
                    {
                        CCstatus.ImageSharpness = clipInt(_ImageSharpness, 3, -3);
                    }
                }
            }
            else if (_param == "CAMAUTOSHARPNESS")
            {
                CCstatus.ImageAutoSharpness = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMSPECIALEFFECT")
            {
                std::string _ImageSpecialEffect = toUpper(splitted[1]);

                if (isStringNumeric(_ImageSpecialEffect))
                {
                    int _ImageSpecialEffect_ = std::stoi(_ImageSpecialEffect);
                    CCstatus.ImageSpecialEffect = clipInt(_ImageSpecialEffect_, 6, 0);
                }
                else
                {
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
            }
            else if (_param == "CAMWBMODE")
            {
                std::string _ImageWbMode = toUpper(splitted[1]);

                if (isStringNumeric(_ImageWbMode))
                {
                    int _ImageWbMode_ = std::stoi(_ImageWbMode);
                    CCstatus.ImageWbMode = clipInt(_ImageWbMode_, 4, 0);
                }
                else
                {
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
            }
            else if (_param == "CAMAWB")
            {
                CCstatus.ImageAwb = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAWBGAIN")
            {
                CCstatus.ImageAwbGain = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAEC")
            {
                CCstatus.ImageAec = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAEC2")
            {
                CCstatus.ImageAec2 = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAELEVEL")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageAeLevel = std::stoi(splitted[1]);
                    if (Camera.CamSensor_id == OV2640_PID)
                    {
                        CCstatus.ImageAeLevel = clipInt(_ImageAeLevel, 2, -2);
                    }
                    else
                    {
                        CCstatus.ImageAeLevel = clipInt(_ImageAeLevel, 5, -5);
                    }
                }
            }
            else if (_param == "CAMAECVALUE")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageAecValue = std::stoi(splitted[1]);
                    CCstatus.ImageAecValue = clipInt(_ImageAecValue, 1200, 0);
                }
            }
            else if (_param == "CAMAGC")
            {
                CCstatus.ImageAgc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAGCGAIN")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageAgcGain = std::stoi(splitted[1]);
                    CCstatus.ImageAgcGain = clipInt(_ImageAgcGain, 30, 0);
                }
            }
            else if (_param == "CAMBPC")
            {
                CCstatus.ImageBpc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMWPC")
            {
                CCstatus.ImageWpc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMRAWGMA")
            {
                CCstatus.ImageRawGma = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMLENC")
            {
                CCstatus.ImageLenc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMHMIRROR")
            {
                CCstatus.ImageHmirror = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMVFLIP")
            {
                CCstatus.ImageVflip = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMDCW")
            {
                CCstatus.ImageDcw = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMDENOISE")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageDcw = std::stoi(splitted[1]);
                    if (Camera.CamSensor_id == OV2640_PID)
                    {
                        CCstatus.ImageDcw = 0;
                    }
                    else
                    {
                        CCstatus.ImageDcw = clipInt(_ImageDcw, 8, 0);
                    }
                }
            }
            else if (_param == "CAMZOOM")
            {
                CCstatus.ImageZoomEnabled = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMZOOMOFFSETX")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageZoomOffsetX = std::stoi(splitted[1]);
                    if (Camera.CamSensor_id == OV2640_PID)
                    {
                        CCstatus.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 480, -480);
                    }
                    else if (Camera.CamSensor_id == OV3660_PID)
                    {
                        CCstatus.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 704, -704);
                    }
                    else if (Camera.CamSensor_id == OV5640_PID)
                    {
                        CCstatus.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 960, -960);
                    }
                }
            }
            else if (_param == "CAMZOOMOFFSETY")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageZoomOffsetY = std::stoi(splitted[1]);
                    if (Camera.CamSensor_id == OV2640_PID)
                    {
                        CCstatus.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 360, -360);
                    }
                    else if (Camera.CamSensor_id == OV3660_PID)
                    {
                        CCstatus.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 528, -528);
                    }
                    else if (Camera.CamSensor_id == OV5640_PID)
                    {
                        CCstatus.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 720, -720);
                    }
                }
            }
            else if (_param == "CAMZOOMSIZE")
            {
                if (isStringNumeric(splitted[1]))
                {
                    int _ImageZoomSize = std::stoi(splitted[1]);
                    if (Camera.CamSensor_id == OV2640_PID)
                    {
                        CCstatus.ImageZoomSize = clipInt(_ImageZoomSize, 29, 0);
                    }
                    else if (Camera.CamSensor_id == OV3660_PID)
                    {
                        CCstatus.ImageZoomSize = clipInt(_ImageZoomSize, 43, 0);
                    }
                    else if (Camera.CamSensor_id == OV5640_PID)
                    {
                        CCstatus.ImageZoomSize = clipInt(_ImageZoomSize, 59, 0);
                    }
                }
            }
            else if (_param == "LEDINTENSITY")
            {
                if (isStringNumeric(splitted[1]))
                {
                    CCstatus.ImageLedIntensity = std::stoi(splitted[1]);
                }
            }
            else if (_param == "DEMO")
            {
                Camera.DemoMode = alphanumericToBoolean(splitted[1]);
                if (Camera.DemoMode == true)
                {
                    Camera.use_demo_mode();
                }
            }
        }
    }

    Camera.changedCameraSettings = true;
    Camera.CamTempImage = false;

    rawImage = new (std::nothrow) CImageBasis("rawImage");
    if (rawImage == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Cannot create raw image object");
        return false;
    }

    rawImage->CreateEmptyImage(CCstatus.ImageWidth, CCstatus.ImageHeight, 3);
    if (!rawImage->ImageOkay())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Cannot allocate raw image buffer in PSRAM");
        delete rawImage;
        rawImage = nullptr;
        return false;
    }

    return true;
}

// wird bei jeder Auswertrunde aufgerufen
bool ClassFlowTakeImage::doFlow(std::string zwtime)
{
    if (!psram_init_shared_memory_for_take_image_step())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Take Image aborted: shared PSRAM is already owned by another processing step");
        return false;
    }

    std::string logPath = CreateLogFolder(zwtime);

    int flash_duration = (int)(CCstatus.WaitBeforePicture * 1000);

    if (!take_picture_with_flash(flash_duration))
    {
        psram_deinit_shared_memory_for_take_image_step();
        return false;
    }

    LogImage(logPath, "raw", NULL, NULL, zwtime, rawImage);

    RemoveOldLogs();

    psram_deinit_shared_memory_for_take_image_step();

    return true;
}

bool ClassFlowTakeImage::take_picture_with_flash(int flash_duration)
{
    if (rawImage == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "take_picture_with_flash: raw image object is not initialized");
        return false;
    }

    const esp_err_t captureResult = Camera.capture_to_basis_image(rawImage, flash_duration);
    if (captureResult != ESP_OK)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Camera capture failed; skipping this readout");
        return false;
    }

    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    if (Camera.SaveAllFiles)
    {
        rawImage->SaveToFile(namerawimage);
    }

    return true;
}

std::string ClassFlowTakeImage::get_html_single_step(std::string host)
{
    return ("Raw Image: <br>\n<img src=\"" + host + "/img_tmp/raw.jpg\">\n");
}

esp_err_t ClassFlowTakeImage::send_raw_jpg(httpd_req_t *req)
{
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    int flash_duration = Camera.CamTempImage ? (int)(CFstatus.WaitBeforePicture * 1000) : (int)(CCstatus.WaitBeforePicture * 1000);
    return Camera.capture_to_http(req, flash_duration);
}

ImageData *ClassFlowTakeImage::send_raw_image(void)
{
    if (rawImage == nullptr || !rawImage->ImageOkay())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SendRawImage: raw image is not available");
        return nullptr;
    }

    CImageBasis *_ImageBasis = new (std::nothrow) CImageBasis("SendRawImage", rawImage);
    if (_ImageBasis == nullptr || !_ImageBasis->ImageOkay())
    {
        delete _ImageBasis;
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "SendRawImage: cannot allocate image copy");
        return nullptr;
    }

    int flash_duration = (int)(CCstatus.WaitBeforePicture * 1000);
    if (Camera.capture_to_basis_image(_ImageBasis, flash_duration) != ESP_OK)
    {
        delete _ImageBasis;
        return nullptr;
    }

    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    ImageData *_ImageData = _ImageBasis->writeToMemoryAsJPG();
    delete _ImageBasis;

    return _ImageData;
}

esp_err_t ClassFlowTakeImage::camera_capture(void)
{
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);

    return Camera.capture_to_file(namerawimage);
}

time_t ClassFlowTakeImage::get_time_image_taken(void)
{
    return TimeImageTaken;
}
