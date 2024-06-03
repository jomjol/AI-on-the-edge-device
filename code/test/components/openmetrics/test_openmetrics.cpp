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
    NUMBERS.push_back(number_1);

    const std::string metricNamePrefix = "ai_on_the_edge_device";
    const std::string metricName = metricNamePrefix + "_flow_value";

    std::string expected1 = "# HELP " + metricName + " current value of meter readout\n# TYPE " + metricName + " gauge\n" +
                             metricName + "{sequence=\"" + number_1->name + "\"} " + number_1->ReturnValue + "\n";
    TEST_ASSERT_EQUAL_STRING(expected1.c_str(), createSequenceMetrics(metricNamePrefix, NUMBERS).c_str());

    NumberPost *number_2 = new NumberPost;
    number_2->name = "secondary";
    number_2->ReturnValue = "1.0";
    NUMBERS.push_back(number_2);

    std::string expected2 = "# HELP " + metricName + " current value of meter readout\n# TYPE " + metricName + " gauge\n" +
                             metricName + "{sequence=\"" + number_1->name + "\"} " + number_1->ReturnValue + "\n" +
                             metricName + "{sequence=\"" + number_2->name + "\"} " + number_2->ReturnValue + "\n";
    TEST_ASSERT_EQUAL_STRING(expected2.c_str(), createSequenceMetrics(metricNamePrefix, NUMBERS).c_str());
}

void test_openmetrics()
{
    test_createMetric();
    test_replaceString();
    test_createSequenceMetrics();
}
