#include <unity.h>
#include <ClassFlowCNNGeneral.h>

class UnderTest : public ClassFlowCNNGeneral {
    public:
    using ClassFlowCNNGeneral::ZeigerEval;
    using ClassFlowCNNGeneral::ZeigerEvalHybrid;
    using ClassFlowCNNGeneral::ClassFlowCNNGeneral;
    
};


void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}



/**
 * @brief test if all combinations of digit 
 * evaluation are running correctly
 */
void test_ZeigerEval() 
{
    UnderTest undertest = UnderTest(nullptr, Digital100);

    // the 5.2 is already above 5.0 and the previous digit too (3)
    int result = undertest.ZeigerEval(5.2, 3);
    TEST_ASSERT_EQUAL(5, result);

    // the 5.2 is already above 5.0 and the previous digit not (9)
    // so the current digit shoult be reduced (4.9)
    TEST_ASSERT_EQUAL(4, undertest.ZeigerEval(5.2, 9));

    // the 4.4 (digital100) is not above 5  and the previous digit (analog) too (9.3)
    TEST_ASSERT_EQUAL(4, undertest.ZeigerEval(4.4, 9));

}

/**
 * @brief test if all combinations of digit 
 * evaluation are running correctly
 */
void test_ZeigerEvalHybrid() {
    UnderTest undertest = UnderTest(nullptr, Digital100);

    // the 5.2 and no previous should round down
    TEST_ASSERT_EQUAL(5, undertest.ZeigerEvalHybrid(5.2, 0, -1));

    // the 5.3 and no previous should trunc to 5
    TEST_ASSERT_EQUAL(5, undertest.ZeigerEvalHybrid(5.3, 0, -1));

    // the 5.7 and no previous should trunc to 5
    TEST_ASSERT_EQUAL(5, undertest.ZeigerEvalHybrid(5.7, 0, -1));

    // the 5.8 and no previous should round up to 6
    TEST_ASSERT_EQUAL(6, undertest.ZeigerEvalHybrid(5.8, 0, -1));

    // the 5.7 with previous and the previous between 0.3-0.7 should round up to 6
    TEST_ASSERT_EQUAL(6, undertest.ZeigerEvalHybrid(5.7, 0.7, 1));

    // the 5.3 with previous and the previous between 0.3-0.7 should round down to 5
    TEST_ASSERT_EQUAL(5, undertest.ZeigerEvalHybrid(5.3, 0.7, 1));

    // the 5.3 with previous and the previous <=0.5 should trunc to 5
    TEST_ASSERT_EQUAL(5, undertest.ZeigerEvalHybrid(5.3, 0.1, 1));

    // the 5.3 with previous and the previous >=9.5 should reduce to 4
    TEST_ASSERT_EQUAL(4, undertest.ZeigerEvalHybrid(5.3, 9.6, 9));

    // the 5.7 with previous and the previous >=9.5 should trunc to 5
    TEST_ASSERT_EQUAL(5, undertest.ZeigerEvalHybrid(5.7, 9.6, 9));

}

