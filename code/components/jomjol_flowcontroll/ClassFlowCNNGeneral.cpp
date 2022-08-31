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
    CNNGoodThreshold = 0.0;
    ListFlowControll = NULL;
    previousElement = NULL;   
    SaveAllFiles = false; 
    disabled = false;
    isLogImageSelect = false;
    CNNType = AutoDetect;
    CNNType = _cnntype;
    flowpostalignment = _flowalign;
}

string ClassFlowCNNGeneral::getReadout(int _analog = 0, bool _extendedResolution, int prev, float _vorgaengerAnalog)
{
    string result = "";    

    if (GENERAL[_analog]->ROI.size() == 0)
        return result;
    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::getReadout _analog=" + std::to_string(_analog) + ", _extendedResolution=" + std::to_string(_extendedResolution) + ", prev=" + std::to_string(prev));

    if (CNNType == Analogue || CNNType == Analogue100)
    {
        float zahl = GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float;
        int ergebnis_nachkomma = ((int) floor(zahl * 10) + 10) % 10;
        
        prev = ZeigerEvalAnalogNeu(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, prev);
//        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::getReadout(analog) zahl=" + std::to_string(zahl) + ", ergebnis_nachkomma=" + std::to_string(ergebnis_nachkomma) + ", prev=" + std::to_string(prev));
        result = std::to_string(prev);

        if (_extendedResolution && (CNNType != Digital))
            result = result + std::to_string(ergebnis_nachkomma);

        for (int i = GENERAL[_analog]->ROI.size() - 2; i >= 0; --i)
        {
            prev = ZeigerEvalAnalogNeu(GENERAL[_analog]->ROI[i]->result_float, prev);
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

    if ((CNNType == DoubleHyprid10) || (CNNType == Digital100))
    {

        float zahl = GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float;
        if (zahl >= 0)       // NaN?
        {
            if (_extendedResolution)            // ist nur gesetzt, falls es die erste Ziffer ist (kein Analog vorher!)
            {
                int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;
                int ergebnis_vorkomma = ((int) floor(zahl)) % 10;

                result = std::to_string(ergebnis_vorkomma) + std::to_string(ergebnis_nachkomma);
                prev = ergebnis_vorkomma;
                if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::getReadout(dig100-ext) ergebnis_vorkomma=" + std::to_string(ergebnis_vorkomma) + ", ergebnis_nachkomma=" + std::to_string(ergebnis_nachkomma) + ", prev=" + std::to_string(prev));
            }
            else
            {
//                prev = ZeigerEval(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, prev);
                if (_vorgaengerAnalog >= 0)
                    prev = ZeigerEvalHybridNeu(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, _vorgaengerAnalog, prev, true);
                else
                    prev = ZeigerEvalHybridNeu(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, prev, prev);
                result = std::to_string(prev);
                if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::getReadout(dig100)  prev=" + std::to_string(prev));
        
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
                prev = ZeigerEvalHybridNeu(GENERAL[_analog]->ROI[i]->result_float, GENERAL[_analog]->ROI[i+1]->result_float, prev);
                if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::getReadout#ZeigerEvalHybridNeu()= " + std::to_string(prev));
                result = std::to_string(prev) + result;
                if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::getReadout#result= " + result);
                
            }
            else
            {
                prev = -1;
                result = "N" + result;
                if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::getReadout(result_float<0 /'N')  result_float=" + std::to_string(GENERAL[_analog]->ROI[i]->result_float));
        
            }
        }
        return result;
    }


    return result;
}

/*
int ClassFlowCNNGeneral::ZeigerEvalHybrid(float zahl, float zahl_vorgaenger, int eval_vorgaenger)
{
    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalHybrid( " + std::to_string(zahl) + ", " + std::to_string(zahl_vorgaenger) + ", " + std::to_string(eval_vorgaenger) + ")");
                
    int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;
    int ergebnis_vorkomma = ((int) floor(zahl) + 10) % 10;


    if (eval_vorgaenger < 0)                // keine Vorzahl vorhanden !!! --> Runde die Zahl
    {
        if ((ergebnis_nachkomma <= 2) || (ergebnis_nachkomma >= 8))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
            return ((int) round(zahl) + 10) % 10;
        else
            return ((int) trunc(zahl) + 10) % 10;
    }

    // 9.0, da bei getReadout() prev als int übergeben wird (9 statt 9.5)
    // tritt bei der ersten ziffer von digit auf, wenn analog davor (2. Aufruf von getReadout)
    if ((zahl_vorgaenger >= 0.5 ) && (zahl_vorgaenger < 9.5))
    {
        // kein Ziffernwechsel, da Vorkomma weit genug weg ist (0+/-0.5) --> zahl wird gerundet
        if ((ergebnis_nachkomma <= 2) || (ergebnis_nachkomma >= 8))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
            return ((int) round(zahl) + 10) % 10;
        else
            return ((int) trunc(zahl) + 10) % 10;
    }  
    else
    {
        if (eval_vorgaenger <= 1)  // Nulldurchgang hat stattgefunden (!Bewertung über Prev_value und nicht Zahl!) --> hier aufrunden (2.8 --> 3, aber auch 3.1 --> 3)
        {
            if (ergebnis_nachkomma > 5)
                return (ergebnis_vorkomma + 1) % 10;
            else
                return ergebnis_vorkomma;
        }
        else // bleibt nur >= 9.5 --> noch kein Nulldurchgang --> 2.8 --> 2, und 3.1 --> 2
        {
            // hier auf 4 reduziert, da erst ab Vorgänder 9 anfängt umzustellen. Bei 9.5 Vorgänger kann die aktuelle
            // Zahl noch x.4 - x.5 sein.
            if (ergebnis_nachkomma >= 4)
                return ergebnis_vorkomma;
            else
                return (ergebnis_vorkomma - 1 + 10) % 10;
        }
    }
    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalHybrid(return -1)  zahl=" + std::to_string(zahl) 
                        + ", zahl_vorgaenger=" + std::to_string(zahl_vorgaenger) + ", eval_vorgaenger=" + std::to_string(eval_vorgaenger));
    return -1;

}
*/

int ClassFlowCNNGeneral::ZeigerEvalHybridNeu(float zahl, float zahl_vorgaenger, int eval_vorgaenger, bool AnalogerVorgaenger)
{
    int result;
    int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;
    int ergebnis_vorkomma = ((int) floor(zahl) + 10) % 10;

    if (eval_vorgaenger < 0)
    {
        if ((ergebnis_nachkomma <= DigitalUnschaerfe * 10) || (ergebnis_nachkomma >= DigitalUnschaerfe * 10))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
            result = (int) (round(zahl) + 10) % 10;
        else
            result = (int) ((int) trunc(zahl) + 10) % 10;

        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalHybridNeu - kein Vorgänger - Ergebnis = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " zahl_vorgaenger = " + std::to_string(zahl_vorgaenger)+ " eval_vorgaenger = " + std::to_string(eval_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe));
        return result;
    }

    if (AnalogerVorgaenger)
    {
//        result = ZeigerEvalAnalogToDigitNeu(zahl, eval_vorgaenger);
        result = ZeigerEvalAnalogToDigitNeu(zahl, zahl_vorgaenger);
        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalHybridNeu - Analoger Vorgänger, Bewertung über ZeigerEvalAnalogNeu = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " zahl_vorgaenger = " + std::to_string(zahl_vorgaenger)+ " eval_vorgaenger = " + std::to_string(eval_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe));
        return result;
    }

    if ((zahl_vorgaenger >= DigitalUebergangsbereichVorgaenger ) && (zahl_vorgaenger <= (10.0 - DigitalUebergangsbereichVorgaenger)))
    {
        // kein Ziffernwechsel, da Vorgänger weit genug weg ist (0+/-DigitalUebergangsbereichVorgaenger) --> zahl wird gerundet
        if ((ergebnis_nachkomma <= DigitalBand) || (ergebnis_nachkomma >= (10-DigitalBand)))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
            result = ((int) round(zahl) + 10) % 10;
        else
            result = ((int) trunc(zahl) + 10) % 10;

        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalHybridNeu - KEIN Analoger Vorgänger, kein Ziffernwechsel, da Vorkomma weit genug weg = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " zahl_vorgaenger = " + std::to_string(zahl_vorgaenger)+ " eval_vorgaenger = " + std::to_string(eval_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe));
        return result;
    }  

    if (eval_vorgaenger <= 1)  // Nulldurchgang hat stattgefunden (!Bewertung über Prev_value und nicht Zahl!) --> hier aufrunden (2.8 --> 3, aber auch 3.1 --> 3)
    {
        if (ergebnis_nachkomma > 5)
            result =  (ergebnis_vorkomma + 1) % 10;
        else
            result =  ergebnis_vorkomma;
        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalHybridNeu - KEIN Analoger Vorgänger, Nulldurchgang hat stattgefunden = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " zahl_vorgaenger = " + std::to_string(zahl_vorgaenger)+ " eval_vorgaenger = " + std::to_string(eval_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe));
        return result;
    }

    // bleibt nur >= 9.5 --> noch kein Nulldurchgang --> 2.8 --> 2, und 3.1 --> 2
    // alles <=x.6 kann als aktuelle Zahl gelten im Übergang. Bei 9.5 Vorgänger kann die aktuelle
    // Zahl noch x.6 - x.7 sein. 
    if (ergebnis_nachkomma <= 6)
        result =  ergebnis_vorkomma;
    else
        result =  (ergebnis_vorkomma - 1 + 10) % 10;

    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalHybridNeu - KEIN Analoger Vorgänger, >= 9.5 --> noch kein Nulldurchgang = " + std::to_string(result) +
                                                " zahl: " + std::to_string(zahl) + " zahl_vorgaenger = " + std::to_string(zahl_vorgaenger)+ " eval_vorgaenger = " + std::to_string(eval_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe) + " ergebnis_nachkomma = " + std::to_string(ergebnis_nachkomma));
    return result;
}


int ClassFlowCNNGeneral::ZeigerEvalAnalogToDigitNeu(float zahl, float ziffer_vorgaenger)
{
    int result;
    int ergebnis_nachkomma = ((int) floor(zahl * 10)) % 10;
    int ergebnis_vorkomma = ((int) floor(zahl) + 10) % 10;

    if (ziffer_vorgaenger < 0)
    {
        result = (int) floor(zahl);
        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogToDigitNeu - kein Vorgänger - Ergebnis = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " AnalogFehler = " +  std::to_string(AnalogFehler));
        return result;
    }

    if ((ziffer_vorgaenger >= DigitalUebergangsbereichVorgaengerAnalogToDigit ) && (ziffer_vorgaenger <= (10.0 - DigitalUebergangsbereichVorgaengerAnalogToDigit)))
    {
        // kein Ziffernwechsel, da Vorgänger weit genug weg ist (0+/-DigitalUebergangsbereichVorgaenger) --> zahl wird gerundet
        if ((ergebnis_nachkomma <= 2) || (ergebnis_nachkomma >= 8))     // Band um die Ziffer --> Runden, da Ziffer im Rahmen Ungenauigkeit erreicht
            result = ((int) round(zahl) + 10) % 10;
        else
            result = ((int) trunc(zahl) + 10) % 10;

        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogToDigitNeu - kein Ziffernwechsel, da Vorkomma weit genug weg = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe));
        return result;
    }  

    if (ziffer_vorgaenger <= 1)  // Nulldurchgang hat stattgefunden (!Bewertung über Prev_value und nicht Zahl!) --> hier aufrunden (2.8 --> 3, aber auch 3.1 --> 3)
    {
        if (ergebnis_nachkomma > 5)
            result =  (ergebnis_vorkomma + 1) % 10;
        else
            result =  ergebnis_vorkomma;
        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogToDigitNeu - Nulldurchgang hat stattgefunden = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe));
        return result;
    }

    // bleibt nur >= 9.5 --> noch kein Nulldurchgang --> 2.8 --> 2, und 3.1 --> 2
    // hier auf 4 reduziert, da erst ab Vorgänder 9 anfängt umzustellen. Bei 9.5 Vorgänger kann die aktuelle
    // Zahl noch x.4 - x.5 sein.
    if (ergebnis_nachkomma >= 4)
        result =  ergebnis_vorkomma;
    else
        result =  (ergebnis_vorkomma - 1 + 10) % 10;

    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogToDigitNeu - 9.0 --> noch kein Nulldurchgang = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " DigitalUnschaerfe = " +  std::to_string(DigitalUnschaerfe));
    return result;
}

int ClassFlowCNNGeneral::ZeigerEvalAnalogNeu(float zahl, int ziffer_vorgaenger)
{
    float zahl_min, zahl_max;
    int result;

    if (ziffer_vorgaenger == -1)
    {
        result = (int) floor(zahl);
        if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogNeu - kein Vorgänger - Ergebnis = " + std::to_string(result) +
                                                    " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " AnalogFehler = " +  std::to_string(AnalogFehler));
        return result;
    }

    zahl_min = zahl - AnalogFehler / 10.0;
    zahl_max = zahl + AnalogFehler / 10.0;

    if ((int) floor(zahl_max) - (int) floor(zahl_min) != 0)
    {
        if (ziffer_vorgaenger <= AnalogFehler)
        {
            result = ((int) floor(zahl_max) + 10) % 10;
            if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogNeu - Zahl uneindeutig, Korrektur nach oben - Ergebnis = " + std::to_string(result) +
                                                        " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " AnalogFehler = " +  std::to_string(AnalogFehler));
            return result;
        }
        if (ziffer_vorgaenger >= 10 - AnalogFehler)
        {
            result = ((int) floor(zahl_min) + 10) % 10;
            if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogNeu - Zahl uneindeutig, Korrektur nach unten - Ergebnis = " + std::to_string(result) +
                                                        " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " AnalogFehler = " +  std::to_string(AnalogFehler));
            return result;
        }
    }
    

    result = ((int) floor(zahl) + 10) % 10;
    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEvalAnalogNeu - Zahl eindeutig, keine Korrektur notwendig - Ergebnis = " + std::to_string(result) +
                                                " zahl: " + std::to_string(zahl) + " ziffer_vorgaenger = " + std::to_string(ziffer_vorgaenger) + " AnalogFehler = " +  std::to_string(AnalogFehler));

    return result;

}

/*
int ClassFlowCNNGeneral::ZeigerEval(float zahl, int ziffer_vorgaenger)
{   
    int ergebnis_nachkomma = ((int) floor(zahl * 10) + 10) % 10;
    int ergebnis_vorkomma = ((int) floor(zahl) + 10) % 10;
    int ergebnis;
    float ergebnis_rating;
    if (debugdetailgeneral) LogFile.WriteToFile("ClassFlowCNNGeneral::ZeigerEval erg_v=" + std::to_string(ergebnis_vorkomma) + ", erg_n=" + std::to_string(ergebnis_nachkomma) + ", ziff_v=" + std::to_string(ziffer_vorgaenger));

    if (ziffer_vorgaenger == -1)
        return ergebnis_vorkomma % 10;

    // Ist die aktuelle Stelle schon umgesprungen und die Vorstelle noch nicht?
    // Akt.: 2.1, Vorstelle = 0.9 => 1.9
    // Problem sind mehrere Rundungen 
    // Bsp. zahl=4.5, Vorgänger= 9.6 (ziffer_vorgaenger=0)
    // Tritt nur auf bei Übergang von analog auf digit
    ergebnis_rating = ergebnis_nachkomma - ziffer_vorgaenger;
    if (ergebnis_nachkomma >= 5)
        ergebnis_rating-=5.1;
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
*/

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
        if ((toUpper(zerlegt[0]) == "LOGIMAGELOCATION") && (zerlegt.size() > 1))
        {
            this->LogImageLocation = "/sdcard" + zerlegt[1];
            this->isLogImage = true;
        }
        if ((toUpper(zerlegt[0]) == "LOGIMAGESELECT") && (zerlegt.size() > 1))
        {
            LogImageSelect = zerlegt[1];
            isLogImageSelect = true;            
        }

        if ((toUpper(zerlegt[0]) == "LOGFILERETENTIONINDAYS") && (zerlegt.size() > 1))
        {
            this->logfileRetentionInDays = std::stoi(zerlegt[1]);
        }
//        if ((toUpper(zerlegt[0]) == "MODELTYPE") && (zerlegt.size() > 1))
//        {
//            if (toUpper(zerlegt[1]) == "DIGITHYPRID")
//                CNNType = DigitalHyprid;
//        }

        if ((toUpper(zerlegt[0]) == "MODEL") && (zerlegt.size() > 1))
        {
            this->cnnmodelfile = zerlegt[1];
        }
        
        if ((toUpper(zerlegt[0]) == "CNNGOODTHRESHOLD") && (zerlegt.size() > 1))
        {
            CNNGoodThreshold = std::stof(zerlegt[1]);
        }
        if (zerlegt.size() >= 5)
        {
            general* _analog = GetGENERAL(zerlegt[0], true);
            roi* neuroi = _analog->ROI[_analog->ROI.size()-1];
            neuroi->posx = std::stoi(zerlegt[1]);
            neuroi->posy = std::stoi(zerlegt[2]);
            neuroi->deltax = std::stoi(zerlegt[3]);
            neuroi->deltay = std::stoi(zerlegt[4]);
            neuroi->CCW = false;
            if (zerlegt.size() >= 6)
            {
                neuroi->CCW = toUpper(zerlegt[5]) == "TRUE";
            }
            neuroi->result_float = -1;
            neuroi->image = NULL;
            neuroi->image_org = NULL;
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }
    }

    if (!getNetworkParameter())
        return false;


    for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
            GENERAL[_ana]->ROI[i]->image = new CImageBasis(modelxsize, modelysize, modelchannel);
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

    printf("GetGENERAL - GENERAL %s - roi %s - CCW: %d\n", _analog.c_str(), _roi.c_str(), neuroi->CCW);

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
    if (CNNType == Analogue || CNNType == Analogue100)
    {
        int r = 0;
        int g = 255;
        int b = 0;

        for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
            for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
            {
                _zw->drawRect(GENERAL[_ana]->ROI[i]->posx, GENERAL[_ana]->ROI[i]->posy, GENERAL[_ana]->ROI[i]->deltax, GENERAL[_ana]->ROI[i]->deltay, r, g, b, 1);
                _zw->drawEllipse( (int) (GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax/2), (int)  (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay/2), (int) (GENERAL[_ana]->ROI[i]->deltax/2), (int) (GENERAL[_ana]->ROI[i]->deltay/2), r, g, b, 2);
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

bool ClassFlowCNNGeneral::getNetworkParameter()
{
    if (disabled)
        return true;

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
        tflite->GetInputDimension(false);
        modelxsize = tflite->ReadInputDimenstion(0);
        modelysize = tflite->ReadInputDimenstion(1);
        modelchannel = tflite->ReadInputDimenstion(2);

        int _anzoutputdimensions = tflite->GetAnzOutPut();
        switch (_anzoutputdimensions) 
        {
            case 2:
                CNNType = Analogue;
                printf("TFlite-Type set to Analogue\n");
                break;
            case 10:
                CNNType = DoubleHyprid10;
                printf("TFlite-Type set to DoubleHyprid10\n");
                break;
            case 11:
                CNNType = Digital;
                printf("TFlite-Type set to Digital\n");
                break;
            case 20:
                CNNType = DigitalHyprid10;
                printf("TFlite-Type set to DigitalHyprid10\n");
                break;
//            case 22:
//                CNNType = DigitalHyprid;
//                printf("TFlite-Type set to DigitalHyprid\n");
//                break;
             case 100:
                if (modelxsize==32 && modelysize == 32) {
                    CNNType = Analogue100;
                    printf("TFlite-Type set to Analogue100\n");
                } else {
                    CNNType = Digital100;
                    printf("TFlite-Type set to Digital\n");
                }
                break;
            default:
                LogFile.WriteToFile("ERROR ERROR ERROR - tflite passt nicht zur Firmware - ERROR ERROR ERROR (outout_dimension=" + std::to_string(_anzoutputdimensions) + ")");
                printf("ERROR ERROR ERROR - tflite passt nicht zur Firmware - ERROR ERROR ERROR\n");
        }
    }

    delete tflite;
    return true;
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
                              
                        if(GENERAL[_ana]->ROI[i]->CCW)
                            GENERAL[_ana]->ROI[i]->result_float = 10 - (result * 10);
                        else
                            GENERAL[_ana]->ROI[i]->result_float = result * 10;
                              
                        printf("Result General(Analog)%i - CCW: %d -  %f\n", i, GENERAL[_ana]->ROI[i]->CCW, GENERAL[_ana]->ROI[i]->result_float); 
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
                            string _imagename = GENERAL[_ana]->name +  "_" + GENERAL[_ana]->ROI[i]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[_ana]->ROI[i]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                        }
                    } break;
/*
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
                        {
                            string _imagename = GENERAL[_ana]->name +  "_" + GENERAL[_ana]->ROI[i]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[_ana]->ROI[i]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                        }
                    } break;
*/
                case DigitalHyprid10:
                    {
                        int _num, _nachkomma;

                        tflite->LoadInputImageBasis(GENERAL[_ana]->ROI[i]->image);        
                        tflite->Invoke();
                        if (debugdetailgeneral) LogFile.WriteToFile("Nach Invoke");

                        _num = tflite->GetOutClassification(0, 9);
                        _nachkomma = tflite->GetOutClassification(10, 19);


                        string _zwres = "Nach Invoke - Nummer: " + to_string(_num) + " Nachkomma: " + to_string(_nachkomma);
                        if (debugdetailgeneral) LogFile.WriteToFile(_zwres);

                        GENERAL[_ana]->ROI[i]->result_float = fmod((double) _num + (((double)_nachkomma)-5)/10 + (double) 10, 10);

                        printf("Result General(DigitalHyprid)%i: %f\n", i, GENERAL[_ana]->ROI[i]->result_float); 
                        _zwres = "Result General(DigitalHyprid)" + to_string(i) + ": " + to_string(GENERAL[_ana]->ROI[i]->result_float);
                        if (debugdetailgeneral) LogFile.WriteToFile(_zwres);

                        if (isLogImage)
                        {
                            string _imagename = GENERAL[_ana]->name +  "_" + GENERAL[_ana]->ROI[i]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[_ana]->ROI[i]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, NULL, &GENERAL[_ana]->ROI[i]->result_klasse, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                        }
                    } break;

                case DoubleHyprid10:
                    {
                        int _num, _numplus, _numminus;
                        float _val, _valplus, _valminus;
                        float _fit;
                        float _result_save_file;

                        tflite->LoadInputImageBasis(GENERAL[_ana]->ROI[i]->image);        
                        tflite->Invoke();
                        if (debugdetailgeneral) LogFile.WriteToFile("Nach Invoke");

                        _num = tflite->GetOutClassification(0, 9);
                        _numplus = (_num + 1) % 10;
                        _numminus = (_num - 1 + 10) % 10;

                        _val = tflite->GetOutputValue(_num);
                        _valplus = tflite->GetOutputValue(_numplus);
                        _valminus = tflite->GetOutputValue(_numminus);

                        float result = _num;

                        if (_valplus > _valminus)
                        {
                            result = result + _valplus / (_valplus + _val);
                            _fit = _val + _valplus;
                        }
                        else
                        {
                            result = result - _valminus / (_val + _valminus);
                            _fit = _val + _valminus;

                        }
                        if (result > 10)
                            result = result - 10;
                        if (result < 0)
                            result = result + 10;

                        string zw = "_num (p, m): " + to_string(_num) + " " + to_string(_numplus) + " " + to_string(_numminus);
                        zw = zw + " _val (p, m): " + to_string(_val) + " " + to_string(_valplus) + " " + to_string(_valminus);
                        zw = zw + " result: " + to_string(result) + " _fit: " + to_string(_fit);
                        printf("details cnn: %s\n", zw.c_str());
                        LogFile.WriteToFile(zw);


                        _result_save_file = result;

                        if (_fit < CNNGoodThreshold)
                        {
                            GENERAL[_ana]->ROI[i]->isReject = true;
                            result = -1;
                            _result_save_file+= 100;     // Für den Fall, dass fit nicht ausreichend, soll trotzdem das Ergebnis mit "-10x.y" abgespeichert werden.
                            string zw = "Value Rejected due to Threshold (Fit: " + to_string(_fit) + "Threshold: " + to_string(CNNGoodThreshold);
                            printf("Value Rejected due to Threshold (Fit: %f, Threshold: %f\n", _fit, CNNGoodThreshold);
                            LogFile.WriteToFile(zw);
                        }
                        else
                        {
                            GENERAL[_ana]->ROI[i]->isReject = false;
                        }


                        GENERAL[_ana]->ROI[i]->result_float = result;
                        printf("Result General(Analog)%i: %f\n", i, GENERAL[_ana]->ROI[i]->result_float); 

                        if (isLogImage)
                        {
                            string _imagename = GENERAL[_ana]->name +  "_" + GENERAL[_ana]->ROI[i]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[_ana]->ROI[i]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                        }
                    }
                    break;
                case Digital100:
                case Analogue100:
                    {
                        int _num;
                        float _result_save_file;
                        
                        tflite->LoadInputImageBasis(GENERAL[_ana]->ROI[i]->image);        
                        tflite->Invoke();
    
                        _num = tflite->GetOutClassification();
                        
                        if(GENERAL[_ana]->ROI[i]->CCW)
                            GENERAL[_ana]->ROI[i]->result_float = 10 - ((float)_num / 10.0);                              
                        else
                            GENERAL[_ana]->ROI[i]->result_float = (float)_num / 10.0;

                        _result_save_file = GENERAL[_ana]->ROI[i]->result_float;

                        
                        GENERAL[_ana]->ROI[i]->isReject = false;
                        
                        printf("Result General(Analog)%i - CCW: %d -  %f\n", i, GENERAL[_ana]->ROI[i]->CCW, GENERAL[_ana]->ROI[i]->result_float); 

                        if (isLogImage)
                        {
                            string _imagename = GENERAL[_ana]->name +  "_" + GENERAL[_ana]->ROI[i]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[_ana]->ROI[i]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[_ana]->ROI[i]->image_org);
                            }
                        }

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
            printf("Image: %d\n", (int) GENERAL[_ana]->ROI[i]->image);
            if (GENERAL[_ana]->ROI[i]->image)
            {
                if (GENERAL[_ana]->name == "default")
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->ROI[i]->name + ".bmp"));
                else
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".bmp"));
            }

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

            result.push_back(zw);
        }

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
