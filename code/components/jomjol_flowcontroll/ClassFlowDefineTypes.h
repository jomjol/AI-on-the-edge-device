#ifndef __CLASSFLOWIMAGE_CLASS__
#define __CLASSFLOWIMAGE_CLASS__

#include "ClassFlowImage.h"

struct roi {
    int posx, posy, deltax, deltay;
    float result_float;
    int result_klasse;
    string name;
    CImageBasis *image, *image_org;
};

struct general {
    string name;
    std::vector<roi*> ROI;
};


struct NumberPost {
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
    string ReturnRateValue;      // RückgabewertRate
    string ReturnRawValue;      // Rohwert (mit N & führenden 0)    
    string ReturnValue;         // korrigierter Rückgabewert, ggf. mit Fehlermeldung
    string ReturnPreValue;  // korrigierter Rückgabewert ohne Fehlermeldung
    string ReturnValueNoError;
    string ErrorMessageText;        // Fehlermeldung bei Consistency Check
    int AnzahlAnalog;
    int AnzahlDigital;
    int DecimalShift;
    int DecimalShiftInitial;
    int Nachkomma;

    bool isExtendedResolution;

    general *digit_roi;
    general *analog_roi;

    string name;
};

#endif

