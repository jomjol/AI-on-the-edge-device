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

    if (CCstatus.SaveAllFiles) {
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
    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0) {
        if (!this->GetNextParagraph(pfile, aktparamgraph)) {
            return false;
        }
    }

    if (aktparamgraph.compare("[TakeImage]") != 0) {
        // Paragraph does not fit TakeImage
        return false;
    }

    std::vector<string> splitted;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph)) {
        splitted = ZerlegeZeile(aktparamgraph);

        if (splitted.size() > 1) {
            std::string _param = toUpper(splitted[0]);

            if (_param == "RAWIMAGESLOCATION") {
                imagesLocation = "/sdcard" + splitted[1];
                isLogImage = true;
            }
            else if (_param == "RAWIMAGESRETENTION") {
                if (isStringNumeric(splitted[1])) {
                    this->imagesRetention = std::stod(splitted[1]);
                }
            }
            else if (_param == "SAVEALLFILES") {
                CCstatus.SaveAllFiles = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "WAITBEFORETAKINGPICTURE") {
                if (isStringNumeric(splitted[1])) {
                    int _WaitBeforePicture = std::stoi(splitted[1]);
                    if (_WaitBeforePicture != 0) {
                        CCstatus.WaitBeforePicture = _WaitBeforePicture;
                    }
                    else {
                        CCstatus.WaitBeforePicture = 2;
                    }
                }
            }
            else if (_param == "CAMGAINCEILING") {
                std::string _ImageGainceiling = toUpper(splitted[1]);
                if (isStringNumeric(_ImageGainceiling)) {
                    int _ImageGainceiling_ = std::stoi(_ImageGainceiling);
                    switch (_ImageGainceiling_) {
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
                else {
                    if (_ImageGainceiling == "X4") {
                        CCstatus.ImageGainceiling = GAINCEILING_4X;
                    }
                    else if (_ImageGainceiling == "X8") {
                        CCstatus.ImageGainceiling = GAINCEILING_8X;
                    }
                    else if (_ImageGainceiling == "X16") {
                        CCstatus.ImageGainceiling = GAINCEILING_16X;
                    }
                    else if (_ImageGainceiling == "X32") {
                        CCstatus.ImageGainceiling = GAINCEILING_32X;
                    }
                    else if (_ImageGainceiling == "X64") {
                        CCstatus.ImageGainceiling = GAINCEILING_64X;
                    }
                    else if (_ImageGainceiling == "X128") {
                        CCstatus.ImageGainceiling = GAINCEILING_128X;
                    }
                    else {
                        CCstatus.ImageGainceiling = GAINCEILING_2X;
                    }
                }
            }
            else if (_param == "CAMQUALITY") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageQuality = std::stoi(splitted[1]);
                    CCstatus.ImageQuality = clipInt(_ImageQuality, 63, 6);
                }
            }
            else if (_param == "CAMBRIGHTNESS") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageBrightness = std::stoi(splitted[1]);
                    CCstatus.ImageBrightness = clipInt(_ImageBrightness, 2, -2);
                }
            }
            else if (_param == "CAMCONTRAST") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageContrast = std::stoi(splitted[1]);
                    CCstatus.ImageContrast = clipInt(_ImageContrast, 2, -2);
                }
            }
            else if (_param == "CAMSATURATION") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageSaturation = std::stoi(splitted[1]);
                    CCstatus.ImageSaturation = clipInt(_ImageSaturation, 2, -2);
                }
            }
            else if (_param == "CAMSHARPNESS") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageSharpness = std::stoi(splitted[1]);
                    if (CCstatus.CamSensor_id == OV2640_PID) {
                        CCstatus.ImageSharpness = clipInt(_ImageSharpness, 2, -2);
                    }
                    else {
                        CCstatus.ImageSharpness = clipInt(_ImageSharpness, 3, -3);
                    }
                }
            }
            else if (_param == "CAMAUTOSHARPNESS") {
                CCstatus.ImageAutoSharpness = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMSPECIALEFFECT") {
                std::string _ImageSpecialEffect = toUpper(splitted[1]);
                if (isStringNumeric(_ImageSpecialEffect)) {
                    int _ImageSpecialEffect_ = std::stoi(_ImageSpecialEffect);
                    CCstatus.ImageSpecialEffect = clipInt(_ImageSpecialEffect_, 6, 0);
                }
                else {
                    if (_ImageSpecialEffect == "NEGATIVE") {
                        CCstatus.ImageSpecialEffect = 1;
                    }
                    else if (_ImageSpecialEffect == "GRAYSCALE") {
                        CCstatus.ImageSpecialEffect = 2;
                    }
                    else if (_ImageSpecialEffect == "RED") {
                        CCstatus.ImageSpecialEffect = 3;
                    }
                    else if (_ImageSpecialEffect == "GREEN") {
                        CCstatus.ImageSpecialEffect = 4;
                    }
                    else if (_ImageSpecialEffect == "BLUE") {
                        CCstatus.ImageSpecialEffect = 5;
                    }
                    else if (_ImageSpecialEffect == "RETRO") {
                        CCstatus.ImageSpecialEffect = 6;
                    }
                    else {
                        CCstatus.ImageSpecialEffect = 0;
                    }
                }
            }
            else if (_param == "CAMWBMODE") {
                std::string _ImageWbMode = toUpper(splitted[1]);
                if (isStringNumeric(_ImageWbMode)) {
                    int _ImageWbMode_ = std::stoi(_ImageWbMode);
                    CCstatus.ImageWbMode = clipInt(_ImageWbMode_, 4, 0);
                }
                else {
                    if (_ImageWbMode == "SUNNY") {
                        CCstatus.ImageWbMode = 1;
                    }
                    else if (_ImageWbMode == "CLOUDY") {
                        CCstatus.ImageWbMode = 2;
                    }
                    else if (_ImageWbMode == "OFFICE") {
                        CCstatus.ImageWbMode = 3;
                    }
                    else if (_ImageWbMode == "HOME") {
                        CCstatus.ImageWbMode = 4;
                    }
                    else {
                        CCstatus.ImageWbMode = 0;
                    }
                }
            }
            else if (_param == "CAMAWB") {
                CCstatus.ImageAwb = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAWBGAIN") {
                CCstatus.ImageAwbGain = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAEC") {
                CCstatus.ImageAec = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAEC2") {
                CCstatus.ImageAec2 = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAELEVEL") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageAeLevel = std::stoi(splitted[1]);
                    if (CCstatus.CamSensor_id == OV2640_PID) {
                        CCstatus.ImageAeLevel = clipInt(_ImageAeLevel, 2, -2);
                    }
                    else {
                        CCstatus.ImageAeLevel = clipInt(_ImageAeLevel, 5, -5);
                    }
                }
            }
            else if (_param == "CAMAECVALUE") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageAecValue = std::stoi(splitted[1]);
                    CCstatus.ImageAecValue = clipInt(_ImageAecValue, 1200, 0);
                }
            }
            else if (_param == "CAMAGC") {
                CCstatus.ImageAgc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMAGCGAIN") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageAgcGain = std::stoi(splitted[1]);
                    CCstatus.ImageAgcGain = clipInt(_ImageAgcGain, 30, 0);
                }
            }
            else if (_param == "CAMBPC") {
                CCstatus.ImageBpc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMWPC") {
                CCstatus.ImageWpc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMRAWGMA") {
                CCstatus.ImageRawGma = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMLENC") {
                CCstatus.ImageLenc = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMHMIRROR") {
                CCstatus.ImageHmirror = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMVFLIP") {
                CCstatus.ImageVflip = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMDCW") {
                CCstatus.ImageDcw = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMDENOISE") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageDenoiseLevel = std::stoi(splitted[1]);
                    if (CCstatus.CamSensor_id == OV2640_PID) {
                        CCstatus.ImageDenoiseLevel = 0;
                    }
                    else {
                        CCstatus.ImageDenoiseLevel = clipInt(_ImageDenoiseLevel, 8, 0);
                    }
                }
            }
            else if (_param == "CAMZOOM") {
                CCstatus.ImageZoomEnabled = alphanumericToBoolean(splitted[1]);
            }
            else if (_param == "CAMZOOMOFFSETX") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageZoomOffsetX = std::stoi(splitted[1]);
                    if (CCstatus.CamSensor_id == OV2640_PID) {
                        CCstatus.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 480, -480);
                    }
                    else if (CCstatus.CamSensor_id == OV3660_PID) {
                        CCstatus.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 704, -704);
                    }
                    else if (CCstatus.CamSensor_id == OV5640_PID) {
                        CCstatus.ImageZoomOffsetX = clipInt(_ImageZoomOffsetX, 960, -960);
                    }
                }
            }
            else if (_param == "CAMZOOMOFFSETY") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageZoomOffsetY = std::stoi(splitted[1]);
                    if (CCstatus.CamSensor_id == OV2640_PID) {
                        CCstatus.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 360, -360);
                    }
                    else if (CCstatus.CamSensor_id == OV3660_PID) {
                        CCstatus.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 528, -528);
                    }
                    else if (CCstatus.CamSensor_id == OV5640_PID) {
                        CCstatus.ImageZoomOffsetY = clipInt(_ImageZoomOffsetY, 720, -720);
                    }
                }
            }
            else if (_param == "CAMZOOMSIZE") {
                if (isStringNumeric(splitted[1])) {
                    int _ImageZoomSize = std::stoi(splitted[1]);
                    if (CCstatus.CamSensor_id == OV2640_PID) {
                        CCstatus.ImageZoomSize = clipInt(_ImageZoomSize, 29, 0);
                    }
                    else if (CCstatus.CamSensor_id == OV3660_PID) {
                        CCstatus.ImageZoomSize = clipInt(_ImageZoomSize, 43, 0);
                    }
                    else if (CCstatus.CamSensor_id == OV5640_PID) {
                        CCstatus.ImageZoomSize = clipInt(_ImageZoomSize, 59, 0);
                    }
                }
            }
            else if (_param == "LEDINTENSITY") {
                if (isStringNumeric(splitted[1])) {
                    int ledintensity = std::stoi(splitted[1]);
                    CCstatus.ImageLedIntensity = Camera.SetLEDIntensity(ledintensity);
                }
            }
            else if (_param == "DEMO") {
                CCstatus.DemoMode = alphanumericToBoolean(splitted[1]);
                if (CCstatus.DemoMode == true) {
                    Camera.useDemoMode();
                }
            }
        }
    }

    Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
    Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize, CCstatus.ImageVflip);

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
    if (CFstatus.changedCameraSettings) {
        Camera.setSensorDatenFromCCstatus(); // CCstatus >>> Kamera
        Camera.SetQualityZoomSize(CCstatus.ImageQuality, CCstatus.ImageFrameSize, CCstatus.ImageZoomEnabled, CCstatus.ImageZoomOffsetX, CCstatus.ImageZoomOffsetY, CCstatus.ImageZoomSize, CCstatus.ImageVflip);
        Camera.LedIntensity = CCstatus.ImageLedIntensity;
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
