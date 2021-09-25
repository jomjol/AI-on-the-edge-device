#ifndef __FLOWPOSTPROCESSING__
#define __FLOWPOSTPROCESSING__

#include "ClassFlow.h"
#include "ClassFlowMakeImage.h"
#include "ClassFlowCNNGeneral.h"
#include "ClassFlowDefineTypes.h"

#include <string>

class ClassFlowPostProcessing :
    public ClassFlow
{
protected:
    std::vector<NumberPost*> NUMBERS;
    bool UpdatePreValueINI;

    int PreValueAgeStartup; 
    bool ErrorMessage;
    bool IgnoreLeadingNaN;          // SPEZIALFALL für User Gustl


    ClassFlowCNNGeneral* flowAnalog;
    ClassFlowCNNGeneral* flowDigit;    


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
    void handleDecimalExtendedResolution(string _decsep, string _value);    



public:
    bool PreValueUse;

    ClassFlowPostProcessing(std::vector<ClassFlow*>* lfc, ClassFlowCNNGeneral *_analog, ClassFlowCNNGeneral *_digit);
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

    void UpdateNachkommaDecimalShift();

    std::vector<NumberPost*>* GetNumbers(){return &NUMBERS;};

    string name(){return "ClassFlowPostProcessing";};
};


#endif
