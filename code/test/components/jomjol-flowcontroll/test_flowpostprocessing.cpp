#include <unity.h>
#include <ClassFlowPostProcessing.h>
#include <ClassFlowCNNGeneral.h>
#include <ClassFlowCNNGeneral.h>
#include <ClassFlowMakeImage.h>

void setUpClassFlowPostprocessing(void);
string process_doFlow(std::vector<float> analog, std::vector<float> digits);

ClassFlowCNNGeneral* _analog;
ClassFlowCNNGeneral* _digit;
std::vector<ClassFlow*> FlowControll;
ClassFlowMakeImage* flowmakeimage;


class UnderTestPost : public ClassFlowPostProcessing {
    public:
        UnderTestPost(std::vector<ClassFlow*>* lfc, ClassFlowCNNGeneral *_analog, ClassFlowCNNGeneral *_digit)
            : ClassFlowPostProcessing::ClassFlowPostProcessing(lfc, _analog, _digit) {}
        using ClassFlowPostProcessing::InitNUMBERS;
};

UnderTestPost* undertestPost;


/**
 * @brief Testet die doFlow-Methode von ClassFlowPostprocessing
 * digits[] - enthält die liste der vom Model zurückgegebenen Ergebnisse (class100/cont) in der Reihenfolge von links nach rechts
 * analog[] - enthält die Liste der Zeiger vom Model, wie bei den digits
 * expected - enthält das erwartete Ergebnis, wobei der Dezimalpunkt genau zwischen digits und analog ist.
 * 
 */
void test_doFlow() {
        /*
         * 
         * digit1 = 1.2
         * digit2 = 6.7
         * analog1 = 9.5
         * analog2 = 8.4
         * 
         * Das Ergebnis sollte "16.984" sein. Bzw. 16.98 ohne Extended true
         */
        std::vector<float> digits = { 1.2, 6.7};
        std::vector<float> analogs = { 9.5, 8.4};
        const char* expected = "16.98";
        std::string result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921
         * 
         * Das Ergebnis sollte "376529.6" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 3.0, 7.0, 6.0, 5.0, 2.5, 9.6};
        analogs = { 6.4};
        expected = "376529.6";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921
         * 
         * Das Ergebnis sollte "167734.6" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 1.1, 6.0, 7.0, 7.0, 3.0, 4.6};
        analogs = { 6.2};
        expected = "167734.6";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/919
         * 
         * Das Ergebnis sollte "58.96889" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 5.0, 8.6};
        analogs = { 9.8, 6.7, 8.9, 8.6, 9.8};
        expected = "58.96889";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
  
        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921
         * 
         * Das Ergebnis sollte "376529.6" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 2.9, 7.0, 6.8, 9.9, 8.0, 3.9};
        analogs = { 9.7};
        expected = "377083.9";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        digits = { 1.1, 9.0, 4.0};
        analogs = { 6.1, 2.6, 6.25, 9.7};
        expected = "194.6259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        digits = { 1.1, 9.0, 4.0};
        analogs = { 8.1, 2.6, 6.25, 9.7};
        expected = "194.8259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        digits = { 1.1, 9.0, 4.0};
        analogs = { 9.1, 2.6, 6.25, 9.7};
        expected = "193.9259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
}



void setUpClassFlowPostprocessing(void)
{
    
    // wird im doFlow verwendet
    flowmakeimage = new ClassFlowMakeImage(&FlowControll);
    FlowControll.push_back(flowmakeimage);

    // Die Modeltypen werden gesetzt, da keine Modelle verwendet werden.
    _analog = new ClassFlowCNNGeneral(nullptr, Analogue100);
    
    _digit =  new ClassFlowCNNGeneral(nullptr, Digital100);

    undertestPost = new UnderTestPost(&FlowControll, _analog, _digit);
  
}


std::string process_doFlow(std::vector<float> analog, std::vector<float> digits) {
    // setup the classundertest
    setUpClassFlowPostprocessing();

    printf("SetupClassFlowPostprocessing completed.\n");

    // digits
    if (digits.size()>0) {
        general* gen_digit = _digit->GetGENERAL("default", true);
        gen_digit->ROI.clear();

        for (int i = 0; i<digits.size(); i++) {
            roi* digitROI = new roi();
            string name = "digit_" + std::to_string(i);
            digitROI->name = name;
            digitROI->result_float = digits[i];
            gen_digit->ROI.push_back(digitROI);
        }
    }

    // analog
    if (analog.size()>0) {
        general* gen_analog = _analog->GetGENERAL("default", true);
        gen_analog->ROI.clear();

        for (int i = 0; i<analog.size(); i++) {
            roi* anaROI = new roi();
            string name = "ana_" + std::to_string(i);
            anaROI->name = name;
            anaROI->result_float = analog[i];
            gen_analog->ROI.push_back(anaROI);
        }
    }
    printf("Setup ROIs completed.\n");

    undertestPost->InitNUMBERS();

    string time;
    // run test
    TEST_ASSERT_TRUE(undertestPost->doFlow(time));

    return undertestPost->getReadout(0);

}

