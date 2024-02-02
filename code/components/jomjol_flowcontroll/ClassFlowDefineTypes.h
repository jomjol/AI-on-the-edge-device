#pragma once

#ifndef CLASSFLOWDEFINETYPES_H
#define CLASSFLOWDEFINETYPES_H

#include "ClassFlowImage.h"

struct roi {
    int posx, posy, deltax, deltay;
    float result_float;
    int result_klasse;
    bool isReject, CCW;
    string name;
    CImageBasis *image, *image_org;
};

struct general {
    string name;
    std::vector<roi*> ROI;
};

enum t_RateType {
    AbsoluteChange,
    RateChange
 };


struct NumberPost {
    float MaxRateValue;
    bool useMaxRateValue;
    t_RateType RateType;
    bool ErrorMessage;
    bool PreValueOkay;
    bool AllowNegativeRates;
    bool checkDigitIncreaseConsistency;
    time_t lastvalue;
    time_t timeStampTimeUTC;
    string timeStamp;
    double FlowRateAct; // m3 / min
    double PreValue; // last value that was read out well
    double Value; // last value read out, incl. corrections
    string ReturnRateValue; // return value rate
    string ReturnChangeAbsolute; // return value rate
    string ReturnRawValue; // Raw value (with N & leading 0)    
    string ReturnValue; // corrected return value, if necessary with error message
    string ReturnPreValue; // corrected return value without error message
    string ErrorMessageText; // Error message for consistency check
    int AnzahlAnalog;
    int AnzahlDigital;
    int DecimalShift;
    int DecimalShiftInitial;
    float AnalogDigitalTransitionStart; // When is the digit > x.1, i.e. when does it start to tilt?
    int Nachkomma;

    string FieldV1; // Fieldname in InfluxDBv1  
    string MeasurementV1;   // Measurement in InfluxDBv1

    string FieldV2;         // Fieldname in InfluxDBv2  
    string MeasurementV2;   // Measurement in InfluxDBv2

    bool isExtendedResolution;

    general *digit_roi;
    general *analog_roi;

    string name;
};

#endif

