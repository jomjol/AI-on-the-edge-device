#include "ClassFlowPostProcessing.h"
#include "Helper.h"
#include "ClassFlowMakeImage.h"
#include "ClassLogFile.h"

#include <iomanip>
#include <sstream>

#include <time.h>

#include "time_sntp.h"


#define PREVALUE_TIME_FORMAT_OUTPUT "%Y-%m-%dT%H:%M:%S"
#define PREVALUE_TIME_FORMAT_INPUT "%d-%d-%dT%d:%d:%d"


std::string ClassFlowPostProcessing::GetJSON(std::string _id, std::string _mac, std::string _lineend)
{
    std::string json="{" + _lineend;

    for (int i = 0; i < NUMBERS.size(); ++i)
    {
        json += "\"" + NUMBERS[i]->name + "\":"  + _lineend;
        json += "  {"  + _lineend;

        if (_id.length() > 0)
            json += "    \"ID\": \"" + _id + "\","  + _lineend;
        if (_mac.length() > 0)
            json += "    \"MAC\": \"" + _mac + "\","  + _lineend;

        if (NUMBERS[i]->ReturnValue.length() > 0)
            json += "    \"value\": \""      + NUMBERS[i]->ReturnValue          + "\"," + _lineend;
        else
            json += "    \"value\": \"\","  + _lineend;
        json += "    \"raw\": \""        + NUMBERS[i]->ReturnRawValue              + "\","  + _lineend;
        json += "    \"error\": \""     + NUMBERS[i]->ErrorMessageText             + "\","  + _lineend;
        if (NUMBERS[i]->ReturnRateValue.length() > 0)
            json += "    \"rate\": "      + NUMBERS[i]->ReturnRateValue                + ","  + _lineend;
        else
            json += "    \"rate\": \"\","  + _lineend;

        json += "    \"timestamp\": \"" + NUMBERS[i]->timeStamp                    + "\""  + _lineend;
        if ((i+1) < NUMBERS.size())
            json += "  }," + _lineend;
        else
            json += "  }" + _lineend;
    }
    json += "}";

    return json;
}

string ClassFlowPostProcessing::GetPreValue(std::string _number)
{
    std::string result;
    int index = -1;

    if (_number == "")
        _number = "default";

    for (int i = 0; i < NUMBERS.size(); ++i)
        if (NUMBERS[i]->name == _number)
            index = i;

    result = RundeOutput(NUMBERS[index]->PreValue, NUMBERS[index]->Nachkomma);

    return result;
}

void ClassFlowPostProcessing::SetPreValue(float zw, string _numbers, bool _extern)
{
    printf("SetPrevalue: %f, %s\n", zw, _numbers.c_str());
    for (int j = 0; j < NUMBERS.size(); ++j)
    {
//        printf("Number %d, %s\n", j, NUMBERS[j]->name.c_str());
        if (NUMBERS[j]->name == _numbers)
        {
            NUMBERS[j]->PreValue = zw;
            NUMBERS[j]->ReturnPreValue = std::to_string(zw);
            NUMBERS[j]->PreValueOkay = true;
            if (_extern)
            {
                time(&(NUMBERS[j]->lastvalue));
                localtime(&(NUMBERS[j]->lastvalue));
            }
//            printf("Found %d! - set to %f\n", j,  NUMBERS[j]->PreValue);
        }
    }
    UpdatePreValueINI = true;
    SavePreValue();
}


bool ClassFlowPostProcessing::LoadPreValue(void)
{
    std::vector<string> zerlegt;
    FILE* pFile;
    char zw[1024];
    string zwtime, zwvalue, name;
    bool _done = false;

    UpdatePreValueINI = false;       // Konvertierung ins neue Format


    pFile = fopen(FilePreValue.c_str(), "r");
    if (pFile == NULL)
        return false;

    fgets(zw, 1024, pFile);
    printf("Read Zeile Prevalue.ini: %s", zw);
    zwtime = trim(std::string(zw));
    if (zwtime.length() == 0)
        return false;

    zerlegt = HelperZerlegeZeile(zwtime, "\t");
    if (zerlegt.size() > 1)     // neues Format
    {
        while ((zerlegt.size() > 1) && !_done)
        {
            name = trim(zerlegt[0]);
            zwtime = trim(zerlegt[1]);
            zwvalue = trim(zerlegt[2]);

            for (int j = 0; j < NUMBERS.size(); ++j)
            {
                if (NUMBERS[j]->name == name)
                {
                    NUMBERS[j]->PreValue = stof(zwvalue.c_str());
                    NUMBERS[j]->ReturnPreValue = RundeOutput(NUMBERS[j]->PreValue, NUMBERS[j]->Nachkomma + 1);      // SIcherheitshalber 1 Stelle mehr, da ggf. Exgtended Resolution an ist (wird erst beim ersten Durchlauf gesetzt)

                    time_t tStart;
                    int yy, month, dd, hh, mm, ss;
                    struct tm whenStart;

                    sscanf(zwtime.c_str(), PREVALUE_TIME_FORMAT_INPUT, &yy, &month, &dd, &hh, &mm, &ss);
                    whenStart.tm_year = yy - 1900;
                    whenStart.tm_mon = month - 1;
                    whenStart.tm_mday = dd;
                    whenStart.tm_hour = hh;
                    whenStart.tm_min = mm;
                    whenStart.tm_sec = ss;
                    whenStart.tm_isdst = -1;

                    NUMBERS[j]->lastvalue = mktime(&whenStart);

                    time(&tStart);
                    localtime(&tStart);
                    double difference = difftime(tStart, NUMBERS[j]->lastvalue);
                    difference /= 60;
                    if (difference > PreValueAgeStartup)
                        NUMBERS[j]->PreValueOkay = false;
                    else
                        NUMBERS[j]->PreValueOkay = true;
                }
            }

            if (!fgets(zw, 1024, pFile))
                _done = true;
            else
            {
                printf("Read Zeile Prevalue.ini: %s", zw);
                zerlegt = HelperZerlegeZeile(trim(std::string(zw)), "\t");
                if (zerlegt.size() > 1)
                {
                    name = trim(zerlegt[0]);
                    zwtime = trim(zerlegt[1]);
                    zwvalue = trim(zerlegt[2]);
                }
            }
        }
        fclose(pFile);
    }   
    else        // altes Format
    {
        fgets(zw, 1024, pFile);
        fclose(pFile);
        printf("%s", zw);
        zwvalue = trim(std::string(zw));
        NUMBERS[0]->PreValue = stof(zwvalue.c_str());

        time_t tStart;
        int yy, month, dd, hh, mm, ss;
        struct tm whenStart;

        sscanf(zwtime.c_str(), PREVALUE_TIME_FORMAT_INPUT, &yy, &month, &dd, &hh, &mm, &ss);
        whenStart.tm_year = yy - 1900;
        whenStart.tm_mon = month - 1;
        whenStart.tm_mday = dd;
        whenStart.tm_hour = hh;
        whenStart.tm_min = mm;
        whenStart.tm_sec = ss;
        whenStart.tm_isdst = -1;

        printf("TIME: %d, %d, %d, %d, %d, %d\n", whenStart.tm_year, whenStart.tm_mon, whenStart.tm_wday, whenStart.tm_hour, whenStart.tm_min, whenStart.tm_sec);

        NUMBERS[0]->lastvalue = mktime(&whenStart);

        time(&tStart);
        localtime(&tStart);
        double difference = difftime(tStart, NUMBERS[0]->lastvalue);
        difference /= 60;
        if (difference > PreValueAgeStartup)
            return false;

        NUMBERS[0]->Value = NUMBERS[0]->PreValue;
        NUMBERS[0]->ReturnValue = to_string(NUMBERS[0]->Value);

        if (NUMBERS[0]->digit_roi || NUMBERS[0]->analog_roi)
        {
            NUMBERS[0]->ReturnValue = RundeOutput(NUMBERS[0]->Value, NUMBERS[0]->Nachkomma);
        }

        UpdatePreValueINI = true;       // Konvertierung ins neue Format
        SavePreValue();
    } 

    return true;
}

void ClassFlowPostProcessing::SavePreValue()
{
    FILE* pFile;
    string _zw;

    if (!UpdatePreValueINI)         // PreValues unverändert --> File muss nicht neu geschrieben werden
        return;

    pFile = fopen(FilePreValue.c_str(), "w");

    for (int j = 0; j < NUMBERS.size(); ++j)
    {
        char buffer[80];
        struct tm* timeinfo = localtime(&NUMBERS[j]->lastvalue);
        strftime(buffer, 80, PREVALUE_TIME_FORMAT_OUTPUT, timeinfo);
        NUMBERS[j]->timeStamp = std::string(buffer);
//        printf("SaverPreValue %d, Value: %f, Nachkomma %d\n", j, NUMBERS[j]->PreValue, NUMBERS[j]->Nachkomma);

        _zw = NUMBERS[j]->name + "\t" + NUMBERS[j]->timeStamp + "\t" + RundeOutput(NUMBERS[j]->PreValue, NUMBERS[j]->Nachkomma) + "\n";
        printf("Write PreValue Zeile: %s\n", _zw.c_str());
        if (pFile) {
            fputs(_zw.c_str(), pFile);
        }
    }

    UpdatePreValueINI = false;

    fclose(pFile);
}


ClassFlowPostProcessing::ClassFlowPostProcessing(std::vector<ClassFlow*>* lfc, ClassFlowCNNGeneral *_analog, ClassFlowCNNGeneral *_digit)
{
    PreValueUse = false;
    PreValueAgeStartup = 30;
    ErrorMessage = false;
    ListFlowControll = NULL;
    FilePreValue = FormatFileName("/sdcard/config/prevalue.ini");
    ListFlowControll = lfc;
    flowMakeImage = NULL;
    UpdatePreValueINI = false;
    IgnoreLeadingNaN = false;
    flowAnalog = _analog;
    flowDigit = _digit;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowMakeImage") == 0)
        {
            flowMakeImage = (ClassFlowMakeImage*) (*ListFlowControll)[i];
        }
    }
}

void ClassFlowPostProcessing::handleDecimalExtendedResolution(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
//    printf("Name: %s, Pospunkt: %d\n", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1)
        _digit = _decsep.substr(0, _pospunkt);
    else
        _digit = "default";

    for (int j = 0; j < NUMBERS.size(); ++j)
    {
        bool _zwdc = false;

        if (toUpper(_value) == "TRUE")
            _zwdc = true;
     
        if (_digit == "default")                        // erstmal auf default setzen (falls sonst nichts gesetzt)
        {
            NUMBERS[j]->isExtendedResolution = _zwdc;
        }

        if (NUMBERS[j]->name == _digit)
        {
            NUMBERS[j]->isExtendedResolution = _zwdc;
        }
    }
}


void ClassFlowPostProcessing::handleDecimalSeparator(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
//    printf("Name: %s, Pospunkt: %d\n", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1)
        _digit = _decsep.substr(0, _pospunkt);
    else
        _digit = "default";

    for (int j = 0; j < NUMBERS.size(); ++j)
    {
        int _zwdc = 0;

//        try
        {
            _zwdc = stoi(_value);
        }
/*        catch(const std::exception& e)
        {
            printf("ERROR - Decimalshift is not a number: %s\n", _value.c_str());
        }
*/        
        if (_digit == "default")                        // erstmal auf default setzen (falls sonst nichts gesetzt)
        {
            NUMBERS[j]->DecimalShift = _zwdc;
            NUMBERS[j]->DecimalShiftInitial = _zwdc;
        }

        if (NUMBERS[j]->name == _digit)
        {
            NUMBERS[j]->DecimalShift = _zwdc;
            NUMBERS[j]->DecimalShiftInitial = _zwdc;
        }

        NUMBERS[j]->Nachkomma = NUMBERS[j]->AnzahlAnalog - NUMBERS[j]->DecimalShift;
    }
}



void ClassFlowPostProcessing::handleMaxRateType(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
//    printf("Name: %s, Pospunkt: %d\n", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1)
        _digit = _decsep.substr(0, _pospunkt);
    else
        _digit = "default";

    for (int j = 0; j < NUMBERS.size(); ++j)
    {
        t_RateType _rt = AbsoluteChange;

        if (toUpper(_value) == "RATECHANGE")
            _rt = RateChange;

        if (_digit == "default")                        // erstmal auf default setzen (falls sonst nichts gesetzt)
        {
            NUMBERS[j]->RateType = _rt;
        }

        if (NUMBERS[j]->name == _digit)
        {
            NUMBERS[j]->RateType = _rt;
        }
    }
}



void ClassFlowPostProcessing::handleMaxRateValue(string _decsep, string _value)
{
    string _digit, _decpos;
    int _pospunkt = _decsep.find_first_of(".");
//    printf("Name: %s, Pospunkt: %d\n", _decsep.c_str(), _pospunkt);
    if (_pospunkt > -1)
        _digit = _decsep.substr(0, _pospunkt);
    else
        _digit = "default";

    for (int j = 0; j < NUMBERS.size(); ++j)
    {
        float _zwdc = 1;

//        try
        {
            _zwdc = stof(_value);
        }
/*        catch(const std::exception& e)
        {
            printf("ERROR - MaxRateValue is not a number: %s\n", _value.c_str());
        }
*/

        if (_digit == "default")                        // erstmal auf default setzen (falls sonst nichts gesetzt)
        {
            NUMBERS[j]->useMaxRateValue = true;
            NUMBERS[j]->MaxRateValue = _zwdc;
        }

        if (NUMBERS[j]->name == _digit)
        {
            NUMBERS[j]->useMaxRateValue = true;
            NUMBERS[j]->MaxRateValue = _zwdc;
        }
    }
}


bool ClassFlowPostProcessing::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;
    int _n;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if (aktparamgraph.compare("[PostProcessing]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    InitNUMBERS();


    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        std::string _param = GetParameterName(zerlegt[0]);

        if ((toUpper(_param) == "EXTENDEDRESOLUTION") && (zerlegt.size() > 1))
        {
            handleDecimalExtendedResolution(zerlegt[0], zerlegt[1]);
        }

        if ((toUpper(_param) == "DECIMALSHIFT") && (zerlegt.size() > 1))
        {
            handleDecimalSeparator(zerlegt[0], zerlegt[1]);
        }
        if ((toUpper(_param) == "MAXRATEVALUE") && (zerlegt.size() > 1))
        {
            handleMaxRateValue(zerlegt[0], zerlegt[1]);
        }
        if ((toUpper(_param) == "MAXRATETYPE") && (zerlegt.size() > 1))
        {
            handleMaxRateType(zerlegt[0], zerlegt[1]);
        }

        if ((toUpper(_param) == "PREVALUEUSE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                PreValueUse = true;
            }
        }
        if ((toUpper(_param) == "CHECKDIGITINCREASECONSISTENCY") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                for (_n = 0; _n < NUMBERS.size(); ++_n)
                    NUMBERS[_n]->checkDigitIncreaseConsistency = true;
        }        
        if ((toUpper(_param) == "ALLOWNEGATIVERATES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                for (_n = 0; _n < NUMBERS.size(); ++_n)
                    NUMBERS[_n]->AllowNegativeRates = true;
        }
        if ((toUpper(_param) == "ERRORMESSAGE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                ErrorMessage = true;
        }
        if ((toUpper(_param) == "IGNORELEADINGNAN") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                IgnoreLeadingNaN = true;
        }

        
        if ((toUpper(_param) == "PREVALUEAGESTARTUP") && (zerlegt.size() > 1))
        {
            PreValueAgeStartup = std::stoi(zerlegt[1]);
        }
    }

    if (PreValueUse) {
        LoadPreValue();
    }

    return true;
}

void ClassFlowPostProcessing::InitNUMBERS()
{
    int anzDIGIT = 0;
    int anzANALOG = 0;
    std::vector<std::string> name_numbers;

    if (flowDigit)
    {
        anzDIGIT = flowDigit->getAnzahlGENERAL();
        flowDigit->UpdateNameNumbers(&name_numbers);
    }
    if (flowAnalog)
    {
        anzANALOG = flowAnalog->getAnzahlGENERAL();
        flowAnalog->UpdateNameNumbers(&name_numbers);
    }

    printf("Anzahl NUMBERS: %d - DIGITS: %d, ANALOG: %d\n", name_numbers.size(), anzDIGIT, anzANALOG);

    for (int _num = 0; _num < name_numbers.size(); ++_num)
    {
        NumberPost *_number = new NumberPost;

        _number->name = name_numbers[_num];
        
        _number->digit_roi = NULL;
        if (flowDigit)
            _number->digit_roi = flowDigit->FindGENERAL(name_numbers[_num]);
        
        if (_number->digit_roi)
            _number->AnzahlDigital = _number->digit_roi->ROI.size();
        else
            _number->AnzahlDigital = 0;

        _number->analog_roi = NULL;
        if (flowAnalog)
            _number->analog_roi = flowAnalog->FindGENERAL(name_numbers[_num]);


        if (_number->analog_roi)
            _number->AnzahlAnalog = _number->analog_roi->ROI.size();
        else
            _number->AnzahlAnalog = 0;

        _number->ReturnRawValue = "";      // Rohwert (mit N & führenden 0)    
        _number->ReturnValue = "";         // korrigierter Rückgabewert, ggf. mit Fehlermeldung
        _number->ErrorMessageText = "";        // Fehlermeldung bei Consistency Check
        _number->ReturnPreValue = "";
        _number->PreValueOkay = false;
        _number->AllowNegativeRates = false;
        _number->MaxRateValue = 0.1;
        _number->RateType = AbsoluteChange;
        _number->useMaxRateValue = false;
        _number->checkDigitIncreaseConsistency = false;
        _number->DecimalShift = 0;
        _number->DecimalShiftInitial = 0;
        _number->isExtendedResolution = false;


        _number->FlowRateAct = 0;          // m3 / min
        _number->PreValue = 0;             // letzter Wert, der gut ausgelesen wurde
        _number->Value = 0;                // letzer ausgelesener Wert, inkl. Korrekturen
        _number->ReturnRawValue = "";      // Rohwert (mit N & führenden 0)    
        _number->ReturnValue = "";         // korrigierter Rückgabewert, ggf. mit Fehlermeldung
        _number->ErrorMessageText = "";        // Fehlermeldung bei Consistency Check

        _number->Nachkomma = _number->AnzahlAnalog;

        NUMBERS.push_back(_number);
    }

    for (int i = 0; i < NUMBERS.size(); ++i) {
        printf("Number %s, Anz DIG: %d, Anz ANA %d\n", NUMBERS[i]->name.c_str(), NUMBERS[i]->AnzahlDigital, NUMBERS[i]->AnzahlAnalog);
    }

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
    string zw;
    time_t imagetime = 0;
    string rohwert;

    // Update Nachkomma, da sich beim Wechsel von CNNType Auto --> xyz auch die Nachkommastellen ändern können:

    imagetime = flowMakeImage->getTimeImageTaken();
    if (imagetime == 0)
        time(&imagetime);

    struct tm* timeinfo;
    timeinfo = localtime(&imagetime);
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%dT%H:%M:%S", timeinfo);
    zwtime = std::string(strftime_buf);

    printf("Anzahl NUMBERS: %d\n", NUMBERS.size());

    for (int j = 0; j < NUMBERS.size(); ++j)
    {
        NUMBERS[j]->ReturnRawValue = "";
        NUMBERS[j]->ReturnRateValue = "";
        NUMBERS[j]->ReturnValue = "";
        NUMBERS[j]->ErrorMessageText = "";
        NUMBERS[j]->Value = -1;

        UpdateNachkommaDecimalShift();

        int previous_value = -1;

        if (NUMBERS[j]->analog_roi)
        {
            NUMBERS[j]->ReturnRawValue = flowAnalog->getReadout(j, NUMBERS[j]->isExtendedResolution); 
            if (NUMBERS[j]->ReturnRawValue.length() > 0)
            {
                char zw = NUMBERS[j]->ReturnRawValue[0];
                if (zw >= 48 && zw <=57)
                    previous_value = zw - 48;
            }
        }

        if (NUMBERS[j]->digit_roi && NUMBERS[j]->analog_roi)
            NUMBERS[j]->ReturnRawValue = "." + NUMBERS[j]->ReturnRawValue;

        if (NUMBERS[j]->digit_roi)
        {
            if (NUMBERS[j]->analog_roi) 
                NUMBERS[j]->ReturnRawValue = flowDigit->getReadout(j, false, previous_value, NUMBERS[j]->analog_roi->ROI[0]->result_float) + NUMBERS[j]->ReturnRawValue;
            else
                NUMBERS[j]->ReturnRawValue = flowDigit->getReadout(j, NUMBERS[j]->isExtendedResolution, previous_value);        // Extended Resolution nur falls es keine analogen Ziffern gibt
        }

        NUMBERS[j]->ReturnRawValue = ShiftDecimal(NUMBERS[j]->ReturnRawValue, NUMBERS[j]->DecimalShift);

        printf("ReturnRaw %s", NUMBERS[j]->ReturnRawValue.c_str());  


        if (IgnoreLeadingNaN)               
            while ((NUMBERS[j]->ReturnRawValue.length() > 1) && (NUMBERS[j]->ReturnRawValue[0] == 'N'))
                NUMBERS[j]->ReturnRawValue.erase(0, 1);

        NUMBERS[j]->ReturnValue = NUMBERS[j]->ReturnRawValue;

        if (findDelimiterPos(NUMBERS[j]->ReturnValue, "N") != std::string::npos)
        {
            if (PreValueUse && NUMBERS[j]->PreValueOkay)
                NUMBERS[j]->ReturnValue = ErsetzteN(NUMBERS[j]->ReturnValue, NUMBERS[j]->PreValue); 
            else
                continue; // es gibt keinen Zahl, da noch ein N vorhanden ist.
        }

        // Lösche führende Nullen (außer es ist nur noch einen 0)
        while ((NUMBERS[j]->ReturnValue.length() > 1) && (NUMBERS[j]->ReturnValue[0] == '0'))
            NUMBERS[j]->ReturnValue.erase(0, 1);

        NUMBERS[j]->Value = std::stof(NUMBERS[j]->ReturnValue);

        if (NUMBERS[j]->checkDigitIncreaseConsistency)
        {
            NUMBERS[j]->Value = checkDigitConsistency(NUMBERS[j]->Value, NUMBERS[j]->DecimalShift, NUMBERS[j]->analog_roi != NULL, NUMBERS[j]->PreValue);
        }



        if (!NUMBERS[j]->AllowNegativeRates)
        {
            if (NUMBERS[j]->Value < NUMBERS[j]->PreValue)
            {
                NUMBERS[j]->ErrorMessageText = NUMBERS[j]->ErrorMessageText + "Neg. Rate - Read: " + zwvalue + " - Raw: " + NUMBERS[j]->ReturnRawValue + " - Pre: " + RundeOutput(NUMBERS[j]->PreValue, NUMBERS[j]->Nachkomma) + " "; 
                NUMBERS[j]->Value = NUMBERS[j]->PreValue;
                NUMBERS[j]->ReturnValue = "";
                continue;
            }
        }

        double difference = difftime(imagetime, NUMBERS[j]->lastvalue);      // in Sekunden
        difference /= 60;  
        NUMBERS[j]->FlowRateAct = (NUMBERS[j]->Value - NUMBERS[j]->PreValue) / difference;
        NUMBERS[j]->ReturnRateValue =  to_string(NUMBERS[j]->FlowRateAct);

        if (NUMBERS[j]->useMaxRateValue && PreValueUse && NUMBERS[j]->PreValueOkay)
        {
            float _ratedifference;  
            if (NUMBERS[j]->RateType == RateChange)
                _ratedifference = NUMBERS[j]->FlowRateAct;
            else
                _ratedifference = (NUMBERS[j]->Value - NUMBERS[j]->PreValue);

            if (abs(_ratedifference) > abs(NUMBERS[j]->MaxRateValue))
            {
                NUMBERS[j]->ErrorMessageText = NUMBERS[j]->ErrorMessageText + "Rate too high - Read: " + RundeOutput(NUMBERS[j]->Value, NUMBERS[j]->Nachkomma) + " - Pre: " + RundeOutput(NUMBERS[j]->PreValue, NUMBERS[j]->Nachkomma);
                NUMBERS[j]->Value = NUMBERS[j]->PreValue;
                NUMBERS[j]->ReturnValue = "";
                NUMBERS[j]->ReturnRateValue = "";
                continue;
            }
        }

        NUMBERS[j]->ReturnChangeAbsolute = RundeOutput(NUMBERS[j]->Value - NUMBERS[j]->PreValue, NUMBERS[j]->Nachkomma);                                                
        NUMBERS[j]->lastvalue = imagetime;
        NUMBERS[j]->PreValue = NUMBERS[j]->Value;
        NUMBERS[j]->PreValueOkay = true;


        NUMBERS[j]->ReturnValue = RundeOutput(NUMBERS[j]->Value, NUMBERS[j]->Nachkomma);
        NUMBERS[j]->ReturnPreValue = RundeOutput(NUMBERS[j]->PreValue, NUMBERS[j]->Nachkomma);

        NUMBERS[j]->ErrorMessageText = "no error";
        UpdatePreValueINI = true;

        string _zw = "PostProcessing - Raw: " + NUMBERS[j]->ReturnRawValue + " Value: " + NUMBERS[j]->ReturnValue + " Error: " + NUMBERS[j]->ErrorMessageText;
        LogFile.WriteToFile(_zw);
    }

    SavePreValue();
    return true;
}


void ClassFlowPostProcessing::UpdateNachkommaDecimalShift()
{
    for (int j = 0; j < NUMBERS.size(); ++j)
    {
        if (NUMBERS[j]->digit_roi && !NUMBERS[j]->analog_roi)            // es gibt nur digitale ziffern
        {
//            printf("Nurdigital\n");
            NUMBERS[j]->DecimalShift = NUMBERS[j]->DecimalShiftInitial;

            if (NUMBERS[j]->isExtendedResolution && flowDigit->isExtendedResolution())  // extended resolution ist an und soll auch bei dieser Ziffer verwendet werden
                NUMBERS[j]->DecimalShift = NUMBERS[j]->DecimalShift-1;

            NUMBERS[j]->Nachkomma = -NUMBERS[j]->DecimalShift;
        }

        if (!NUMBERS[j]->digit_roi && NUMBERS[j]->analog_roi)            // es gibt nur analoge ziffern
        {
//            printf("Nur analog\n");
            NUMBERS[j]->DecimalShift = NUMBERS[j]->DecimalShiftInitial;
            if (NUMBERS[j]->isExtendedResolution && flowAnalog->isExtendedResolution())  // extended resolution ist an und soll auch bei dieser Ziffer verwendet werden
                NUMBERS[j]->DecimalShift = NUMBERS[j]->DecimalShift-1;

            NUMBERS[j]->Nachkomma = -NUMBERS[j]->DecimalShift;
        }

        if (NUMBERS[j]->digit_roi && NUMBERS[j]->analog_roi)            // digital + analog
        {
//            printf("Nur digital + analog\n");

            NUMBERS[j]->DecimalShift = NUMBERS[j]->DecimalShiftInitial;
            NUMBERS[j]->Nachkomma = NUMBERS[j]->analog_roi->ROI.size() - NUMBERS[j]->DecimalShift;

            if (NUMBERS[j]->isExtendedResolution && flowAnalog->isExtendedResolution())  // extended resolution ist an und soll auch bei dieser Ziffer verwendet werden
                NUMBERS[j]->Nachkomma = NUMBERS[j]->Nachkomma+1;

        }

        printf("UpdateNachkommaDecShift NUMBER%i: Nachkomma %i, DecShift %i\n", j, NUMBERS[j]->Nachkomma,NUMBERS[j]->DecimalShift);
    }
}


string ClassFlowPostProcessing::getReadout(int _number)
{
    return NUMBERS[_number]->ReturnValue;
}

string ClassFlowPostProcessing::getReadoutParam(bool _rawValue, bool _noerror, int _number)
{
    if (_rawValue)
        return NUMBERS[_number]->ReturnRawValue;
    if (_noerror)
        return NUMBERS[_number]->ReturnValue;
    return NUMBERS[_number]->ReturnValue;
}

string ClassFlowPostProcessing::RundeOutput(double _in, int _anzNachkomma){
    std::stringstream stream;
    int _zw = _in;    
//    printf("AnzNachkomma: %d\n", _anzNachkomma);

    if (_anzNachkomma < 0) {
        _anzNachkomma = 0;
    }

    if (_anzNachkomma > 0)
    {
        stream << std::fixed << std::setprecision(_anzNachkomma) << _in;
        return stream.str();          
    }
    else
    {
        stream << _zw;
    }


    return stream.str();  
}


string ClassFlowPostProcessing::ErsetzteN(string input, float _prevalue)
{
    int posN, posPunkt;
    int pot, ziffer;
    float zw;

    posN = findDelimiterPos(input, "N");
    posPunkt = findDelimiterPos(input, ".");
    if (posPunkt == std::string::npos){
        posPunkt = input.length();
    }

    while (posN != std::string::npos)
    {
        if (posN < posPunkt) {
            pot = posPunkt - posN - 1;
        }
        else {
            pot = posPunkt - posN;
        }

        zw =_prevalue / pow(10, pot);
        ziffer = ((int) zw) % 10;
        input[posN] = ziffer + 48;

        posN = findDelimiterPos(input, "N");
    }

    return input;
}

float ClassFlowPostProcessing::checkDigitConsistency(float input, int _decilamshift, bool _isanalog, float _preValue){
    int aktdigit, olddigit;
    int aktdigit_before, olddigit_before;
    int pot, pot_max;
    float zw;
    bool no_nulldurchgang = false;

    pot = _decilamshift;
    if (!_isanalog)             // falls es keine analogwerte gibt, kann die letzte nicht bewertet werden
    {
        pot++;
    }
    pot_max = ((int) log10(input)) + 1;

    while (pot <= pot_max)
    {
        zw = input / pow(10, pot-1);
        aktdigit_before = ((int) zw) % 10;
        zw = _preValue / pow(10, pot-1);
        olddigit_before = ((int) zw) % 10;

        zw = input / pow(10, pot);
        aktdigit = ((int) zw) % 10;
        zw = _preValue / pow(10, pot);
        olddigit = ((int) zw) % 10;

        no_nulldurchgang = (olddigit_before <= aktdigit_before);

        if (no_nulldurchgang)
        {
            if (aktdigit != olddigit) 
            {
                input = input + ((float) (olddigit - aktdigit)) * pow(10, pot);     // Neue Digit wird durch alte Digit ersetzt;
            }
        }
        else
        {
            if (aktdigit == olddigit)                   // trotz Nulldurchgang wurde Stelle nicht hochgezählt --> addiere 1
            {
                input = input + ((float) (1)) * pow(10, pot);   // addiere 1 an der Stelle
            }
        }

        pot++;
    }

    return input;
}

string ClassFlowPostProcessing::getReadoutRate(int _number)
{
    return std::to_string(NUMBERS[_number]->FlowRateAct);
}

string ClassFlowPostProcessing::getReadoutTimeStamp(int _number)
{
   return NUMBERS[_number]->timeStamp; 
}


string ClassFlowPostProcessing::getReadoutError(int _number) 
{
    return NUMBERS[_number]->ErrorMessageText;
}
