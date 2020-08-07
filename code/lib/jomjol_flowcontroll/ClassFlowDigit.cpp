#include "ClassFlowDigit.h"

//#include "CFindTemplate.h"
//#include "CTfLiteClass.h"

// #define OHNETFLITE

#ifndef OHNETFLITE
#include "CTfLiteClass.h"
#endif

// #include "bitmap_image.hpp"

ClassFlowDigit::ClassFlowDigit()
{
    isLogImage = false;
    string cnnmodelfile = "";
    modelxsize = 1;
    modelysize = 1;
    ListFlowControll = NULL;
}

ClassFlowDigit::ClassFlowDigit(std::vector<ClassFlow*>* lfc)
{
    isLogImage = false;
    string cnnmodelfile = "";
    modelxsize = 1;
    modelysize = 1;
    ListFlowControll = NULL;
    ListFlowControll = lfc;
}

string ClassFlowDigit::getReadout()
{
    string rst = "";

    for (int i = 0; i < ROI.size(); ++i)
    {
        if (ROI[i]->resultklasse == 10)
            rst = rst + "N";
        else
            rst = rst + std::to_string(ROI[i]->resultklasse);
    }

    return rst;
}

bool ClassFlowDigit::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if (aktparamgraph.compare("[Digits]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] == "LogImageLocation") && (zerlegt.size() > 1))
        {
            isLogImage = true;
            LogImageLocation = zerlegt[1];
        }
        if ((zerlegt[0] == "Model") && (zerlegt.size() > 1))
        {
            cnnmodelfile = zerlegt[1];
        }
        if ((zerlegt[0] == "ModelInputSize") && (zerlegt.size() > 2))
        {
            modelxsize = std::stoi(zerlegt[1]);
            modelysize = std::stoi(zerlegt[2]);
        }
        if (zerlegt.size() >= 5)
        {
            roi* neuroi = new roi;
            neuroi->name = zerlegt[0];
            neuroi->posx = std::stoi(zerlegt[1]);
            neuroi->posy = std::stoi(zerlegt[2]);
            neuroi->deltax = std::stoi(zerlegt[3]);
            neuroi->deltay = std::stoi(zerlegt[4]);
            ROI.push_back(neuroi);
        }
    }
    return true;
}


bool ClassFlowDigit::doFlow(string time)
{
    doAlignAndCut(time);
    doNeuralNetwork(time);

    return true;
}

bool ClassFlowDigit::doAlignAndCut(string time)
{
    string input = "/sdcard/img_tmp/alg.jpg";
    string input_roi = "/sdcard/img_tmp/alg_roi.jpg";
    string ioresize = "/sdcard/img_tmp/resize.bmp";
    string output;
    string nm;
    input = FormatFileName(input);
    input_roi = FormatFileName(input_roi);   
     
    CResizeImage *rs;
    CImageBasis *img_roi = NULL;
    CAlignAndCutImage *caic = new CAlignAndCutImage(input);

    if (input_roi.length() > 0)
        img_roi = new CImageBasis(input_roi);


    for (int i = 0; i < ROI.size(); ++i)
    {
        printf("DigitalDigit %d - Align&Cut\n", i);
        output = "/sdcard/img_tmp/" + ROI[i]->name + ".jpg";
        output = FormatFileName(output);
        caic->CutAndSave(output, ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay);

        rs = new CResizeImage(output);
        rs->Resize(modelxsize, modelysize);
        ioresize = "/sdcard/img_tmp/rd" + std::to_string(i) + ".bmp";
        ioresize = FormatFileName(ioresize);
        rs->SaveToFile(ioresize);
        delete rs;

        if (img_roi)
        {
            img_roi->drawRect(ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay, 0, 0, 255, 2);
        }
    }
    delete caic;

    if (img_roi)
    {
        img_roi->SaveToFile(input_roi);
        delete img_roi;
    }

    return true;
} 

bool ClassFlowDigit::doNeuralNetwork(string time)
{
    string input = "/sdcard/img_tmp/alg.jpg";
    string ioresize = "/sdcard/img_tmp/resize.bmp";
    string output;
    string nm;
    input = FormatFileName(input);


#ifndef OHNETFLITE
    CTfLiteClass *tflite = new CTfLiteClass;  
    string zwcnn = "/sdcard" + cnnmodelfile;
    zwcnn = FormatFileName(zwcnn);
    printf(zwcnn.c_str());printf("\n");
    tflite->LoadModel(zwcnn); 
    tflite->MakeAllocate();
#endif

    for (int i = 0; i < ROI.size(); ++i)
    {
        printf("DigitalDigit %d - TfLite\n", i);
        ioresize = "/sdcard/img_tmp/rd" + std::to_string(i) + ".bmp";
        ioresize = FormatFileName(ioresize);
//        printf("output: %s, ioresize: %s\n", output.c_str(), ioresize.c_str());

        ROI[i]->resultklasse = 0;
#ifndef OHNETFLITE
        ROI[i]->resultklasse = tflite->GetClassFromImage(ioresize);
#endif
        printf("Result Digit%i: %d\n", i, ROI[i]->resultklasse);           

        if (isLogImage)
        {
            nm = "/sdcard" + LogImageLocation + "/" + std::to_string(ROI[i]->resultklasse) + "/" + time + "_" + ROI[i]->name + ".jpg";
            output = "/sdcard/img_tmp/" + ROI[i]->name + ".jpg";
            output = FormatFileName(output);            
            nm = FormatFileName(nm);
            CopyFile(output, nm);
        }        
    }
#ifndef OHNETFLITE
        delete tflite;
#endif
    return true;
}


std::vector<HTMLInfo*> ClassFlowDigit::GetHTMLInfo()
{
    std::vector<HTMLInfo*> result;

    for (int i = 0; i < ROI.size(); ++i)
    {
        HTMLInfo *zw = new HTMLInfo;
        zw->filename = ROI[i]->name + ".jpg";
        zw->val = ROI[i]->resultklasse;
        result.push_back(zw);
    }

    return result;
}