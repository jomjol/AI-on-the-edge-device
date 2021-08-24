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
    isLogImageSelect = false;
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

string ClassFlowDigit::getReadout(int _digit = 0)
{
    string rst = "";

    for (int i = 0; i < DIGIT[_digit]->ROI.size(); ++i)
    {
        if (DIGIT[_digit]->ROI[i]->resultklasse == 10)
            rst = rst + "N";
        else
            rst = rst + std::to_string(DIGIT[_digit]->ROI[i]->resultklasse);
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

    if ((aktparamgraph.compare(0, 7, "[Digits") != 0) && (aktparamgraph.compare(0, 8, ";[Digits") != 0))       // Paragraph passt nich zu MakeImage
        return false;

    int _pospkt = aktparamgraph.find_first_of(".");
    int _posklammerzu = aktparamgraph.find_first_of("]");
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

        if ((zerlegt[0] == "LogImageSelect") && (zerlegt.size() > 1))
        {
            LogImageSelect = zerlegt[1];
            isLogImageSelect = true;            
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
            digit* _digit = GetDIGIT(zerlegt[0], true);
            roi* neuroi = _digit->ROI[_digit->ROI.size()-1];
            neuroi->posx = std::stoi(zerlegt[1]);
            neuroi->posy = std::stoi(zerlegt[2]);
            neuroi->deltax = std::stoi(zerlegt[3]);
            neuroi->deltay = std::stoi(zerlegt[4]);
            neuroi->resultklasse = -1;
            neuroi->image = NULL;
            neuroi->image_org = NULL;            
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }

    }

   for (int _dig = 0; _dig < DIGIT.size(); ++_dig)
        for (int i = 0; i < DIGIT[_dig]->ROI.size(); ++i)
        {
            DIGIT[_dig]->ROI[i]->image = new CImageBasis(modelxsize, modelysize, 3);
            DIGIT[_dig]->ROI[i]->image_org = new CImageBasis(DIGIT[_dig]->ROI[i]->deltax, DIGIT[_dig]->ROI[i]->deltay, 3);
        }

    return true;
}

digit* ClassFlowDigit::FindDIGIT(string _name_number)
{
    for (int i = 0; i < DIGIT.size(); ++i)
    {
        if (DIGIT[i]->name == _name_number)
            return DIGIT[i];
    }

    return NULL;
}


digit* ClassFlowDigit::GetDIGIT(string _name, bool _create = true)
{
    string _digit, _roi;
    int _pospunkt = _name.find_first_of(".");
//    printf("Name: %s, Pospunkt: %d\n", _name.c_str(), _pospunkt);
    if (_pospunkt > -1)
    {
        _digit = _name.substr(0, _pospunkt);
        _roi = _name.substr(_pospunkt+1, _name.length() - _pospunkt - 1);
    }
    else
    {
        _digit = "default";
        _roi = _name;
    }

    digit *_ret = NULL;

    for (int i = 0; i < DIGIT.size(); ++i)
    {
        if (DIGIT[i]->name == _digit)
            _ret = DIGIT[i];
    }

    if (!_create)         // nicht gefunden und soll auch nicht erzeugt werden, ggf. geht eine NULL zurück
        return _ret;

    if (_ret == NULL)
    {
        _ret = new digit;
        _ret->name = _digit;
        DIGIT.push_back(_ret);
    }

    roi* neuroi = new roi;
    neuroi->name = _roi;
    _ret->ROI.push_back(neuroi);

    printf("GetDIGIT - digit %s - roi %s\n", _digit.c_str(), _roi.c_str());

    return _ret;
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

    for (int _dig = 0; _dig < DIGIT.size(); ++_dig)
    {
        printf("DIGIT[_dig]->ROI.size() %d\n", DIGIT[_dig]->ROI.size());
        for (int i = 0; i < DIGIT[_dig]->ROI.size(); ++i)
        {
            printf("DigitalDigit %d - Align&Cut\n", i);
            
            caic->CutAndSave(DIGIT[_dig]->ROI[i]->posx, DIGIT[_dig]->ROI[i]->posy, DIGIT[_dig]->ROI[i]->deltax, DIGIT[_dig]->ROI[i]->deltay, DIGIT[_dig]->ROI[i]->image_org);
            if (SaveAllFiles)
            {
                if (DIGIT[_dig]->name == "default")
                    DIGIT[_dig]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + DIGIT[_dig]->ROI[i]->name + ".jpg"));
                else
                    DIGIT[_dig]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + DIGIT[_dig]->name + "_" + DIGIT[_dig]->ROI[i]->name + ".jpg"));
            } 

            DIGIT[_dig]->ROI[i]->image_org->Resize(modelxsize, modelysize, DIGIT[_dig]->ROI[i]->image);
            if (SaveAllFiles)
            {
                if (DIGIT[_dig]->name == "default")
                    DIGIT[_dig]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + DIGIT[_dig]->ROI[i]->name + ".bmp"));
                else
                    DIGIT[_dig]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + DIGIT[_dig]->name + "_" + DIGIT[_dig]->ROI[i]->name + ".bmp"));
            } 
        }
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
        delete tflite;
        return false;
    } 

    tflite->MakeAllocate();
#endif

    for (int _dig = 0; _dig < DIGIT.size(); ++_dig)
        for (int i = 0; i < DIGIT[_dig]->ROI.size(); ++i)
        {
            printf("DigitalDigit %d - TfLite\n", i);

            DIGIT[_dig]->ROI[i]->resultklasse = 0;
    #ifndef OHNETFLITE
            DIGIT[_dig]->ROI[i]->resultklasse = tflite->GetClassFromImageBasis(DIGIT[_dig]->ROI[i]->image);

    #endif
            printf("Result Digit%i: %d\n", i, DIGIT[_dig]->ROI[i]->resultklasse);

            if (isLogImage)
            {
                if (isLogImageSelect)
                {
                    if (LogImageSelect.find(DIGIT[_dig]->ROI[i]->name) != std::string::npos)
                    {
                        LogImage(logPath, DIGIT[_dig]->ROI[i]->name, NULL, &DIGIT[_dig]->ROI[i]->resultklasse, time, DIGIT[_dig]->ROI[i]->image_org);
                    }
                }
                else
                {
                    LogImage(logPath, DIGIT[_dig]->ROI[i]->name, NULL, &DIGIT[_dig]->ROI[i]->resultklasse, time, DIGIT[_dig]->ROI[i]->image_org);
                }
            }
        }
#ifndef OHNETFLITE
        delete tflite;
#endif
    return true;
}

void ClassFlowDigit::DrawROI(CImageBasis *_zw)
{
    for (int _dig = 0; _dig < DIGIT.size(); ++_dig)
        for (int i = 0; i < DIGIT[_dig]->ROI.size(); ++i)
            _zw->drawRect(DIGIT[_dig]->ROI[i]->posx, DIGIT[_dig]->ROI[i]->posy, DIGIT[_dig]->ROI[i]->deltax, DIGIT[_dig]->ROI[i]->deltay, 0, 0, (255 - _dig*100), 2);
}     

std::vector<HTMLInfo*> ClassFlowDigit::GetHTMLInfo()
{
    std::vector<HTMLInfo*> result;

    for (int _dig = 0; _dig < DIGIT.size(); ++_dig)
        for (int i = 0; i < DIGIT[_dig]->ROI.size(); ++i)
        {
                if (DIGIT[_dig]->name == "default")
                    DIGIT[_dig]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + DIGIT[_dig]->ROI[i]->name + ".bmp"));
                else
                    DIGIT[_dig]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + DIGIT[_dig]->name + "_" + DIGIT[_dig]->ROI[i]->name + ".bmp"));


            HTMLInfo *zw = new HTMLInfo;
            if (DIGIT[_dig]->name == "default")
            {
                zw->filename = DIGIT[_dig]->ROI[i]->name + ".bmp";
                zw->filename_org = DIGIT[_dig]->ROI[i]->name + ".jpg";
            }
            else
            {
                zw->filename = DIGIT[_dig]->name + "_" + DIGIT[_dig]->ROI[i]->name + ".bmp";
                zw->filename_org = DIGIT[_dig]->name + "_" + DIGIT[_dig]->ROI[i]->name + ".jpg";
            }

            zw->val = DIGIT[_dig]->ROI[i]->resultklasse;
            zw->image = DIGIT[_dig]->ROI[i]->image;
            zw->image_org = DIGIT[_dig]->ROI[i]->image_org;
            result.push_back(zw);
        }

    return result;
}

int ClassFlowDigit::getAnzahlDIGIT()
{
    return DIGIT.size();
}

string ClassFlowDigit::getNameDIGIT(int _digit)
{
    if (_digit < DIGIT.size())
        return DIGIT[_digit]->name;

    return "DIGIT DOES NOT EXIST";
}

digit* ClassFlowDigit::GetDIGIT(int _digit)
{
    if (_digit < DIGIT.size())
        return DIGIT[_digit];

    return NULL;
}

void ClassFlowDigit::UpdateNameNumbers(std::vector<std::string> *_name_numbers)
{
    for (int _dig = 0; _dig < DIGIT.size(); _dig++)
    {
        std::string _name = DIGIT[_dig]->name;
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




