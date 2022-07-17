#include <unity.h>
#include "components/jomjol-flowcontroll/test_cnnflowcontroll.cpp"

/**
 * @brief startup the test. Like a test-suite 
 * all test methods must be called here
 */
extern "C" void app_main()
{
  UNITY_BEGIN();

  RUN_TEST(test_ZeigerEval);
  RUN_TEST(test_ZeigerEvalHybrid);
  
  UNITY_END();
}