#include "ClassFlowMakeImage.h"
#include "Helper.h"


#include "CFindTemplate.h"
#include "ClassControllCamera.h"

#include <time.h>


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




ClassFlowMakeImage::ClassFlowMakeImage()
{
    isLogImage = false;
    waitbeforepicture = 5;
    isImageSize = false;
    ImageQuality = -1;    
    TimeImageTaken = 0;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
}

ClassFlowMakeImage::ClassFlowMakeImage(std::vector<ClassFlow*>* lfc)
{
    isLogImage = false;
    waitbeforepicture = 5;
    isImageSize = false;
    ImageQuality = -1;
    TimeImageTaken = 0;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";

    ListFlowControll = lfc;
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
            this->isLogImage = true;
            this->LogImageLocation = zerlegt[1];
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


void ClassFlowMakeImage::CopyFile(string input, string output)
{
    input = FormatFileName(input);
    output = FormatFileName(output);
    input = namerawimage;


    printf("Copy Input : %s\n", input.c_str());
    printf("Copy Output: %s\n", output.c_str());

    char cTemp;
    FILE* fpSourceFile = fopen(input.c_str(), "rb");
    FILE* fpTargetFile = fopen(output.c_str(), "wb");

    if (fpSourceFile == NULL)
    {
        printf("fpSourceFile == NULL\n");
        perror("Error");
    }

    if (fpTargetFile == NULL)
    {
        printf("fpTargetFile == NULL\n");
        perror("Error");
    }


    while (fread(&cTemp, 1, 1, fpSourceFile) == 1)
    {
        fwrite(&cTemp, 1, 1, fpTargetFile);
    }

    // Close The Files
    fclose(fpSourceFile);
    fclose(fpTargetFile);
    printf("Copy done\n");
}


bool ClassFlowMakeImage::doFlow(string zwtime)
{
    ////////////////////////////////////////////////////////////////////
    // TakeImage and Store into /image_tmp/raw.jpg  TO BE DONE
    ////////////////////////////////////////////////////////////////////

    int flashdauer = (int) waitbeforepicture * 1000;
    

    takePictureWithFlash(flashdauer);
    time(&TimeImageTaken);
    localtime(&TimeImageTaken);


    if (this->isLogImage)
    {
        string nm = "/sdcard" + this->LogImageLocation + "/" + zwtime + ".jpg";
        string input = "/sdcard/image_tmp/raw.jgp";
        printf("loginput from: %s to: %s\n", input.c_str(), nm.c_str());
        nm = FormatFileName(nm);
        input = FormatFileName(input);
        CopyFile(input, nm);
    }

    return true;
}

time_t ClassFlowMakeImage::getTimeImageTaken()
{
    return TimeImageTaken;
}
