#pragma once

#ifndef CLASSFFLOWPOSTPROCESSING_H
#define CLASSFFLOWPOSTPROCESSING_H

#include "ClassFlow.h"
#include "ClassFlowTakeImage.h"
#include "ClassFlowCNNGeneral.h"
#include "ClassFlowDefineTypes.h"

#include <string>
#include <mutex>

class ClassFlowPostProcessing : public ClassFlow
{
protected:
    std::mutex preValueMutex; // PreValue is set from HTTP/MQTT tasks while the flow task post-processes

    bool UpdatePreValueINI;

    int PreValueAgeStartup;
    bool ErrorMessage;

    ClassFlowCNNGeneral *flowAnalog;
    ClassFlowCNNGeneral *flowDigit;

    std::string FilePreValue;

    ClassFlowTakeImage *flowTakeImage;

    bool LoadPreValue(void);
    std::string ShiftDecimal(std::string in, int _decShift);

    std::string ErsetzteN(std::string, double _prevalue);
    double checkDigitConsistency(double input, int _decilamshift, bool _isanalog, double _preValue);

    void InitNUMBERS();

    void handleDecimalSeparator(std::string _decsep, std::string _value);
    void handleMaxRateValue(std::string _decsep, std::string _value);
    void handleDecimalExtendedResolution(std::string _decsep, std::string _value);
    void handleMaxRateType(std::string _decsep, std::string _value);
    void handleAnalogToDigitTransitionStart(std::string _decsep, std::string _value);
    void handleAllowNegativeRate(std::string _decsep, std::string _value);
    void handleIgnoreLeadingNaN(std::string _decsep, std::string _value);
    void handleContinuousLastDigit(std::string _decsep, std::string _value);
    void handleChangeRateThreshold(std::string _decsep, std::string _value);
    void handlecheckDigitIncreaseConsistency(std::string _decsep, std::string _value);

    void WriteDataLog(int _index);

public:
    bool PreValueUse;
    std::vector<NumberPost *> NUMBERS;

    ClassFlowPostProcessing(std::vector<ClassFlow *> *lfc, ClassFlowCNNGeneral *_analog, ClassFlowCNNGeneral *_digit);
    virtual ~ClassFlowPostProcessing() {};

    bool ReadParameter(FILE *pfile, std::string &aktparamgraph);
    bool doFlow(std::string time);
    std::string getReadout(int _number);
    std::string getReadoutParam(bool _rawValue, bool _noerror, int _number = 0);
    std::string getReadoutError(int _number = 0);
    std::string getReadoutRate(int _number = 0);
    std::string getReadoutTimeStamp(int _number = 0);
    void SavePreValue(void);
    std::string getJsonFromNumber(int i, std::string _lineend);
    std::string GetPreValue(std::string _number = "");
    bool SetPreValue(double zw, std::string _numbers, bool _extern = false);

    std::string GetJSON(std::string _lineend = "\n");
    std::string getNumbersName(void);

    void UpdateNachkommaDecimalShift(void);

    std::vector<NumberPost *> *GetNumbers() { return &NUMBERS; };

    std::string name(void) { return "ClassFlowPostProcessing"; };
};

#endif // CLASSFFLOWPOSTPROCESSING_H
