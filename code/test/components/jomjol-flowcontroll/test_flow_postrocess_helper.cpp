#include "test_flow_postrocess_helper.h"
#include "esp_log.h"

static const char *TAG = "POSTPROC TEST";

UnderTestPost* setUpClassFlowPostprocessing(t_CNNType digType, t_CNNType anaType)
{
    
    ClassFlowCNNGeneral* _analog;
    ClassFlowCNNGeneral* _digit;
    std::vector<ClassFlow*> FlowControll;
    ClassFlowTakeImage* flowtakeimage;

    // wird im doFlow verwendet
    flowtakeimage = new ClassFlowTakeImage(&FlowControll);
    FlowControll.push_back(flowtakeimage);

    // Die Modeltypen werden gesetzt, da keine Modelle verwendet werden.
    _analog = new ClassFlowCNNGeneral(nullptr, anaType);
    
    _digit =  new ClassFlowCNNGeneral(nullptr, digType);

    return new UnderTestPost(&FlowControll, _analog, _digit);
  
}

std::string process_doFlow(UnderTestPost* _underTestPost) {
        string time;
 
    // run test
    TEST_ASSERT_TRUE(_underTestPost->doFlow(time));

    return _underTestPost->getReadout(0);
}

/**
 * @brief setup flow like it runs after recognition.
 * 
 * @param analog the analog recognitions as array begins with the highest ROI
 * @param digits the digital regocnitions as array begins with the highest ROI
 * @param digType type of the model defaults do Digital100
 * @param checkConsistency for Digital type only. Not relvant for newer models
 * @param extendedResolution the lowest ROI will directly used (9.7 => 9.7) if false 9.7 => 9
 * @param decimal_shift the decimal point offset. -3 corresponds to x.yyy
 * @return std::string the value result
 */
std::string process_doFlow(std::vector<float> analog, std::vector<float> digits, t_CNNType digType, 
            bool checkConsistency, bool extendedResolution, int decimal_shift) {
    // setup the classundertest
    UnderTestPost* _undertestPost = init_do_flow(analog, digits, digType, checkConsistency, extendedResolution, decimal_shift);
    ESP_LOGD(TAG, "SetupClassFlowPostprocessing completed.");

    string time;
    // run test
    TEST_ASSERT_TRUE(_undertestPost->doFlow(time));

    std::string result =  _undertestPost->getReadout(0);
    delete _undertestPost;
    return result;

}

UnderTestPost* init_do_flow(std::vector<float> analog, std::vector<float> digits, t_CNNType digType, 
                bool checkConsistency,  bool extendedResolution, int decimal_shift) {

    UnderTestPost* _undertestPost = setUpClassFlowPostprocessing(digType, Analogue100);


    // digits
    if (digits.size()>0) {
        general* gen_digit = _undertestPost->flowDigit->GetGENERAL("default", true);
        gen_digit->ROI.clear();
        for (int i = 0; i<digits.size(); i++) {
            roi* digitROI = new roi();
            string name = "digit_" + std::to_string(i);
            digitROI->name = name;
            digitROI->result_klasse = (int) digits[i];
            digitROI->result_float = digits[i];
            gen_digit->ROI.push_back(digitROI);
        }
    }

    // analog
    if (analog.size()>0) {
        general* gen_analog = _undertestPost->flowAnalog->GetGENERAL("default", true);
        gen_analog->ROI.clear();

        for (int i = 0; i<analog.size(); i++) {
            roi* anaROI = new roi();
            string name = "ana_1" + std::to_string(i);
            anaROI->name = name;
            anaROI->result_float = analog[i];
            gen_analog->ROI.push_back(anaROI);
        }
    } else {
        _undertestPost->flowAnalog = NULL;
    }
    ESP_LOGD(TAG, "Setting up of ROIs completed.");

    _undertestPost->InitNUMBERS();
   
    setConsitencyCheck(_undertestPost, checkConsistency);
    setExtendedResolution(_undertestPost, extendedResolution);
    setDecimalShift(_undertestPost, decimal_shift);

    return _undertestPost;

}

void setPreValue(UnderTestPost* _underTestPost, double _preValue) {
        if (_preValue>0) {
        ESP_LOGD(TAG, "preValue=%f", _preValue);
        std::vector<NumberPost*>* NUMBERS = _underTestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            (*NUMBERS)[_n]->PreValue = _preValue;
        }
    }
}

void setAllowNegatives(UnderTestPost* _underTestPost, bool _allowNegatives) {
        ESP_LOGD(TAG, "checkConsistency=true");
        std::vector<NumberPost*>* NUMBERS = _underTestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            (*NUMBERS)[_n]->AllowNegativeRates = _allowNegatives;
        }
    
}

void setConsitencyCheck(UnderTestPost* _underTestPost, bool _checkConsistency) {
        if (_checkConsistency) {
        ESP_LOGD(TAG, "checkConsistency=true");
        std::vector<NumberPost*>* NUMBERS = _underTestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            (*NUMBERS)[_n]->checkDigitIncreaseConsistency = true;
        }
    }
}


void setExtendedResolution(UnderTestPost* _underTestPost, bool _extendedResolution) {
    if (_extendedResolution ) {
       std::vector<NumberPost*>* NUMBERS = _underTestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            (*NUMBERS)[_n]->isExtendedResolution = true;
        }
    }
}

void setDecimalShift(UnderTestPost* _underTestPost, int _decimal_shift) {
    if (_decimal_shift!=0) {
        std::vector<NumberPost*>* NUMBERS = _underTestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            ESP_LOGD(TAG, "Setting decimal shift on number: %d to %d", _n, _decimal_shift);
            (*NUMBERS)[_n]->DecimalShift = _decimal_shift;
            (*NUMBERS)[_n]->DecimalShiftInitial = _decimal_shift;   
        }       
    }
}

void setAnalogdigitTransistionStart(UnderTestPost* _underTestPost, float _analogdigitTransistionStart) {
    if (_analogdigitTransistionStart!=0) {
        std::vector<NumberPost*>* NUMBERS = _underTestPost->GetNumbers();    
        for (int _n = 0; _n < (*NUMBERS).size(); ++_n) {
            ESP_LOGD(TAG, "Setting decimal shift on number: %d to %f", _n, _analogdigitTransistionStart);
            (*NUMBERS)[_n]->AnalogDigitalTransitionStart = _analogdigitTransistionStart; 
        }       
    }
}
