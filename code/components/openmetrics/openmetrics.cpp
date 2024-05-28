#include "openmetrics.h"

using namespace std;

/**
 * create a singe metric from the given input
 **/
string createMetric(const string &metricName, const string &help, const string &type, const string &value)
{
    return "# HELP " + metricName + " " + help + "\n# TYPE " + metricName + " " + type + "\n" + metricName + " " + value + "\n";
}

/**
 * Generate the MetricFamily from all available sequences
 * @returns the string containing the text wire format of the MetricFamily
 **/
string createSequenceMetrics(string prefix, std::vector<NumberPost *> *numbers)
{
    string res = "";

    for (int i = 0; i < (*numbers).size(); ++i)
    {
        auto number = (*numbers)[i];
        // only valid data is reported (https://github.com/OpenObservability/OpenMetrics/blob/main/specification/OpenMetrics.md#missing-data)
        if (number->ReturnValue.length() > 0)
        {
            auto label = number->name;

            // except newline, double quote, and backslash (https://github.com/OpenObservability/OpenMetrics/blob/main/specification/OpenMetrics.md#abnf)
            // to keep it simple, these characters are just remove from the label
            label = ReplaceString(label, "\\", "");
            label = ReplaceString(label, "\"", "");
            label = ReplaceString(label, "\n", "");
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