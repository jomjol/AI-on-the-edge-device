#include "ClassFlowPostProcessing.h"

#include "Helper.h"
#include "ClassFlowAnalog.h"
#include "ClassFlowDigit.h"
#include "ClassFlowMakeImage.h"

#include <iomanip>
#include <sstream>

#include <time.h>

string ClassFlowPostProcessing::GetPreValue()
{
    std::string result;
    result = to_string(PreValue);

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowAnalog") == 0)
        {
            int AnzahlNachkomma = ((ClassFlowAnalog*)(*ListFlowControll)[i])->AnzahlROIs();
            std::stringstream stream;
            stream << std::fixed << std::setprecision(AnzahlNachkomma) << PreValue;
            result = stream.str();
        }
    }

    return result;
}

bool ClassFlowPostProcessing::LoadPreValue(void)
{
    FILE* pFile;
    char zw[1024];
    string zwtime, zwvalue;

    pFile = fopen(FilePreValue.c_str(), "r");
    if (pFile == NULL)
        return false;

    fgets(zw, 1024, pFile);
    printf("%s", zw);
    zwtime = trim(std::string(zw));

    fgets(zw, 1024, pFile);
    printf("%s", zw);
    zwvalue = trim(std::string(zw));
    PreValue = stof(zwvalue.c_str());

    time_t tStart;
    int yy, month, dd, hh, mm, ss;
    struct tm whenStart;

    sscanf(zwtime.c_str(), "%d-%d-%d_%d-%d-%d", &yy, &month, &dd, &hh, &mm, &ss);
    whenStart.tm_year = yy - 1900;
    whenStart.tm_mon = month - 1;
    whenStart.tm_mday = dd;
    whenStart.tm_hour = hh;
    whenStart.tm_min = mm;
    whenStart.tm_sec = ss;
    whenStart.tm_isdst = -1;

    tStart = mktime(&whenStart);

    time_t now;
    time(&now);
    localtime(&now);
    double difference = difftime(now, tStart);
    difference /= 60;
    if (difference > PreValueAgeStartup)
        return false;

    Value = PreValue;
    ReturnValue = to_string(Value);
    ReturnValueNoError = ReturnValue; 

    // falls es Analog gibt, dann die Anzahl der Nachkommastellen feststellen und entsprechend runden:   
    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowAnalog") == 0)
        {
            int AnzahlNachkomma = ((ClassFlowAnalog*)(*ListFlowControll)[i])->AnzahlROIs();
            std::stringstream stream;
            stream << std::fixed << std::setprecision(AnzahlNachkomma) << Value;
            ReturnValue = stream.str();
            ReturnValueNoError = ReturnValue;
        }
    }

    
    return true;
}

void ClassFlowPostProcessing::SavePreValue(float value, string zwtime)
{
    FILE* pFile;
    PreValue = value;

    pFile = fopen(FilePreValue.c_str(), "w");

    if (strlen(zwtime.c_str()) == 0)
    {
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S", timeinfo);

        zwtime = std::string(buffer);
    }

fputs(zwtime.c_str(), pFile);
fputs("\n", pFile);

fputs(to_string(value).c_str(), pFile);
fputs("\n", pFile);

fclose(pFile);
}



ClassFlowPostProcessing::ClassFlowPostProcessing()
{
    PreValueUse = false;
    PreValueAgeStartup = 30;
    AllowNegativeRates = false;
    MaxRateValue = 0.1;
    ErrorMessage = false;
    ListFlowControll = NULL;
    PreValueOkay = false;
    useMaxRateValue = false;
    checkDigitIncreaseConsistency = false;
    DecimalShift = 0;
    FilePreValue = FormatFileName("/sdcard/config/prevalue.ini");
}

ClassFlowPostProcessing::ClassFlowPostProcessing(std::vector<ClassFlow*>* lfc)
{
    PreValueUse = false;
    PreValueAgeStartup = 30;
    AllowNegativeRates = false;
    MaxRateValue = 0.1;
    ErrorMessage = false;
    ListFlowControll = NULL;
    PreValueOkay = false;
    useMaxRateValue = false;
    checkDigitIncreaseConsistency = false;
    DecimalShift = 0;    
    FilePreValue = FormatFileName("/sdcard/config/prevalue.ini");
    ListFlowControll = lfc;
}


bool ClassFlowPostProcessing::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if (aktparamgraph.compare("[PostProcessing]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((toUpper(zerlegt[0]) == "DECIMALSHIFT") && (zerlegt.size() > 1))
        {
            DecimalShift = stoi(zerlegt[1]);
        }

        if ((toUpper(zerlegt[0]) == "PREVALUEUSE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                PreValueUse = true;
                PreValueOkay = LoadPreValue();
            }
        }
        if ((toUpper(zerlegt[0]) == "CHECKDIGITINCREASECONSISTENCY") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                checkDigitIncreaseConsistency = true;
        }        
        if ((toUpper(zerlegt[0]) == "ALLOWNEGATIVERATES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                AllowNegativeRates = true;
        }
        if ((toUpper(zerlegt[0]) == "ERRORMESSAGE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                ErrorMessage = true;
        }
        if ((toUpper(zerlegt[0]) == "PREVALUEAGESTARTUP") && (zerlegt.size() > 1))
        {
            PreValueAgeStartup = std::stoi(zerlegt[1]);
        }
        if ((toUpper(zerlegt[0]) == "MAXRATEVALUE") && (zerlegt.size() > 1))
        {
            useMaxRateValue = true;
            MaxRateValue = std::stof(zerlegt[1]);
        }
    }
    return true;
}

string ClassFlowPostProcessing::ShiftDecimal(string in, int _decShift){

    if (_decShift == 0){
        return in;
    }

    int _pos_dec_org, _pos_dec_neu;

    _pos_dec_org = findDelimiterPos(in, ".");
    if (_pos_dec_org == std::string::npos) {
        _pos_dec_org = in.length();
    }
    else
    {
        in = in.erase(_pos_dec_org, 1);
    }
    
    _pos_dec_neu = _pos_dec_org + _decShift;

    if (_pos_dec_neu <= 0) {        // Komma ist vor der ersten Ziffer
        for (int i = 0; i > _pos_dec_neu; --i){
            in = in.insert(0, "0");
        }
        in = "0." + in;
        return in;
    }

    if (_pos_dec_neu > in.length()){    // Komma soll hinter String (123 --> 1230)
        for (int i = in.length(); i < _pos_dec_neu; ++i){
            in = in.insert(in.length(), "0");
        }  
        return in;      
    }

    string zw;
    zw = in.substr(0, _pos_dec_neu);
    zw = zw + ".";
    zw = zw + in.substr(_pos_dec_neu, in.length() - _pos_dec_neu);

    return zw;
}

bool ClassFlowPostProcessing::doFlow(string zwtime)
{
    string result = "";
    string digit = "";
    string analog = "";
    string zwvalue;
    bool isdigit = false;
    bool isanalog = false;
    int AnzahlNachkomma = 0;
    string zw;
    string error = "";
    time_t imagetime = 0;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowMakeImage") == 0)
        {
            imagetime = ((ClassFlowMakeImage*)(*ListFlowControll)[i])->getTimeImageTaken();
        }
        if (((*ListFlowControll)[i])->name().compare("ClassFlowDigit") == 0)
        {
            isdigit = true;
            digit = (*ListFlowControll)[i]->getReadout();
        }
        if (((*ListFlowControll)[i])->name().compare("ClassFlowAnalog") == 0)
        {
            isanalog = true;
            analog = (*ListFlowControll)[i]->getReadout();
            AnzahlNachkomma = ((ClassFlowAnalog*)(*ListFlowControll)[i])->AnzahlROIs();
        }
    }

    if (imagetime == 0)
        time(&imagetime);

    struct tm* timeinfo;
    timeinfo = localtime(&imagetime);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d_%H-%M-%S", timeinfo);
    zwtime = std::string(strftime_buf);


    //    // TESTING ONLY////////////////////
    //    isdigit = true; digit = "12N";
    //    isanalog = true; analog = "456";

    if (isdigit)
        ReturnRawValue = digit;
    if (isdigit && isanalog)
        ReturnRawValue = ReturnRawValue + ".";
    if (isanalog)
        ReturnRawValue = ReturnRawValue + analog; 

    ReturnRawValue = ShiftDecimal(ReturnRawValue, DecimalShift);   

    if (!PreValueUse || !PreValueOkay)
    {
        ReturnValue = ReturnRawValue;
        ReturnValueNoError = ReturnRawValue;

        if ((findDelimiterPos(ReturnValue, "N") == std::string::npos) && (ReturnValue.length() > 0))
        {
            while ((ReturnValue.length() > 1) && (ReturnValue[0] == '0'))
            {
                ReturnValue.erase(0, 1);
            }
            Value = std::stof(ReturnValue);
            ReturnValueNoError = ReturnValue;
            
            SavePreValue(Value, zwtime);
        }
        return true;
    }

    if (isdigit)
    {
        int lastanalog = -1;
        if (isanalog)
            lastanalog = analog[0] - 48;
        digit = ErsetzteN(digit, lastanalog);
        zw = digit;
    }

    if (isdigit && isanalog)
        zw = zw + ".";
    if (isanalog)
        zw = zw + analog;

    Value = std::stof(zw);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(AnzahlNachkomma) << Value;
    zwvalue = stream.str();

    if ((!AllowNegativeRates) && (Value < PreValue))
    {
        error = "Negative Rate - Returned old value - read value: " + zwvalue;
        Value = PreValue;
        stream.str("");
        stream << std::fixed << std::setprecision(AnzahlNachkomma) << Value;
        zwvalue = stream.str();
    }
    else
    {
        if (useMaxRateValue && (abs(Value - PreValue) > MaxRateValue))
        {
            error = "Rate too high - Returned old value - read value: " + zwvalue;
            Value = PreValue;
            stream.str("");
            stream << std::fixed << std::setprecision(AnzahlNachkomma) << Value;
            zwvalue = stream.str();
        }
    }

    ReturnValueNoError = zwvalue;
    ReturnValue = zwvalue;
    if (ErrorMessage && (error.length() > 0))
        ReturnValue = ReturnValue + "\t" + error;

    if (error.length() == 0)
        SavePreValue(Value, zwtime);

    return true;
}

string ClassFlowPostProcessing::getReadout()
{
    return ReturnValue;
}

string ClassFlowPostProcessing::getReadoutParam(bool _rawValue, bool _noerror)
{
    if (_rawValue)
        return ReturnRawValue;
    if (_noerror)
        return ReturnValueNoError;
    return ReturnValue;
}


string ClassFlowPostProcessing::ErsetzteN(string input, int lastvalueanalog = -1)
{
    int posN, posPunkt;
    int pot, ziffer;
    float zw;

    posN = findDelimiterPos(input, "N");
    posPunkt = input.length();

    while (posN != std::string::npos)
    {
        pot = posPunkt - posN - 1;
        zw = PreValue / pow(10, pot);
        ziffer = ((int) zw) % 10;
        input[posN] = ziffer + 48;

        posN = findDelimiterPos(input, "N");
    }

///////////////////////////// TestCode
/*
    input = "10";
    posPunkt = input.length();
    PreValue = 9.5;
    lastvalueanalog = 7;
*/
    if (checkDigitIncreaseConsistency && lastvalueanalog > -1)
    {
        int zifferIST;
//        int substrakt = 0;
        bool lastcorrected = false;
        for (int i = input.length() - 1; i >= 0; --i)
        {
            zifferIST = input[i] - 48;  //std::stoi(std::string(input[i]));
            if (lastcorrected)
            {
                zifferIST--;
                input[i] = zifferIST + 48;
                lastcorrected = false;
            }

            pot = posPunkt - i - 1;
            zw = PreValue / pow(10, pot);
            ziffer = ((int) zw) % 10;
            if (zifferIST < ziffer)
            {
                if (lastvalueanalog >= 7)
                {
                    input[i] = ziffer + 48;
                    lastvalueanalog = ziffer;
                    lastcorrected = true;
                }
            }
        
            
        }
        
    }
    return input;
}
