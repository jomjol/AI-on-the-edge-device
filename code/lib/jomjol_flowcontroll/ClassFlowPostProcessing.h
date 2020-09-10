#pragma once
#include "ClassFlow.h"

#include <string>


class ClassFlowPostProcessing :
    public ClassFlow
{
protected:
    bool PreValueUse;
    int PreValueAgeStartup;
    bool AllowNegativeRates;
    float MaxRateValue;
    bool useMaxRateValue;
    bool ErrorMessage;
    bool PreValueOkay;
    bool checkDigitIncreaseConsistency;

    string FilePreValue;
    float PreValue;
    float Value;
    string ReturnValue;
    string ReturnRawValue;
    string ReturnValueNoError;

    bool LoadPreValue(void);


    string ErsetzteN(string, int lastvalueanalog);

public:
    ClassFlowPostProcessing();
    ClassFlowPostProcessing(std::vector<ClassFlow*>* lfc);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getReadout();
    string getReadoutParam(bool _rawValue, bool _noerror);
    void SavePreValue(float value, string time = "");
    string GetPreValue();

    string name(){return "ClassFlowPostProcessing";};
};

