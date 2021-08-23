#pragma once
#include "ClassFlow.h"
#include "ClassFlowMakeImage.h"
#include "ClassFlowAnalog.h"
#include "ClassFlowDigit.h"


#include <string>


struct NumberPost {
//    int PreValueAgeStartup; 
    float MaxRateValue;
    bool useMaxRateValue;
    bool ErrorMessage;
    bool PreValueOkay;
    bool AllowNegativeRates;
    bool checkDigitIncreaseConsistency;
    time_t lastvalue;
    string timeStamp;
    float FlowRateAct;          // m3 / min
    float PreValue;             // letzter Wert, der gut ausgelesen wurde
    float Value;                // letzer ausgelesener Wert, inkl. Korrekturen
    string ReturnRawValue;      // Rohwert (mit N & führenden 0)    
    string ReturnValue;         // korrigierter Rückgabewert, ggf. mit Fehlermeldung
    string ReturnPreValue;  // korrigierter Rückgabewert ohne Fehlermeldung
    string ReturnValueNoError;
    string ErrorMessageText;        // Fehlermeldung bei Consistency Check
    int AnzahlAnalog;
    int AnzahlDigital;
    int DecimalShift;
    int Nachkomma;
//    ClassFlowAnalog* ANALOG;
//    ClassFlowDigit* DIGIT;

    digit *digit_roi;
    analog *analog_roi;



    string name;
};




class ClassFlowPostProcessing :
    public ClassFlow
{
protected:
    std::vector<NumberPost*> NUMBERS;
    bool UpdatePreValueINI;

    int PreValueAgeStartup; 
    bool ErrorMessage;


    ClassFlowAnalog* flowAnalog;
    ClassFlowDigit* flowDigit;    


    string FilePreValue;

    ClassFlowMakeImage *flowMakeImage;

    bool LoadPreValue(void);
    string ShiftDecimal(string in, int _decShift);

    string ErsetzteN(string, float _prevalue);
    float checkDigitConsistency(float input, int _decilamshift, bool _isanalog, float _preValue);
    string RundeOutput(float _in, int _anzNachkomma);

    void InitNUMBERS();
    void handleDecimalSeparator(string _decsep, string _value);
    void handleMaxRateValue(string _decsep, string _value);


public:
    bool PreValueUse;

    ClassFlowPostProcessing(std::vector<ClassFlow*>* lfc);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getReadout(int _number);
    string getReadoutParam(bool _rawValue, bool _noerror, int _number = 0);
    string getReadoutError(int _number = 0);
    string getReadoutRate(int _number = 0);
    string getReadoutTimeStamp(int _number = 0);
    void SavePreValue();
    string GetPreValue(std::string _number = "");
    void SetPreValue(float zw, string _numbers, bool _extern = false);
    std::vector<NumberPost*> GetNumbers(){return NUMBERS;};

    string name(){return "ClassFlowPostProcessing";};
};

