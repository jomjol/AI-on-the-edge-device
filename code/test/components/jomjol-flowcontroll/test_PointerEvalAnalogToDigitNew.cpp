#include <unity.h>
#include <ClassFlowCNNGeneral.h>

class UnderTestCNNGeneral : public ClassFlowCNNGeneral {
    public:
        UnderTestCNNGeneral( ClassFlowAlignment *_flowalign, t_CNNType _cnntype) :
            ClassFlowCNNGeneral(_flowalign, _cnntype) {};
        
        using ClassFlowCNNGeneral::PointerEvalAnalogToDigitNew;
       

};


/**
 * @brief 
 * 
 * Transition = x.8 - x.2 here no transition in the test cases.
 * Offset = dig=x.n, ana= n.y: no offset, because both "n" are the same
 */
void test_analogToDigit_Standard() {

    UnderTestCNNGeneral* undertest = new UnderTestCNNGeneral(nullptr, Digital100);

    // 4.8 is a "hanging" 5, i.e. it has not jumped over to 5.0.
    // A "hanging digit" should still be rounded from Transition.
    // Transition = yes
    // Offset = no
    TEST_ASSERT_EQUAL_INT(5,  undertest->PointerEvalAnalogToDigitNew(4.8, 8.0, 8, 9.2));

    // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issue-1344032217
    // Default: dig=9.6, ana=6.8 => erg=9
    // Transition = no
    // Offset = no
    TEST_ASSERT_EQUAL_INT(9,  undertest->PointerEvalAnalogToDigitNew( 9.6, 6.8, 6, 9.2));


    // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1220365920
    // Default: dig=4.6, ana=6.2 => erg=4
    // Transition = no
    // Offset = no
    TEST_ASSERT_EQUAL_INT(4,  undertest->PointerEvalAnalogToDigitNew( 4.6, 6.2, 6, 9.2));

    // https://github.com/jomjol/AI-on-the-edge-device/issues/1143#issuecomment-1274434805
    // Hanging digit ()
    // Default: dig=6.8, ana=8.6 => erg=7
    // Transition = no
    // Offset = no
    TEST_ASSERT_EQUAL_INT(7,  undertest->PointerEvalAnalogToDigitNew( 6.8, 8.6, 6, 9.2));

    // https://github.com/jomjol/AI-on-the-edge-device/issues/1143#issuecomment-1274434805
    // Also hanging digit () with small pointer after 0 pass.
    // Default: dig=6.8, ana=1.0 => erg=7
    // Transition = no
    // Offset = no
    TEST_ASSERT_EQUAL_INT(7,  undertest->PointerEvalAnalogToDigitNew( 6.8, 1.0, 1, 9.2));


}

void test_analogToDigit_Transition() {
    UnderTestCNNGeneral* undertest = new UnderTestCNNGeneral(nullptr, Digital100);
    
    // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1222672175
    // Default: dig=3.9, ana=9.7 => erg=3
    // Transition = yes
    // Zero crossing = no
    // Offset = no
    TEST_ASSERT_EQUAL_INT(3,  undertest->PointerEvalAnalogToDigitNew( 3.9, 9.7, 9, 9.2));
  
    // without reference
    // Default: dig=4.0, ana=9.1 => erg=4
    // Transition = yes
    // Zero crossing = no
    // Offset = no
    // Special feature: Digit has not yet started at analogue 9.1
    TEST_ASSERT_EQUAL_INT(4,  undertest->PointerEvalAnalogToDigitNew( 4.0, 9.1, 9, 9.2));

    // without reference
    // Default: dig=9.8, ana=0.1, ana_2=9.9 => erg=9
    // transition = yes
    // Zero crossing = no
    // Offset = no
    // Special feature: analogue is set back to 9 by previous analogue
    TEST_ASSERT_EQUAL_INT(9,  undertest->PointerEvalAnalogToDigitNew( 9.8, 0.1, 9, 9.2));


    // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1277425333
    // Default: dig=5.9, ana=9.4 => erg=9
    // Transition = yes
    // Zero crossing = no
    // Offset = no
    // Special feature: 
    TEST_ASSERT_EQUAL_INT(5,  undertest->PointerEvalAnalogToDigitNew( 5.9, 9.4, 9, 9.2));

    // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1282168030
    // Default: dig=1.8, ana=7.8 => erg=9
    // Transition = yes
    // Zero crossing = no
    // Offset = no
    // Special feature: Digit runs with analogue. Therefore 1.8 (vs. 7.8)
    TEST_ASSERT_EQUAL_INT(1,  undertest->PointerEvalAnalogToDigitNew( 1.8, 7.8, 7, 7.7));

}
