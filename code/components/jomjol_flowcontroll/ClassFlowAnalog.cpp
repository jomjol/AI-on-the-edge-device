#include "ClassFlowAnalog.h"

#include <math.h>
#include <iomanip>
#include <sys/types.h>

// #define OHNETFLITE

#ifndef OHNETFLITE
#include "CTfLiteClass.h"
#endif

#include "ClassLogFile.h"

static const char* TAG = "flow_analog";

bool debugdetailanalog = false;

void ClassFlowAnalog::SetInitialParameter(void)
{
    string cnnmodelfile = "";
    modelxsize = 1;
    modelysize = 1;
    ListFlowControll = NULL;
    previousElement = NULL;   
    SaveAllFiles = false; 
}   

ClassFlowAnalog::ClassFlowAnalog(std::vector<ClassFlow*>* lfc) : ClassFlowImage(lfc, TAG)
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


string ClassFlowAnalog::getReadout()
{
    int prev = -1;
    string result = "";
    for (int i = ROI.size() - 1; i >= 0; --i)
    {
        prev = ZeigerEval(ROI[i]->result, prev);
        result = std::to_string(prev) + result;
    }
    return result;
}

int ClassFlowAnalog::ZeigerEval(float zahl, int ziffer_vorgaenger)
{
    int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;
    int ergebnis_vorkomma = ((int) floor(zahl)) % 10;
    int ergebnis, ergebnis_rating;

    if (ziffer_vorgaenger == -1)
        return ergebnis_vorkomma % 10;

    ergebnis_rating = ergebnis_nachkomma - ziffer_vorgaenger;
    if (ergebnis_nachkomma >= 5)
        ergebnis_rating-=5;
    else
        ergebnis_rating+=5;
    ergebnis = (int) round(zahl);
    if (ergebnis_rating < 0)
        ergebnis-=1;
    if (ergebnis == -1)
        ergebnis+=10;

    ergebnis = ergebnis % 10;
    return ergebnis;
}

bool ClassFlowAnalog::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if (aktparamgraph.compare("[Analog]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] == "LogImageLocation") && (zerlegt.size() > 1))
        {
            this->LogImageLocation = "/sdcard" + zerlegt[1];
            this->isLogImage = true;
        }
        if ((toUpper(zerlegt[0]) == "LOGFILERETENTIONINDAYS") && (zerlegt.size() > 1))
        {
            this->logfileRetentionInDays = std::stoi(zerlegt[1]);
        }
        if ((zerlegt[0] == "Model") && (zerlegt.size() > 1))
        {
            this->cnnmodelfile = zerlegt[1];
        }
        if ((zerlegt[0] == "ModelInputSize") && (zerlegt.size() > 2))
        {
            this->modelxsize = std::stoi(zerlegt[1]);
            this->modelysize = std::stoi(zerlegt[2]);
        }
        if (zerlegt.size() >= 5)
        {
            roianalog* neuroi = new roianalog;
            neuroi->name = zerlegt[0];
            neuroi->posx = std::stoi(zerlegt[1]);
            neuroi->posy = std::stoi(zerlegt[2]);
            neuroi->deltax = std::stoi(zerlegt[3]);
            neuroi->deltay = std::stoi(zerlegt[4]);
            neuroi->result = -1;
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
    return true;
}


string ClassFlowAnalog::getHTMLSingleStep(string host)
{
    string result, zw;
    std::vector<HTMLInfo*> htmlinfo;

    result = "<p>Found ROIs: </p> <p><img src=\"" + host + "/img_tmp/alg_roi.jpg\"></p>\n";
    result = result + "Analog Pointers: <p> ";

    htmlinfo = GetHTMLInfo();
    for (int i = 0; i < htmlinfo.size(); ++i)
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << htmlinfo[i]->val;
        zw = stream.str();

        result = result + "<img src=\"" + host + "/img_tmp/" +  htmlinfo[i]->filename + "\"> " + zw;
        delete htmlinfo[i];
    }
    htmlinfo.clear();         

    return result;
}



bool ClassFlowAnalog::doFlow(string time)
{
    if (!doAlignAndCut(time)){
        return false;
    };

    if (debugdetailanalog) LogFile.WriteToFile("ClassFlowAnalog::doFlow nach Alignment");

    doNeuralNetwork(time);

    RemoveOldLogs();

    return true;
}

bool ClassFlowAnalog::doAlignAndCut(string time)
{
    CAlignAndCutImage *caic = flowpostalignment->GetAlignAndCutImage();    

    for (int i = 0; i < ROI.size(); ++i)
    {
        printf("Analog %d - Align&Cut\n", i);

        CResizeImage *rs = caic->CutAndSave(ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay);
        if (ROI[i]->image_org)
            delete ROI[i]->image_org;
        ROI[i]->image_org = new CImageBasis((CImageBasis*) rs);

        if (SaveAllFiles) rs->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ROI[i]->name + ".jpg"));

        rs->Resize(modelxsize, modelysize);
        if (SaveAllFiles) rs->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ROI[i]->name + ".bmp"));
        if (ROI[i]->image)
            delete ROI[i]->image;
        ROI[i]->image = rs;
    }

    return true;
} 

void ClassFlowAnalog::DrawROI(CImageBasis *_zw)
{
    int r = 0;
    int g = 255;
    int b = 0;

    for (int i = 0; i < ROI.size(); ++i)
    {
        _zw->drawRect(ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay, r, g, b, 1);
        _zw->drawCircle((int) (ROI[i]->posx + ROI[i]->deltax/2), (int)  (ROI[i]->posy + ROI[i]->deltay/2), (int) (ROI[i]->deltax/2), r, g, b, 2);
        _zw->drawLine((int) (ROI[i]->posx + ROI[i]->deltax/2), (int) ROI[i]->posy, (int) (ROI[i]->posx + ROI[i]->deltax/2), (int) (ROI[i]->posy + ROI[i]->deltay), r, g, b, 2);
        _zw->drawLine((int) ROI[i]->posx, (int) (ROI[i]->posy + ROI[i]->deltay/2), (int) ROI[i]->posx + ROI[i]->deltax, (int) (ROI[i]->posy + ROI[i]->deltay/2), r, g, b, 2);
    }
} 

bool ClassFlowAnalog::doNeuralNetwork(string time)
{
    string logPath = CreateLogFolder(time);
    
    string input = "/sdcard/img_tmp/alg.jpg";
    string ioresize = "/sdcard/img_tmp/resize.bmp";
    string output;
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
        printf("Analog %d - TfLite\n", i);
        ioresize = "/sdcard/img_tmp/ra" + std::to_string(i) + ".bmp";
        ioresize = FormatFileName(ioresize);


        float f1, f2;
        f1 = 0; f2 = 0;

#ifndef OHNETFLITE
//        LogFile.WriteToFile("ClassFlowAnalog::doNeuralNetwork vor CNN tflite->LoadInputImage(ioresize)");
//        tflite->LoadInputImage(ioresize);
        tflite->LoadInputImageBasis(ROI[i]->image);        
        tflite->Invoke();
        if (debugdetailanalog) LogFile.WriteToFile("Nach Invoke");


        f1 = tflite->GetOutputValue(0);
        f2 = tflite->GetOutputValue(1);
#endif

        float result = fmod(atan2(f1, f2) / (M_PI * 2) + 2, 1);
//        printf("Result sin, cos, ziffer: %f, %f, %f\n", f1, f2, result);  
        ROI[i]->result = result * 10;

        printf("Result Analog%i: %f\n", i, ROI[i]->result); 

        if (isLogImage)
        {
            LogImage(logPath, ROI[i]->name, &ROI[i]->result, NULL, time, ROI[i]->image_org);
        }
    }
#ifndef OHNETFLITE
        delete tflite;
#endif    

    return true;
}


std::vector<HTMLInfo*> ClassFlowAnalog::GetHTMLInfo()
{
    std::vector<HTMLInfo*> result;

    for (int i = 0; i < ROI.size(); ++i)
    {
        HTMLInfo *zw = new HTMLInfo;
        zw->filename = ROI[i]->name + ".jpg";
        zw->val = ROI[i]->result;
        result.push_back(zw);
    }

    return result;
}


