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
    return to_string(PreValue);
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
        if ((zerlegt[0] == "PreValueUse") && (zerlegt.size() > 1))
        {
            if ((zerlegt[1] == "True") || (zerlegt[1] == "true"))
            {
                PreValueUse = true;
                PreValueOkay = LoadPreValue();
                if (PreValueOkay)
                {
                    Value = PreValue;
                    for (int i = 0; i < ListFlowControll->size(); ++i)
                    {
                        if (((*ListFlowControll)[i])->name().compare("ClassFlowAnalog") == 0)
                        {
                            int AnzahlNachkomma = ((ClassFlowAnalog*)(*ListFlowControll)[i])->AnzahlROIs();
                            std::stringstream stream;
                            stream << std::fixed << std::setprecision(AnzahlNachkomma) << Value;
                            ReturnValue = stream.str();
                        }
                    }
                }
            }
        }
        if ((zerlegt[0] == "CheckDigitIncreaseConsistency") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                checkDigitIncreaseConsistency = true;
        }        
        if ((zerlegt[0] == "AllowNegativeRates") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                AllowNegativeRates = true;
        }
        if ((zerlegt[0] == "ErrorMessage") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                ErrorMessage = true;
        }
        if ((zerlegt[0] == "PreValueAgeStartup") && (zerlegt.size() > 1))
        {
            PreValueAgeStartup = std::stoi(zerlegt[1]);
        }
        if ((zerlegt[0] == "MaxRateValue") && (zerlegt.size() > 1))
        {
            useMaxRateValue = true;
            MaxRateValue = std::stof(zerlegt[1]);
        }
    }
    return true;
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


    if (!PreValueUse || !PreValueOkay)
    {
        if (isdigit)
            ReturnValue = digit;
        if (isdigit && isanalog)
            ReturnValue = ReturnValue + ".";
        if (isanalog)
            ReturnValue = ReturnValue + analog;

        if ((findDelimiterPos(ReturnValue, "N") == std::string::npos) && (ReturnValue.length() > 0))
        {
            while ((ReturnValue.length() > 1) && (ReturnValue[0] == '0'))
            {
                ReturnValue.erase(0, 1);
            }
            ReturnRawValue = ReturnValue;
            Value = std::stof(ReturnValue);
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

    ReturnRawValue = zw;

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

    if (useMaxRateValue && (abs(Value - PreValue) > MaxRateValue))
    {
        error = "Rate too high - Returned old value - read value: " + zwvalue;
        Value = PreValue;
        stream.str("");
        stream << std::fixed << std::setprecision(AnzahlNachkomma) << Value;
        zwvalue = stream.str();
    }

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

string ClassFlowPostProcessing::getReadoutParam(bool _rawValue)
{
    if (_rawValue)
        return ReturnRawValue;
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
        int substrakt = 0;
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
