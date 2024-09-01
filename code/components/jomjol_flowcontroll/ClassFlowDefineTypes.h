#pragma once

#ifndef CLASSFLOWDEFINETYPES_H
#define CLASSFLOWDEFINETYPES_H

#include "ClassFlowImage.h"

/**
 * Properties of one ROI
 * FIXME: naming of members could use some refactoring to comply with common C++ coding style guidelines
 */
struct roi {
    int posx, posy, deltax, deltay;
    float result_float;
    int result_klasse;
    bool isReject, CCW;
    string name;
    CImageBasis *image, *image_org;
};

/**
 * FIXME: Why is this additional layer needed?
 */
struct general {
    string name;
    std::vector<roi*> ROI;
};

enum t_RateType {
    AbsoluteChange, // ignores the time difference; only the value difference is used comparison with NumberPost.maxRate
    RateChange      // time difference is considered and a normalized rate is used for comparison with NumberPost.maxRate
 };


/**
 * Holds all properties and settings of a sequence. A sequence is a set of digital and/or analog ROIs that are combined to 
 * provide one meter reading (value).
 * FIXME: can be renamed to `Sequence`
 */
struct NumberPost {
    float MaxRateValue;         // maxRate; upper bound for the difference between two consecutive readings; affected by maxRateType;
    bool useMaxRateValue;       // consistencyChecksEnabled; enables consistency checks; uses maxRate and maxRateType
    t_RateType MaxRateType;        // maxRateType; affects how the value of maxRate is used for comparing the current and previous value
    bool ErrorMessage;          // FIXME: not used; can be removed
    int ChangeRateThreshold;  // threshold parameter for negative rate detection
    bool PreValueOkay;          // previousValueValid; indicates that the reading of the previous round has no errors
    bool AllowNegativeRates;    // allowNegativeRate; defines if the consistency checks allow negative rates between consecutive meter readings.
    bool checkDigitIncreaseConsistency; // extendedConsistencyCheck; performs an additional consistency check to avoid wrong readings
    time_t timeStampLastValue;     // Timestamp for the last read value; is used for the log
    time_t timeStampLastPreValue;  // Timestamp for the last PreValue set; is used for useMaxRateValue
    time_t timeStampTimeUTC;    // FIXME: not used; can be removed.
    string timeStamp;           // localTimeStr; timestamp of last valid reading formatted as local time
    double FlowRateAct;         // currentRate; ΔValue/min; since usage is not limited to water meters, the physical unit is not known.
    double PreValue;            // lastValidValue; most recent value that could be read w/o any errors
    double Value;               // value; most recent readout; may include corrections
    string ReturnRateValue;     // currentRateStr; current normalized rate; ΔValue/min
    string ReturnChangeAbsolute; // currentChangeStr; absolute difference between current and previous measurement
    string ReturnRawValue;      // rawValueStr; Raw value (with N & leading 0)    
    string ReturnValue;         // valueStr; corrected return value, if necessary with error message
    string ReturnPreValue;      // lastValidValueStr; corrected return value without error message
    string ErrorMessageText;    // errorMessage; Error message for consistency checks
    int AnzahlAnalog;           // numAnalogRoi; number of analog ROIs used in this sequence
    int AnzahlDigital;          // numDigitalRoi; number of digital ROIs used in this sequence
    int DecimalShift;           // decimalShift; each increment shifts the decimal separator by one digit; value=value*10^decimalShift; pos. value shifts to the right
    int DecimalShiftInitial;    // decimalShiftInitial; same as decimalShift but is a const to reset decimalShift after calculations
    float AnalogDigitalTransitionStart; // analogDigitalTransitionStartValue; FIXME: need a better description; When is the digit > x.1, i.e. when does it start to tilt?
    int Nachkomma;              // decimalPlaces; usually defined by the number of analog ROIs; affected by DecimalShift

    string FieldV1;             // influxdbFieldName_v1; Name of the Field in InfluxDBv1
    string MeasurementV1;       // influxdbMeasurementName_v1; Name of the Measurement in InfluxDBv1

    string FieldV2;             // influxdbFieldName_v2; Name of the Field in InfluxDBv2
    string MeasurementV2;       // influxdbMeasurementName_v2; Name of the Measurement in InfluxDBv2

    bool isExtendedResolution;  // extendResolution; Adds the decimal place of the least significant analog ROI to the value

    general *digit_roi;         // digitalRoi; set of digital ROIs for the sequence
    general *analog_roi;        // analogRoi; set of analog ROIs for the sequence

    string name;                // name; Designation for the sequence 
};

#endif

