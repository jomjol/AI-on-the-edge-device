#include <unity.h>
#include <openmetrics.h>

void test_createMetric()
{
    // simple happy path
    const char *expected = "# HELP metric_name short description\n# TYPE metric_name gauge\nmetric_name 123.456\n";
    std::string result = createMetric("metric_name", "short description", "gauge", "123.456");
    TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
}

/**
 * test the replaceString function as it's a dependency to sanitize sequence names
 */
void test_replaceString()
{
    std::string sample = "hello\\world\\";
    replaceAll(sample, "\\", "");
    TEST_ASSERT_EQUAL_STRING("helloworld", sample.c_str());

    sample = "hello\"world\"";
    replaceAll(sample, "\"", "");
    TEST_ASSERT_EQUAL_STRING("helloworld", sample.c_str());

    sample = "hello\nworld\n";
    replaceAll(sample, "\n", "");
    TEST_ASSERT_EQUAL_STRING("helloworld", sample.c_str());

    sample = "\\\\\\\\\\\\\\\\\\hello\\world\\\\\\\\\\\\\\\\\\\\";
    replaceAll(sample, "\\", "");
    TEST_ASSERT_EQUAL_STRING("helloworld", sample.c_str());
}

void test_createSequenceMetrics()
{
    std::vector<NumberPost *> NUMBERS;
    NumberPost *number_1 = new NumberPost;
    number_1->name = "main";
    number_1->ReturnValue = "123.456";
    number_1->ReturnRawValue = "N23.456";
    number_1->ReturnPreValue = "986.543";
    number_1->ErrorMessageText = "";
    NUMBERS.push_back(number_1);

    const std::string metricNamePrefix = "ai_on_the_edge_device";
    const std::string metricName1 = metricNamePrefix + "_flow_value";
    const std::string metricName2 = metricNamePrefix + "_flow_raw_value";
    const std::string metricName3 = metricNamePrefix + "_flow_pre_value";
    const std::string metricName4 = metricNamePrefix + "_flow_error";

    std::string expected1 ;
    expected1 = "# HELP " + metricName1 + " current value of meter readout\n# TYPE " + metricName1 + " gauge\n" +
                metricName1 + "{sequence=\"" + number_1->name + "\"} " + number_1->ReturnValue + "\n";
    
    expected1 += "# HELP " + metricName2 + " current raw value of meter readout\n# TYPE " + metricName2 + " gauge\n" +
                metricName2 + "{sequence=\"" + number_1->name + "\"} " + "NaN" + "\n";

    expected1 += "# HELP " + metricName3 + " previous value of meter readout\n# TYPE " + metricName3 + " gauge\n" +
                metricName3 + "{sequence=\"" + number_1->name + "\"} " + number_1->ReturnPreValue + "\n";
    
    expected1 += "# HELP " + metricName4 + " Error message text != 'no error'\n# TYPE " + metricName4 + " gauge\n" +
                metricName4 + "{sequence=\"" + number_1->name + "\"} " + "1" + "\n";

    TEST_ASSERT_EQUAL_STRING(expected1.c_str(), createSequenceMetrics(metricNamePrefix, NUMBERS).c_str());

    NumberPost *number_2 = new NumberPost;
    number_2->name = "secondary";
    number_2->ReturnValue = "1.0";
    number_2->ReturnRawValue = "01.000";
    number_2->ReturnPreValue = "0.987";
    number_2->ErrorMessageText = "no error";
    NUMBERS.push_back(number_2);

    std::string expected2 ;
    expected2 = "# HELP " + metricName1 + " current value of meter readout\n# TYPE " + metricName1 + " gauge\n" +
                metricName1 + "{sequence=\"" + number_1->name + "\"} " + number_1->ReturnValue + "\n" +
                metricName1 + "{sequence=\"" + number_2->name + "\"} " + number_2->ReturnValue + "\n";
    
    expected2 += "# HELP " + metricName2 + " current raw value of meter readout\n# TYPE " + metricName2 + " gauge\n" +
                metricName2 + "{sequence=\"" + number_1->name + "\"} " + "NaN" + "\n" +
                metricName2 + "{sequence=\"" + number_2->name + "\"} " + number_2->ReturnRawValue + "\n";

    expected2 += "# HELP " + metricName3 + " previous value of meter readout\n# TYPE " + metricName3 + " gauge\n" +
                metricName3 + "{sequence=\"" + number_1->name + "\"} " + number_1->ReturnPreValue + "\n" +
                metricName3 + "{sequence=\"" + number_2->name + "\"} " + number_2->ReturnPreValue + "\n";

    expected2 += "# HELP " + metricName4 + " Error message text != 'no error'\n# TYPE " + metricName4 + " gauge\n" +
                metricName4 + "{sequence=\"" + number_1->name + "\"} " + "1" + "\n" +
                metricName4 + "{sequence=\"" + number_2->name + "\"} " + "0" + "\n";

    
    TEST_ASSERT_EQUAL_STRING(expected2.c_str(), createSequenceMetrics(metricNamePrefix, NUMBERS).c_str());
}

void test_openmetrics()
{
    test_createMetric();
    test_replaceString();
    test_createSequenceMetrics();
}
