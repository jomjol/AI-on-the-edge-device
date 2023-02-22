#pragma once
#ifndef TEST_FLOW_H
#define TEST_FLOW_H
#include <unity.h>
#include <ClassFlowPostProcessing.h>
#include <ClassFlowCNNGeneral.h>
#include <ClassFlowTakeImage.h>
#include <Helper.h>


class UnderTestPost : public ClassFlowPostProcessing {
    public:
        UnderTestPost(std::vector<ClassFlow*>* lfc, ClassFlowCNNGeneral *_analog, ClassFlowCNNGeneral *_digit)
            : ClassFlowPostProcessing::ClassFlowPostProcessing(lfc, _analog, _digit) {}
        
        using ClassFlowPostProcessing::InitNUMBERS;
        using ClassFlowPostProcessing::flowAnalog;
        using ClassFlowPostProcessing::flowDigit;    

};

/**
 * @brief Set the Up Class Flow Postprocessing object
 * 
 * @param digType the model type of digits
 * @param anaType the model type of analog
 * @return UnderTestPost* a created, but not setted up testobject
 */
UnderTestPost* setUpClassFlowPostprocessing(t_CNNType digType, t_CNNType anaType);

/**
 * @brief creates a testobject (including setup). AnalogType is Class100, because all analog types do the same.
 * 
 * @param analog the analog recognitions
 * @param digits the digit recognitions
 * @param digType the digit model type (default Digital100)
 * @param checkConsistency sets property checkConsistency (default = false)
 * @param extendedResolution sets property extendedResolution (default = false)
 * @param decimal_shift set property decimal_shift (Nachkommastellen, default = 0)
 * @return UnderTestPost* the created testobject
 */
UnderTestPost* init_do_flow(std::vector<float> analog, std::vector<float> digits, t_CNNType digType = Digital100, 
                bool checkConsistency=false,  bool extendedResolution=false, int decimal_shift=0);

/**
 * @brief creates a testobject an run do flow (including setup). AnalogType is Class100, because all analog types do the same.
 * 
 * @param analog the analog recognitions
 * @param digits the digit recognitions
 * @param digType the digit model type (default Digital100)
 * @param checkConsistency sets property checkConsistency (default = false)
 * @param extendedResolution sets property extendedResolution (default = false)
 * @param decimal_shift set property decimal_shift (Nachkommastellen, default = 0)
 * @return std::string the return value of do_Flow is the Value as string
 */
std::string process_doFlow(std::vector<float> analog, std::vector<float> digits, t_CNNType digType = Digital100, 
                bool checkConsistency=false,  bool extendedResolution=false, int decimal_shift=0);

/**
 * @brief run do_Flow on the testobject
 * 
 * @param _underTestPost the testobject
 * @return std::string the return value of do_Flow is the Value as string
 */
std::string process_doFlow(UnderTestPost* _underTestPost);


/**
 * @brief Set the Consitency Check on testobject
 * 
 * @param _UnderTestPost the testobject
 * @param _checkConsistency true/false if checkConsistency
 */
void setConsitencyCheck(UnderTestPost* _UnderTestPost, bool _checkConsistency);

/**
 * @brief Set the Pre Value on testobject
 * 
 * @param _UnderTestPost the testobject
 * @param _preValue the previous value
 */
void setPreValue(UnderTestPost* _UnderTestPost, double _preValue);

/**
 * @brief Set the Extended Resolution on undertest
 * 
 * @param _UnderTestPost the testobject
 * @param _extendedResolution true/false 
 */
void setExtendedResolution(UnderTestPost* _UnderTestPost, bool _extendedResolution);

/**
 * @brief Set the Decimal Shift (Nachkomma)
 * 
 * @param _UnderTestPost the testobject  
 * @param decimal_shift  count of nachkomma
 */
void setDecimalShift(UnderTestPost* _UnderTestPost, int decimal_shift);

/**
 * @brief Set the Analogdigit Transistion Start 
 * 
 * @param _underTestPost the testobject  
 * @param _analogdigitTransistionStart the analog to digit transition start
 */
void setAnalogdigitTransistionStart(UnderTestPost* _underTestPost, float _analogdigitTransistionStart);

/**
 * @brief Set the allowNegatives in testobject 
 * 
 * @param _underTestPost the testobject  
 * @param _allowNegatives if should be set true or false
 */
void setAllowNegatives(UnderTestPost* _underTestPost, bool _allowNegatives);

#endif // TEST_FLOW_H
