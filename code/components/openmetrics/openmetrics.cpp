#include "openmetrics.h"

/**
 * create a singe metric from the given input
 **/
std::string createMetric(const std::string &metricName, const std::string &help, const std::string &type, const std::string &value)
{
    return "# HELP " + metricName + " " + help + "\n" +
           "# TYPE " + metricName + " " + type + "\n" +
           metricName + " " + value + "\n";
}

/**
 * Generate the MetricFamily from all available sequences
 * @returns the string containing the text wire format of the MetricFamily
 **/
std::string createSequenceMetrics(std::string prefix, const std::vector<NumberPost *> &numbers)
{
    std::string res;

    for (const auto &number : numbers)
    {
        // only valid data is reported (https://github.com/OpenObservability/OpenMetrics/blob/main/specification/OpenMetrics.md#missing-data)
        if (number->ReturnValue.length() > 0)
        {
            auto label = number->name;

            // except newline, double quote, and backslash (https://github.com/OpenObservability/OpenMetrics/blob/main/specification/OpenMetrics.md#abnf)
            // to keep it simple, these characters are just removed from the label
            replaceAll(label, "\\", "");
            replaceAll(label, "\"", "");
            replaceAll(label, "\n", "");
            res += prefix + "_flow_value{sequence=\"" + label + "\"} " + number->ReturnValue + "\n";
        }
    }

    // prepend metadata if a valid metric was created
    if (res.length() > 0)
    {
        res = "# HELP " + prefix + "_flow_value current value of meter readout\n# TYPE " + prefix + "_flow_value gauge\n" + res;
    }
    return res;
}
