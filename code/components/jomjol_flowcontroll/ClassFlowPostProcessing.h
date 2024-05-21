#pragma once

#ifndef CLASSFFLOWPOSTPROCESSING_H
#define CLASSFFLOWPOSTPROCESSING_H

#include "ClassFlow.h"
#include "ClassFlowTakeImage.h"
#include "ClassFlowCNNGeneral.h"
#include "ClassFlowDefineTypes.h"

#include <string>


class ClassFlowPostProcessing :
    public ClassFlow
{
protected:
    bool UpdatePreValueINI;

    int PreValueAgeStartup; 
    bool ErrorMessage;
    bool IgnoreLeadingNaN;          // SPECIAL CASE for User Gustl ???


    ClassFlowCNNGeneral* flowAnalog;
    ClassFlowCNNGeneral* flowDigit;    


    string FilePreValue;

    ClassFlowTakeImage *flowTakeImage;

    bool LoadPreValue(void);
    string ShiftDecimal(string in, int _decShift);

    string ErsetzteN(string, double _prevalue);
    float checkDigitConsistency(double input, int _decilamshift, bool _isanalog, double _preValue);

    void InitNUMBERS();
    void handleDecimalSeparator(string _decsep, string _value);
    void handleMaxRateValue(string _decsep, string _value);
    void handleDecimalExtendedResolution(string _decsep, string _value); 
    void handleMaxRateType(string _decsep, string _value);
    void handleAnalogDigitalTransitionStart(string _decsep, string _value);
    void handleAllowNegativeRate(string _decsep, string _value);
    
    std::string GetStringReadouts(general);

    void WriteDataLog(int _index);




public:
    bool PreValueUse;
    std::vector<NumberPost*> NUMBERS;


    ClassFlowPostProcessing(std::vector<ClassFlow*>* lfc, ClassFlowCNNGeneral *_analog, ClassFlowCNNGeneral *_digit);
    virtual ~ClassFlowPostProcessing(){};
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getReadout(int _number);
    string getReadoutParam(bool _rawValue, bool _noerror, int _number = 0);
    string getReadoutError(int _number = 0);
    string getReadoutRate(int _number = 0);
    string getReadoutTimeStamp(int _number = 0);
    void SavePreValue();
    string getJsonFromNumber(int i, std::string _lineend);
    string GetPreValue(std::string _number = "");
    bool SetPreValue(double zw, string _numbers, bool _extern = false);

    std::string GetJSON(std::string _lineend = "\n");
    std::string getNumbersName();

    void UpdateNachkommaDecimalShift();

    std::vector<NumberPost*>* GetNumbers(){return &NUMBERS;};

    string name(){return "ClassFlowPostProcessing";};
};


#endif //CLASSFFLOWPOSTPROCESSING_H
