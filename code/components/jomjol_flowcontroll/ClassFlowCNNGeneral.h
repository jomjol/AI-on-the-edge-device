#pragma once

#ifndef CLASSFLOWCNNGENERAL_H
#define CLASSFLOWCNNGENERAL_H

#include "ClassFlowDefineTypes.h"
#include "ClassFlowAlignment.h"

enum t_CNNType
{
    AutoDetect,
    Analogue,
    Analogue100,
    Digit,
    DigitHyprid10,
    DoubleHyprid10,
    Digit100,
    None
};

class ClassFlowCNNGeneral : public ClassFlowImage
{
protected:
    t_CNNType CNNType;
    std::vector<general *> GENERAL;
    float CNNGoodThreshold;

    std::string cnnmodelfile;
    int modelxsize, modelysize, modelchannel;
    bool isLogImageSelect;
    std::string LogImageSelect;
    ClassFlowAlignment *flowpostalignment;

    bool SaveAllFiles;

    int PointerEvalAnalogNew(float zahl, int numeral_preceder);
    int PointerEvalAnalogToDigitNew(float zahl, float numeral_preceder, int eval_predecessors, float AnalogToDigitTransitionStart, bool ContinuousLastDigit = false);
    int PointerEvalHybridNew(float zahl, float number_of_predecessors, int eval_predecessors, bool Analog_Predecessors = false, float AnalogToDigitTransitionStart = 9.2, bool ContinuousLastDigit = false);

    bool doNeuralNetwork(std::string time);
    bool doAlignAndCut(std::string time);

    bool getNetworkParameter(void);

public:
    ClassFlowCNNGeneral(ClassFlowAlignment *_flowalign, t_CNNType _cnntype = AutoDetect);

    bool ReadParameter(FILE *pfile, std::string &aktparamgraph);
    bool doFlow(std::string time);

    std::string getHTMLSingleStep(std::string host);
    std::string getReadout(int _analog, bool _extendedResolution = false, int prev = -1, float _before_narrow_Analog = -1, float AnalogToDigitTransitionStart = 9.2, bool ContinuousLastDigit = false);

    std::string getReadoutRawString(int _analog);

    void DrawROI(CImageBasis *_zw);

    std::vector<HTMLInfo *> GetHTMLInfo();

    int getNumberGENERAL(void);
    general *GetGENERAL(int _analog);
    general *GetGENERAL(std::string _name, bool _create);
    general *FindGENERAL(std::string _name_number);
    std::string getNameGENERAL(int _analog);

    bool isExtendedResolution(int _number = 0);

    void UpdateNameNumbers(std::vector<std::string> *_name_numbers);

    t_CNNType getCNNType(void) { return CNNType; };

    std::string name(void) { return "ClassFlowCNNGeneral"; };
};

#endif
