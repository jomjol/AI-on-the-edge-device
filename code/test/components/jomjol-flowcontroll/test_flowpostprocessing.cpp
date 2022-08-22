#include <unity.h>
#include <ClassFlowPostProcessing.h>
#include <ClassFlowCNNGeneral.h>
#include <ClassFlowCNNGeneral.h>
#include <ClassFlowMakeImage.h>


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


/**
 * @brief getestet wird das Ergebnis von
 * digit1 = 1.2
 * digit2 = 6.7
 * analog1 = 9.5
 * analog2 = 8.4
 * 
 * Das Ergebnis sollte "16.984" sein.
 */
void test_doFlow() {
    // setup the classundertest
    setUpClassFlowPostprocessing();

    printf("SetupClassFlowPostprocessing completed.\n");

    general* gen_digit = _digit->GetGENERAL("default", true);
    // digit1
    roi* digit1 = new roi();
    digit1->name = "digit1";
    digit1->result_float = 1.2;
    gen_digit->ROI.push_back(digit1);
    // digit2
    roi* digit2 = new roi();
    digit2->name = "digit2";
    digit2->result_float = 6.7;
    gen_digit->ROI.push_back(digit2);
    
    general* gen_analog = _analog->GetGENERAL("default", true);
    // analog1
    roi* analog1 = new roi();
    analog1->name = "analog1";
    analog1->result_float = 9.5;
    gen_analog->ROI.push_back(analog1);
    
    // analog2
    roi* analog2 = new roi();
    analog2->name = "analog2";
    analog2->result_float = 8.4;
    gen_analog->ROI.push_back(analog2);
 
    printf("Setup ROIs completed.\n");

    undertestPost->InitNUMBERS();


    string time;
    // run test
    TEST_ASSERT_TRUE(undertestPost->doFlow(time));

    printf("DoFlow completed.\n");


    TEST_ASSERT_EQUAL_STRING("0000", undertestPost->getReadout(0).c_str());

}



