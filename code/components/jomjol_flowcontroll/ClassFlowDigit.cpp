#include "ClassFlowDigit.h"


//#include "CFindTemplate.h"
//#include "CTfLiteClass.h"

// #define OHNETFLITE

#ifndef OHNETFLITE
#include "CTfLiteClass.h"
#endif

// #include "bitmap_image.hpp"

#include "ClassLogFile.h"

static const char* TAG = "flow_digital";


void ClassFlowDigit::SetInitialParameter(void)
{
    string cnnmodelfile = "";
    modelxsize = 1;
    modelysize = 1;
    ListFlowControll = NULL;
    previousElement = NULL;    
    SaveAllFiles = false;
    disabled = false;
    DecimalShift = 0;
    DecimalShiftEnabled = false;
}    

ClassFlowDigit::ClassFlowDigit() : ClassFlowImage(TAG)
{
    SetInitialParameter();
}

ClassFlowDigit::ClassFlowDigit(std::vector<ClassFlow*>* lfc) : ClassFlowImage(lfc, TAG)
{
    SetInitialParameter();
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowAlignment") == 0)
        {
            flowpostalignment = (ClassFlowAlignment*) (*ListFlowControll)[i];
        }
    }
}

ClassFlowDigit::ClassFlowDigit(std::vector<ClassFlow*>* lfc, ClassFlow *_prev) : ClassFlowImage(lfc, _prev, TAG)
{
    SetInitialParameter();
    ListFlowControll = lfc;
    previousElement = _prev;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowAlignment") == 0)
        {
            flowpostalignment = (ClassFlowAlignment*) (*ListFlowControll)[i];
        }
    }    
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

    printf("aktparamgraph: %s\n", aktparamgraph.c_str());


/*
    if ((aktparamgraph.compare("[Digits]") != 0) && (aktparamgraph.compare(";[Digits]") != 0))       // Paragraph passt nich zu MakeImage
        return false;
*/

    if ((aktparamgraph.compare(0, 7, "[Digits") != 0) && (aktparamgraph.compare(0, 8, ";[Digits") != 0))       // Paragraph passt nich zu MakeImage
        return false;

    int _pospkt = aktparamgraph.find_first_of(".");
    int _posklammerzu = aktparamgraph.find_first_of("]");
//    printf("Pos: %d, %d\n", _pospkt, _posklammerzu);
    if (_pospkt > -1)
        NameDigit = aktparamgraph.substr(_pospkt+1, _posklammerzu - _pospkt-1);
    else
        NameDigit = "";
    printf("Name Digit: %s\n", NameDigit.c_str());

    if (aktparamgraph[0] == ';')
    {
        disabled = true;
        while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph));
        printf("[Digits] is disabled !!!\n");
        return true;
    }


    while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] == "LogImageLocation") && (zerlegt.size() > 1))
        {
            LogImageLocation = "/sdcard" + zerlegt[1];
            isLogImage = true;            
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
            neuroi->resultklasse = -1;
            neuroi->image = NULL;
            neuroi->image_org = NULL;            
            ROI.push_back(neuroi);
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }

    }

    for (int i = 0; i < ROI.size(); ++i)
    {
        ROI[i]->image = new CImageBasis(modelxsize, modelysize, 3);
        ROI[i]->image_org = new CImageBasis(ROI[i]->deltax, ROI[i]->deltay, 3);
    }

    return true;
}


string ClassFlowDigit::getHTMLSingleStep(string host)
{
    string result, zw;
    std::vector<HTMLInfo*> htmlinfo;

    result = "<p>Found ROIs: </p> <p><img src=\"" + host + "/img_tmp/alg_roi.jpg\"></p>\n";
    result = result + "Digital Counter: <p> ";

    htmlinfo = GetHTMLInfo();
    for (int i = 0; i < htmlinfo.size(); ++i)
    {
        if (htmlinfo[i]->val == 10)
            zw = "NaN";
        else
        {
            zw = to_string((int) htmlinfo[i]->val);
        }
        result = result + "<img src=\"" + host + "/img_tmp/" +  htmlinfo[i]->filename + "\"> " + zw;
        delete htmlinfo[i];
    }
    htmlinfo.clear();    

    return result;
}


bool ClassFlowDigit::doFlow(string time)
{
    if (disabled)
        return true;
        
    if (!doAlignAndCut(time)){
        return false;
    };

    doNeuralNetwork(time);

    RemoveOldLogs();

    return true;
}

bool ClassFlowDigit::doAlignAndCut(string time)
{
    if (disabled)
        return true;

    CAlignAndCutImage *caic = flowpostalignment->GetAlignAndCutImage();

    for (int i = 0; i < ROI.size(); ++i)
    {
        printf("DigitalDigit %d - Align&Cut\n", i);
        
        caic->CutAndSave(ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay, ROI[i]->image_org);
        if (SaveAllFiles) ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ROI[i]->name + ".jpg"));

        ROI[i]->image_org->Resize(modelxsize, modelysize, ROI[i]->image);
        if (SaveAllFiles) ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ROI[i]->name + ".bmp"));
    }

    return true;
} 

bool ClassFlowDigit::doNeuralNetwork(string time)
{
    if (disabled)
        return true;
            
    string logPath = CreateLogFolder(time);

#ifndef OHNETFLITE
    CTfLiteClass *tflite = new CTfLiteClass;  
    string zwcnn =  FormatFileName("/sdcard" + cnnmodelfile);
    printf(zwcnn.c_str());printf("\n");
    if (!tflite->LoadModel(zwcnn)) {
        printf("Can't read model file /sdcard%s\n", cnnmodelfile.c_str());
        return false;
    } 

    tflite->MakeAllocate();
#endif

    for (int i = 0; i < ROI.size(); ++i)
    {
        printf("DigitalDigit %d - TfLite\n", i);

        ROI[i]->resultklasse = 0;
#ifndef OHNETFLITE
        ROI[i]->resultklasse = tflite->GetClassFromImageBasis(ROI[i]->image);

#endif
        printf("Result Digit%i: %d\n", i, ROI[i]->resultklasse);

        if (isLogImage)
        {
            LogImage(logPath, ROI[i]->name, NULL, &ROI[i]->resultklasse, time, ROI[i]->image_org);
        }
    }
#ifndef OHNETFLITE
        delete tflite;
#endif
    return true;
}

void ClassFlowDigit::DrawROI(CImageBasis *_zw)
{
    for (int i = 0; i < ROI.size(); ++i)
        _zw->drawRect(ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay, 0, 0, 255, 2);
}     

std::vector<HTMLInfo*> ClassFlowDigit::GetHTMLInfo()
{
    std::vector<HTMLInfo*> result;

    for (int i = 0; i < ROI.size(); ++i)
    {
        HTMLInfo *zw = new HTMLInfo;
        zw->filename = ROI[i]->name + ".bmp";
        zw->filename_org = ROI[i]->name + ".jpg";
        zw->val = ROI[i]->resultklasse;
        zw->image = ROI[i]->image;
        zw->image_org = ROI[i]->image_org;
        result.push_back(zw);
    }

    return result;
}

