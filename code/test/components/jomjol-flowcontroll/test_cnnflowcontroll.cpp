#include <unity.h>
#include <ClassFlowCNNGeneral.h>

class UnderTestCNN : public ClassFlowCNNGeneral {
    public:
    using ClassFlowCNNGeneral::PointerEvalAnalogNew;
    using ClassFlowCNNGeneral::PointerEvalHybridNew;
    using ClassFlowCNNGeneral::ClassFlowCNNGeneral;
    
};


/**
 * @brief test if all combinations of digit 
 * evaluation are running correctly
 */
void test_ZeigerEval() 
{
    UnderTestCNN undertest = UnderTestCNN(nullptr, Digital100);

    // the 5.2 is already above 5.0 and the previous digit too (3)
    printf("Test 5.2, 3\n");
    int result = undertest.PointerEvalAnalogNew(5.2, 3);
    TEST_ASSERT_EQUAL(5, result);

    // the 5.2 is already above 5.0 and the previous digit not (9)
    // so the current digit shoult be reduced (4.9)
    printf("Test 5.2, 9\n");
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalAnalogNew(5.2, 9));

    printf("Test 4.4, 9\n");
    // the 4.4 (digital100) is not above 5  and the previous digit (analog) too (9.3)
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalAnalogNew(4.4, 9));

    printf("Test 4.5, 0\n");
    // the 4.5 (digital100) is not above 5  and the previous digit (analog) too (9.6)
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalAnalogNew(4.5, 0));    

}

/**
 * @brief test if all combinations of digit 
 * evaluation are running correctly
 * 
 * Desciption on call undertest.PointerEvalHybridNew(float number, float number_of_predecessors, int eval_predecessors, bool Analog_Predecessors, float digitalAnalogTransitionStart)
 * @param number: is the current ROI as float value from recognition
 * @param number_of_predecessors: is the last (lower) ROI as float from recognition
 * @param eval_predecessors: is the evaluated number. Sometimes a much lower value can change higer values
 *                          example: 9.8, 9.9, 0.1
 *                          0.1 => 0 (eval_predecessors)
 *                          The 0 makes a 9.9 to 0 (eval_predecessors)
 *                          The 0 makes a 9.8 to 0 
 * @param Analog_Predecessors false/true if the last ROI is an analog or digital ROI (default=false)
 *                              runs in special handling because analog is much less precise
 * @param digitalAnalogTransitionStart start of the transitionlogic begins on number_of_predecessor (default=9.2)
 *
 * 
 * 
 * 
 */
void test_ZeigerEvalHybrid() {
    UnderTestCNN undertest = UnderTestCNN(nullptr, Digital100);

    // the 5.2 and no previous should round down
    printf("PointerEvalHybridNew(5.2, 0, -1)\n");
    TEST_ASSERT_EQUAL(5, undertest.PointerEvalHybridNew(5.2, 0, -1));

    // the 5.3 and no previous should trunc to 5
    printf("PointerEvalHybridNew(5.3, 0, -1)\n");
    TEST_ASSERT_EQUAL(5, undertest.PointerEvalHybridNew(5.3, 0, -1));

    printf("PointerEvalHybridNew(5.7, 0, -1)\n");
    // the 5.7 and no previous should trunc to 5
    TEST_ASSERT_EQUAL(5, undertest.PointerEvalHybridNew(5.7, 0, -1, false, 9.2));

    // the 5.8 and no previous should round up to 6
    printf("PointerEvalHybridNew(5.8, 0, -1)\n");
    TEST_ASSERT_EQUAL(6, undertest.PointerEvalHybridNew(5.8, 8.0, 8, false, 8.0));

    // the 5.7 with previous and the previous between 0.3-0.5 should round up to 6
    TEST_ASSERT_EQUAL(6, undertest.PointerEvalHybridNew(5.7, 0.4, 1));

    // the 5.3 with previous and the previous between 0.3-0.7 should round down to 5
    TEST_ASSERT_EQUAL(5, undertest.PointerEvalHybridNew(5.3, 0.7, 1));

    // the 5.3 with previous and the previous <=0.5 should trunc to 5
    TEST_ASSERT_EQUAL(5, undertest.PointerEvalHybridNew(5.3, 0.1, 1));

    // the 5.2 with previous and the previous >=9.8 should reduce to 4
    // the digit is already over transistion, but a analog pointer runs behind
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalHybridNew(5.2, 9.8, 9, false, 9.0));

    // the 5.7 with previous and the previous >=9.5 should trunc to 5
    TEST_ASSERT_EQUAL(5, undertest.PointerEvalHybridNew(5.7, 9.6, 9));

    // the 4.5 (digital100) is not above 5  and the previous digit (analog) not over Zero (9.6)
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalHybridNew(4.5, 9.6, 0));    

    // the 4.5 (digital100) is not above 5  and the previous digit (analog) not over Zero (9.6)
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalHybridNew(4.5, 9.6, 9));    
    // the 4.5 (digital100) is not above 5  and the previous digit (analog) not over Zero (9.5)
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalHybridNew(4.5, 9.5, 9));    

    // 59.96889 - Pre: 58.94888
    // 8.6 : 9.8 : 6.7
    // the 4.4 (digital100) is not above 5  and the previous digit (analog) not over Zero (9.5)
    TEST_ASSERT_EQUAL(8, undertest.PointerEvalHybridNew(8.6, 9.8, 9));    

    // pre = 9.9 (0.0 raw)
    // zahl = 1.8
    TEST_ASSERT_EQUAL(2, undertest.PointerEvalHybridNew(1.8, 9.0, 9));    
 
    // if a digit have an early transition and the pointer is < 9.0 
    // prev (pointer) = 6.2, but on digital readout = 6.0 (prev is int parameter)
    // zahl = 4.6
    TEST_ASSERT_EQUAL(4, undertest.PointerEvalHybridNew(4.6, 6.0, 6));    
 
    
    // issue #879 vorgaenger is -1, zahl = 6.7
    //TEST_ASSERT_EQUAL(7, undertest.ZeigerEvalHybrid(6.7, -1.0, -1));    


}

