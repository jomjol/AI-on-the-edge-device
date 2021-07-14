#include "ClassFlowAnalog.h"

#include <math.h>
#include <iomanip> 
#include <sys/types.h>
#include <sstream>      // std::stringstream

  
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
    disabled = false;
    extendedResolution = false;
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


int ClassFlowAnalog::AnzahlROIs(int _analog = 0)
{
    int zw = ANALOG[_analog]->ROI.size();
    if (extendedResolution)
        zw++;
    
    return zw;
} 


string ClassFlowAnalog::getReadout(int _analog = 0)
{
    string result = "";    
    if (ANALOG[_analog]->ROI.size() == 0)
        return result;


    float zahl = ANALOG[_analog]->ROI[ANALOG[_analog]->ROI.size() - 1]->result;
    int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;

    int prev = -1;

    prev = ZeigerEval(ANALOG[_analog]->ROI[ANALOG[_analog]->ROI.size() - 1]->result, prev);
    result = std::to_string(prev);

    if (extendedResolution)
        result = result + std::to_string(ergebnis_nachkomma);

    for (int i = ANALOG[_analog]->ROI.size() - 2; i >= 0; --i)
    {
        prev = ZeigerEval(ANALOG[_analog]->ROI[i]->result, prev);
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


    if ((aktparamgraph.compare("[Analog]") != 0) && (aktparamgraph.compare(";[Analog]") != 0))       // Paragraph passt nich zu MakeImage
        return false;

    if (aktparamgraph[0] == ';')
    {
        disabled = true;
        while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph));
        printf("[Analog] is disabled !!!\n");
        return true;
    }


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
            analog* _analog = GetANALOG(zerlegt[0], true);
            roianalog* neuroi = _analog->ROI[_analog->ROI.size()-1];
            neuroi->posx = std::stoi(zerlegt[1]);
            neuroi->posy = std::stoi(zerlegt[2]);
            neuroi->deltax = std::stoi(zerlegt[3]);
            neuroi->deltay = std::stoi(zerlegt[4]);
            neuroi->result = -1;
            neuroi->image = NULL;
            neuroi->image_org = NULL;
//            ROI.push_back(neuroi);
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }

        if ((toUpper(zerlegt[0]) == "EXTENDEDRESOLUTION") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                extendedResolution = true;
        }
    }

   for (int _ana = 0; _ana < ANALOG.size(); ++_ana)
        for (int i = 0; i < ANALOG[_ana]->ROI.size(); ++i)
        {
            ANALOG[_ana]->ROI[i]->image = new CImageBasis(modelxsize, modelysize, 3);
            ANALOG[_ana]->ROI[i]->image_org = new CImageBasis(ANALOG[_ana]->ROI[i]->deltax, ANALOG[_ana]->ROI[i]->deltay, 3);
        }

    return true;
}

analog* ClassFlowAnalog::FindANALOG(string _name_number)
{

    for (int i = 0; i < ANALOG.size(); ++i)
    {
        if (ANALOG[i]->name == _name_number)
            return ANALOG[i];
    }

    return NULL;
}



analog* ClassFlowAnalog::GetANALOG(string _name, bool _create = true)
{
    string _analog, _roi;
    int _pospunkt = _name.find_first_of(".");
//    printf("Name: %s, Pospunkt: %d\n", _name.c_str(), _pospunkt);
    if (_pospunkt > -1)
    {
        _analog = _name.substr(0, _pospunkt);
        _roi = _name.substr(_pospunkt+1, _name.length() - _pospunkt - 1);
    }
    else
    {
        _analog = "default";
        _roi = _name;
    }

    analog *_ret = NULL;

    for (int i = 0; i < ANALOG.size(); ++i)
    {
        if (ANALOG[i]->name == _analog)
            _ret = ANALOG[i];
    }

    if (!_create)         // nicht gefunden und soll auch nicht erzeugt werden
        return _ret;


    if (_ret == NULL)
    {
        _ret = new analog;
        _ret->name = _analog;
        ANALOG.push_back(_ret);
    }

    roianalog* neuroi = new roianalog;
    neuroi->name = _roi;
    _ret->ROI.push_back(neuroi);

    printf("GetANALOG - ANALOG %s - roi %s\n", _analog.c_str(), _roi.c_str());

    return _ret;
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
    if (disabled)
      return true;

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
    if (disabled)
        return true;

    CAlignAndCutImage *caic = flowpostalignment->GetAlignAndCutImage();    

    for (int _ana = 0; _ana < ANALOG.size(); ++_ana)
        for (int i = 0; i < ANALOG[_ana]->ROI.size(); ++i)
        {
            printf("Analog %d - Align&Cut\n", i);
            
            caic->CutAndSave(ANALOG[_ana]->ROI[i]->posx, ANALOG[_ana]->ROI[i]->posy, ANALOG[_ana]->ROI[i]->deltax, ANALOG[_ana]->ROI[i]->deltay, ANALOG[_ana]->ROI[i]->image_org);
            if (SaveAllFiles)
            {
                if (ANALOG[_ana]->name == "default")
                    ANALOG[_ana]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ANALOG[_ana]->ROI[i]->name + ".jpg"));
                else
                    ANALOG[_ana]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ANALOG[_ana]->name + "_" + ANALOG[_ana]->ROI[i]->name + ".jpg"));
            } 

            ANALOG[_ana]->ROI[i]->image_org->Resize(modelxsize, modelysize, ANALOG[_ana]->ROI[i]->image);
            if (SaveAllFiles)
            {
                if (ANALOG[_ana]->name == "default")
                    ANALOG[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ANALOG[_ana]->ROI[i]->name + ".bmp"));
                else
                    ANALOG[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ANALOG[_ana]->name + "_" + ANALOG[_ana]->ROI[i]->name + ".bmp"));
            } 
        }

    return true;
} 

void ClassFlowAnalog::DrawROI(CImageBasis *_zw)
{
    int r = 0;
    int g = 255;
    int b = 0;

    for (int _ana = 0; _ana < ANALOG.size(); ++_ana)
        for (int i = 0; i < ANALOG[_ana]->ROI.size(); ++i)
        {
            _zw->drawRect(ANALOG[_ana]->ROI[i]->posx, ANALOG[_ana]->ROI[i]->posy, ANALOG[_ana]->ROI[i]->deltax, ANALOG[_ana]->ROI[i]->deltay, r, g, b, 1);
            _zw->drawCircle((int) (ANALOG[_ana]->ROI[i]->posx + ANALOG[_ana]->ROI[i]->deltax/2), (int)  (ANALOG[_ana]->ROI[i]->posy + ANALOG[_ana]->ROI[i]->deltay/2), (int) (ANALOG[_ana]->ROI[i]->deltax/2), r, g, b, 2);
            _zw->drawLine((int) (ANALOG[_ana]->ROI[i]->posx + ANALOG[_ana]->ROI[i]->deltax/2), (int) ANALOG[_ana]->ROI[i]->posy, (int) (ANALOG[_ana]->ROI[i]->posx + ANALOG[_ana]->ROI[i]->deltax/2), (int) (ANALOG[_ana]->ROI[i]->posy + ANALOG[_ana]->ROI[i]->deltay), r, g, b, 2);
            _zw->drawLine((int) ANALOG[_ana]->ROI[i]->posx, (int) (ANALOG[_ana]->ROI[i]->posy + ANALOG[_ana]->ROI[i]->deltay/2), (int) ANALOG[_ana]->ROI[i]->posx + ANALOG[_ana]->ROI[i]->deltax, (int) (ANALOG[_ana]->ROI[i]->posy + ANALOG[_ana]->ROI[i]->deltay/2), r, g, b, 2);
        }
} 

bool ClassFlowAnalog::doNeuralNetwork(string time)
{
    if (disabled)
        return true;

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
    if (!tflite->LoadModel(zwcnn)) {
        printf("Can't read model file /sdcard%s\n", cnnmodelfile.c_str());
        delete tflite;
        return false;
    } 
    tflite->MakeAllocate();
#endif

    for (int _ana = 0; _ana < ANALOG.size(); ++_ana)
    {
        for (int i = 0; i < ANALOG[_ana]->ROI.size(); ++i)
        {
            printf("Analog %d - TfLite\n", i);

            float f1, f2;
            f1 = 0; f2 = 0;

    #ifndef OHNETFLITE
            tflite->LoadInputImageBasis(ANALOG[_ana]->ROI[i]->image);        
            tflite->Invoke();
            if (debugdetailanalog) LogFile.WriteToFile("Nach Invoke");


            f1 = tflite->GetOutputValue(0);
            f2 = tflite->GetOutputValue(1);
    #endif

            float result = fmod(atan2(f1, f2) / (M_PI * 2) + 2, 1);
    //        printf("Result sin, cos, ziffer: %f, %f, %f\n", f1, f2, result);  
            ANALOG[_ana]->ROI[i]->result = result * 10;

            printf("Result Analog%i: %f\n", i, ANALOG[_ana]->ROI[i]->result); 

            if (isLogImage)
            {
                LogImage(logPath, ANALOG[_ana]->ROI[i]->name, &ANALOG[_ana]->ROI[i]->result, NULL, time, ANALOG[_ana]->ROI[i]->image_org);
            }
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

    for (int _ana = 0; _ana < ANALOG.size(); ++_ana)
        for (int i = 0; i < ANALOG[_ana]->ROI.size(); ++i)
        {
                if (ANALOG[_ana]->name == "default")
                    ANALOG[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ANALOG[_ana]->ROI[i]->name + ".bmp"));
                else
                    ANALOG[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ANALOG[_ana]->name + "_" + ANALOG[_ana]->ROI[i]->name + ".bmp"));


            HTMLInfo *zw = new HTMLInfo;
            if (ANALOG[_ana]->name == "default")
            {
                zw->filename = ANALOG[_ana]->ROI[i]->name + ".bmp";
                zw->filename_org = ANALOG[_ana]->ROI[i]->name + ".jpg";
            }
            else
            {
                zw->filename = ANALOG[_ana]->name + "_" + ANALOG[_ana]->ROI[i]->name + ".bmp";
                zw->filename_org = ANALOG[_ana]->name + "_" + ANALOG[_ana]->ROI[i]->name + ".jpg";
            }

            zw->val = ANALOG[_ana]->ROI[i]->result;
            zw->image = ANALOG[_ana]->ROI[i]->image;
            zw->image_org = ANALOG[_ana]->ROI[i]->image_org;

            result.push_back(zw);
        }

    return result;
}



int ClassFlowAnalog::getAnzahlANALOG()
{
    return ANALOG.size();
}

string ClassFlowAnalog::getNameANALOG(int _analog)
{
    if (_analog < ANALOG.size())
        return ANALOG[_analog]->name;

    return "ANALOG DOES NOT EXIST";
}

analog* ClassFlowAnalog::GetANALOG(int _analog)
{
    if (_analog < ANALOG.size())
        return ANALOG[_analog];

    return NULL;
}



void ClassFlowAnalog::UpdateNameNumbers(std::vector<std::string> *_name_numbers)
{
    for (int _dig = 0; _dig < ANALOG.size(); _dig++)
    {
        std::string _name = ANALOG[_dig]->name;
        bool found = false;
        for (int i = 0; i < (*_name_numbers).size(); ++i)
        {
            if ((*_name_numbers)[i] == _name)
                found = true;
        }
        if (!found)
            (*_name_numbers).push_back(_name);
    }
}




