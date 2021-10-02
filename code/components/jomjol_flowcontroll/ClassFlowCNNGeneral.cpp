#include "ClassFlowCNNGeneral.h"

#include <math.h>
#include <iomanip> 
#include <sys/types.h>
#include <sstream>      // std::stringstream

#include "CTfLiteClass.h"
#include "ClassLogFile.h"

static const char* TAG = "flow_analog";

bool debugdetailgeneral = false;

ClassFlowCNNGeneral::ClassFlowCNNGeneral(ClassFlowAlignment *_flowalign, t_CNNType _cnntype) : ClassFlowImage(NULL, TAG)
{
    string cnnmodelfile = "";
    modelxsize = 1;
    modelysize = 1;
    ListFlowControll = NULL;
    previousElement = NULL;   
    SaveAllFiles = false; 
    disabled = false;
//    extendedResolution = false;
    isLogImageSelect = false;
    CNNType = AutoDetect;
    CNNType = _cnntype;
    flowpostalignment = _flowalign;
}

/*
int ClassFlowCNNGeneral::AnzahlROIs(int _analog = 0)
{
    int zw = GENERAL[_analog]->ROI.size();
    if (extendedResolution && (CNNType != Digital)) zw++;   // da letzte Ziffer inkl Nachhkomma, es sei denn, das Nachkomma gibt es nicht (Digital)
    return zw;
} 
*/

string ClassFlowCNNGeneral::getReadout(int _analog = 0, bool _extendedResolution = false)
{
    string result = "";    
    if (GENERAL[_analog]->ROI.size() == 0)
        return result;

    if (CNNType == Analogue)
    {
        float zahl = GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float;
        int ergebnis_nachkomma = ((int) floor(zahl * 10) + 10) % 10;

        int prev = -1;

        prev = ZeigerEval(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, prev);
        result = std::to_string(prev);

        if (_extendedResolution && (CNNType != Digital))
            result = result + std::to_string(ergebnis_nachkomma);

        for (int i = GENERAL[_analog]->ROI.size() - 2; i >= 0; --i)
        {
            prev = ZeigerEval(GENERAL[_analog]->ROI[i]->result_float, prev);
            result = std::to_string(prev) + result;
        }
        return result;
    }

    if (CNNType == Digital)
    {
        for (int i = 0; i < GENERAL[_analog]->ROI.size(); ++i)
        {
            if (GENERAL[_analog]->ROI[i]->result_klasse >= 10)
                result = result + "N";
            else
                result = result + std::to_string(GENERAL[_analog]->ROI[i]->result_klasse);
        }
        return result;
    }

    if (CNNType == DigitalHyprid)
    {
//        int ergebnis_nachkomma = -1;
        int zif_akt = -1;

        float zahl = GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float;
        if (zahl >= 0)       // NaN?
        {
            if (_extendedResolution)
            {
                int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;
                int ergebnis_vorkomma = ((int) floor(zahl)) % 10;

                result = std::to_string(ergebnis_vorkomma) + std::to_string(ergebnis_nachkomma);
                zif_akt = ergebnis_vorkomma;
            }
            else
            {
                zif_akt = ZeigerEvalHybrid(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, -1, -1);
                result = std::to_string(zif_akt);
            }
        }
        else
        {
            result = "N";
            if (_extendedResolution && (CNNType != Digital))
                result = "NN";
        }

        for (int i = GENERAL[_analog]->ROI.size() - 2; i >= 0; --i)
        {
            if (GENERAL[_analog]->ROI[i]->result_float >= 0)
            {
                zif_akt = ZeigerEvalHybrid(GENERAL[_analog]->ROI[i]->result_float, GENERAL[_analog]->ROI[i+1]->result_float, zif_akt);
                result = std::to_string(zif_akt) + result;
            }
            else
            {
                zif_akt = -1;
                result = "N" + result;
            }
        }
        return result;
    }

    return result;
}

int ClassFlowCNNGeneral::ZeigerEvalHybrid(float zahl, float zahl_vorgaenger, int eval_vorgaenger)
{
    int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;
//    int ergebnis_vorkomma = ((int) floor(zahl)) % 10;

    if (zahl_vorgaenger < 0)                // keine Vorzahl vorhanden !!! --> Runde die Zahl
    {
        if ((ergebnis_nachkomma <= 2) || (ergebnis_nachkomma >= 8))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
            return ((int) round(zahl) + 10) % 10;
        else
            return ((int) trunc(zahl) + 10) % 10;
    }

    if (zahl_vorgaenger > 9.2)              // Ziffernwechsel beginnt
    {
        if (eval_vorgaenger == 0)           // Wechsel hat schon stattgefunden
        {
            return ((int) round(zahl) + 10) % 10;      // Annahme, dass die neue Zahl schon in der Nähe des Ziels ist
        }
        else
        {
            if (zahl_vorgaenger <= 9.5)     // Wechsel startet gerade, aber beginnt erst
            {
                if ((ergebnis_nachkomma <= 2) || (ergebnis_nachkomma >= 8))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
                    return ((int) round(zahl) + 10) % 10;
                else
                    return ((int) trunc(zahl) + 10) % 10;
            }
            else
            {
                return ((int) trunc(zahl) + 10) % 10;   // Wechsel schon weiter fortgeschritten, d.h. über 2 als Nachkomma
            }
        }
    }

    if ((ergebnis_nachkomma <= 2) || (ergebnis_nachkomma >= 8))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
        return ((int) round(zahl) + 10) % 10;

    return ((int) trunc(zahl) + 10) % 10;
}

int ClassFlowCNNGeneral::ZeigerEval(float zahl, int ziffer_vorgaenger)
{
    int ergebnis_nachkomma = ((int) floor(zahl * 10) + 10) % 10;
    int ergebnis_vorkomma = ((int) floor(zahl) + 10) % 10;
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

    ergebnis = (ergebnis + 10) % 10;
    return ergebnis;
}

bool ClassFlowCNNGeneral::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if ((toUpper(aktparamgraph) != "[ANALOG]") && (toUpper(aktparamgraph) != ";[ANALOG]") 
        && (toUpper(aktparamgraph) != "[DIGIT]") && (toUpper(aktparamgraph) != ";[DIGIT]")
        && (toUpper(aktparamgraph) != "[DIGITS]") && (toUpper(aktparamgraph) != ";[DIGITS]")
        )       // Paragraph passt nicht
        return false;


/*
    if ((aktparamgraph.compare("[Analog]") != 0) && (aktparamgraph.compare(";[Analog]") != 0) 
        && (aktparamgraph.compare("[Digit]") != 0) && (aktparamgraph.compare(";[Digit]")))       // Paragraph passt nicht
        return false;
*/

    if (aktparamgraph[0] == ';')
    {
        disabled = true;
        while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph));
        printf("[Analog/Digit] is disabled !!!\n");
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
        if ((zerlegt[0] == "LogImageSelect") && (zerlegt.size() > 1))
        {
            LogImageSelect = zerlegt[1];
            isLogImageSelect = true;            
        }

        if ((toUpper(zerlegt[0]) == "LOGFILERETENTIONINDAYS") && (zerlegt.size() > 1))
        {
            this->logfileRetentionInDays = std::stoi(zerlegt[1]);
        }
        if ((toUpper(zerlegt[0]) == "MODELTYPE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "DIGITHYPRID")
                CNNType = DigitalHyprid;
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
            general* _analog = GetGENERAL(zerlegt[0], true);
            roi* neuroi = _analog->ROI[_analog->ROI.size()-1];
            neuroi->posx = std::stoi(zerlegt[1]);
            neuroi->posy = std::stoi(zerlegt[2]);
            neuroi->deltax = std::stoi(zerlegt[3]);
            neuroi->deltay = std::stoi(zerlegt[4]);
            neuroi->result_float = -1;
            neuroi->image = NULL;
            neuroi->image_org = NULL;
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }

/*
        if ((toUpper(zerlegt[0]) == "EXTENDEDRESOLUTION") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                extendedResolution = true;
        }
*/
    }


   for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
            GENERAL[_ana]->ROI[i]->image = new CImageBasis(modelxsize, modelysize, 3);
            GENERAL[_ana]->ROI[i]->image_org = new CImageBasis(GENERAL[_ana]->ROI[i]->deltax, GENERAL[_ana]->ROI[i]->deltay, 3);
        }

    return true;
}

general* ClassFlowCNNGeneral::FindGENERAL(string _name_number)
{
    for (int i = 0; i < GENERAL.size(); ++i)
        if (GENERAL[i]->name == _name_number)
            return GENERAL[i];
    return NULL;
}



general* ClassFlowCNNGeneral::GetGENERAL(string _name, bool _create = true)
{
    string _analog, _roi;
    int _pospunkt = _name.find_first_of(".");

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

    general *_ret = NULL;

    for (int i = 0; i < GENERAL.size(); ++i)
        if (GENERAL[i]->name == _analog)
            _ret = GENERAL[i];

    if (!_create)         // nicht gefunden und soll auch nicht erzeugt werden
        return _ret;

    if (_ret == NULL)
    {
        _ret = new general;
        _ret->name = _analog;
        GENERAL.push_back(_ret);
    }

    roi* neuroi = new roi;
    neuroi->name = _roi;
    _ret->ROI.push_back(neuroi);

    printf("GetGENERAL - GENERAL %s - roi %s\n", _analog.c_str(), _roi.c_str());

    return _ret;
}



string ClassFlowCNNGeneral::getHTMLSingleStep(string host)
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



bool ClassFlowCNNGeneral::doFlow(string time)
{
    if (disabled)
      return true;

    if (!doAlignAndCut(time)){
        return false;
    };

    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::doFlow nach Alignment");

    doNeuralNetwork(time);

    RemoveOldLogs();
    return true;
}

bool ClassFlowCNNGeneral::doAlignAndCut(string time)
{
    if (disabled)
        return true;

    CAlignAndCutImage *caic = flowpostalignment->GetAlignAndCutImage();    

    for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
            printf("General %d - Align&Cut\n", i);
            
            caic->CutAndSave(GENERAL[_ana]->ROI[i]->posx, GENERAL[_ana]->ROI[i]->posy, GENERAL[_ana]->ROI[i]->deltax, GENERAL[_ana]->ROI[i]->deltay, GENERAL[_ana]->ROI[i]->image_org);
            if (SaveAllFiles)
            {
                if (GENERAL[_ana]->name == "default")
                    GENERAL[_ana]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
                else
                    GENERAL[_ana]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
            } 

            GENERAL[_ana]->ROI[i]->image_org->Resize(modelxsize, modelysize, GENERAL[_ana]->ROI[i]->image);
            if (SaveAllFiles)
            {
                if (GENERAL[_ana]->name == "default")
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->ROI[i]->name + ".bmp"));
                else
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".bmp"));
            } 
        }

    return true;
} 

void ClassFlowCNNGeneral::DrawROI(CImageBasis *_zw)
{
    if (CNNType == Analogue)
    {
        int r = 0;
        int g = 255;
        int b = 0;

        for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
            for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
            {
                _zw->drawRect(GENERAL[_ana]->ROI[i]->posx, GENERAL[_ana]->ROI[i]->posy, GENERAL[_ana]->ROI[i]->deltax, GENERAL[_ana]->ROI[i]->deltay, r, g, b, 1);
                _zw->drawCircle((int) (GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax/2), (int)  (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay/2), (int) (GENERAL[_ana]->ROI[i]->deltax/2), r, g, b, 2);
                _zw->drawLine((int) (GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax/2), (int) GENERAL[_ana]->ROI[i]->posy, (int) (GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax/2), (int) (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay), r, g, b, 2);
                _zw->drawLine((int) GENERAL[_ana]->ROI[i]->posx, (int) (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay/2), (int) GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax, (int) (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay/2), r, g, b, 2);
            }
    }
    else
    {
        for (int _dig = 0; _dig < GENERAL.size(); ++_dig)
            for (int i = 0; i < GENERAL[_dig]->ROI.size(); ++i)
                _zw->drawRect(GENERAL[_dig]->ROI[i]->posx, GENERAL[_dig]->ROI[i]->posy, GENERAL[_dig]->ROI[i]->deltax, GENERAL[_dig]->ROI[i]->deltay, 0, 0, (255 - _dig*100), 2);
    }
} 

bool ClassFlowCNNGeneral::doNeuralNetwork(string time)
{
    if (disabled)
        return true;

    string logPath = CreateLogFolder(time);

    CTfLiteClass *tflite = new CTfLiteClass;  
    string zwcnn = "/sdcard" + cnnmodelfile;
    zwcnn = FormatFileName(zwcnn);
    printf(zwcnn.c_str());printf("\n");
    if (!tflite->LoadModel(zwcnn)) {
        printf("Can't read model file /sdcard%s\n", cnnmodelfile.c_str());
        LogFile.WriteToFile("Cannot load model");

        delete tflite;
        return false;
    } 
    tflite->MakeAllocate();

    if (CNNType == AutoDetect)
    {
        int _anzoutputdimensions = tflite->GetAnzOutPut();
        switch (_anzoutputdimensions) 
        {
            case 2:
                CNNType = Analogue;
                printf("TFlite-Type set to Analogue\n");
                break;
            case 11:
                CNNType = Digital;
                printf("TFlite-Type set to Digital\n");
                break;
            case 22:
                CNNType = DigitalHyprid;
                printf("TFlite-Type set to DigitalHyprid\n");
                break;
            default:
                printf("ERROR ERROR ERROR - tflite passt nicht zur Firmware - ERROR ERROR ERROR\n");
        }
//        flowpostprocessing->UpdateNachkommaDecimalShift();
    }

    for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
    {
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
            printf("General %d - TfLite\n", i);

            switch (CNNType) {
                case Analogue:
                    {
                        float f1, f2;
                        f1 = 0; f2 = 0;

                        tflite->LoadInputImageBasis(GENERAL[_ana]->ROI[i]->image);        
                        tflite->Invoke();
                        if (debugdetailgeneral) LogFile.WriteToFile("Nach Invoke");

                        f1 = tflite->GetOutputValue(0);
                        f2 = tflite->GetOutputValue(1);
                        float result = fmod(atan2(f1, f2) / (M_PI * 2) + 2, 1);
                        GENERAL[_ana]->ROI[i]->result_float = result * 10;
                        printf("Result General(Analog)%i: %f\n", i, GENERAL[_ana]->ROI[i]->result_float); 
                        if (isLogImage)
                            LogImage(logPath, GENERAL[_ana]->ROI[i]->name, &GENERAL[_ana]->ROI[i]->result_float, NULL, time, GENERAL[_ana]->ROI[i]->image_org);
                    } break;
                case Digital:
                    {
                        GENERAL[_ana]->ROI[i]->result_klasse = 0;
                        GENERAL[_ana]->ROI[i]->result_klasse = tflite->GetClassFromImageBasis(GENERAL[_ana]->ROI[i]->image);
                        printf("Result General(Digit)%i: %d\n", i, GENERAL[_ana]->ROI[i]->result_klasse);

                        if (isLogImage)
                        {
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[_ana]->ROI[i]->name) != std::string::npos)
                                    LogImage(logPath, GENERAL[_ana]->ROI[i]->name, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, GENERAL[_ana]->ROI[i]->name, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                        }
                    } break;
                case DigitalHyprid:
                    {
                        int _num, _nachkomma;

                        tflite->LoadInputImageBasis(GENERAL[_ana]->ROI[i]->image);        
                        tflite->Invoke();
                        if (debugdetailgeneral) LogFile.WriteToFile("Nach Invoke");

                        _num = tflite->GetOutClassification(0, 10);
                        _nachkomma = tflite->GetOutClassification(11, 21);


                        string _zwres = "Nach Invoke - Nummer: " + to_string(_num) + " Nachkomma: " + to_string(_nachkomma);
                        if (debugdetailgeneral) LogFile.WriteToFile(_zwres);

                        if ((_num == 10) || (_nachkomma == 10))                      // NaN detektiert
                            GENERAL[_ana]->ROI[i]->result_float = -1;
                        else
                            GENERAL[_ana]->ROI[i]->result_float = fmod((double) _num + (((double)_nachkomma)-5)/10 + (double) 10, 10);

                        printf("Result General(DigitalHyprid)%i: %f\n", i, GENERAL[_ana]->ROI[i]->result_float); 
                        _zwres = "Result General(DigitalHyprid)" + to_string(i) + ": " + to_string(GENERAL[_ana]->ROI[i]->result_float);
                        if (debugdetailgeneral) LogFile.WriteToFile(_zwres);

                        if (isLogImage)
                            LogImage(logPath, GENERAL[_ana]->ROI[i]->name, &GENERAL[_ana]->ROI[i]->result_float, NULL, time, GENERAL[_ana]->ROI[i]->image_org);
                    } break;
                default:
                    break;
            }
        }
    }

    delete tflite;

    return true;
}

bool ClassFlowCNNGeneral::isExtendedResolution(int _number)
{
//    if (extendedResolution && !(CNNType == Digital))
    if (!(CNNType == Digital))
        return true;

    return false;
}



std::vector<HTMLInfo*> ClassFlowCNNGeneral::GetHTMLInfo()
{
    std::vector<HTMLInfo*> result;

    for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
                if (GENERAL[_ana]->name == "default")
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->ROI[i]->name + ".bmp"));
                else
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".bmp"));


            HTMLInfo *zw = new HTMLInfo;
            if (GENERAL[_ana]->name == "default")
            {
                zw->filename = GENERAL[_ana]->ROI[i]->name + ".bmp";
                zw->filename_org = GENERAL[_ana]->ROI[i]->name + ".jpg";
            }
            else
            {
                zw->filename = GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".bmp";
                zw->filename_org = GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".jpg";
            }

            if (CNNType == Digital)
                zw->val = GENERAL[_ana]->ROI[i]->result_klasse;
            else
                zw->val = GENERAL[_ana]->ROI[i]->result_float;
            zw->image = GENERAL[_ana]->ROI[i]->image;
            zw->image_org = GENERAL[_ana]->ROI[i]->image_org;

//            printf("Push %s\n", zw->filename.c_str());

            result.push_back(zw);
        }

//    printf("größe: %d\n", result.size());

    return result;
}

int ClassFlowCNNGeneral::getAnzahlGENERAL()
{
    return GENERAL.size();
}

string ClassFlowCNNGeneral::getNameGENERAL(int _analog)
{
    if (_analog < GENERAL.size())
        return GENERAL[_analog]->name;

    return "GENERAL DOES NOT EXIST";
}

general* ClassFlowCNNGeneral::GetGENERAL(int _analog)
{
    if (_analog < GENERAL.size())
        return GENERAL[_analog];

    return NULL;
}



void ClassFlowCNNGeneral::UpdateNameNumbers(std::vector<std::string> *_name_numbers)
{
    for (int _dig = 0; _dig < GENERAL.size(); _dig++)
    {
        std::string _name = GENERAL[_dig]->name;
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
