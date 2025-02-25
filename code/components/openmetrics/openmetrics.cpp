#include "openmetrics.h"
#include "functional"
#include "esp_log.h"

/**
 * create a singe metric from the given input
 **/
std::string createMetric(const std::string &metricName, const std::string &help, const std::string &type, const std::string &value)
{
    return "# HELP " + metricName + " " + help + "\n" +
           "# TYPE " + metricName + " " + type + "\n" +
           metricName + " " + value + "\n";
}

typedef struct sequence_metric {
    const char *name;
    const char *help;
    const char *type;
    std::function<std::string(NumberPost *number)> valueFunc;
} sequence_metric_t;


sequence_metric_t sequenceMetrics[4] = {
    { "flow_value",     "current value of meter readout",     "gauge", [](NumberPost *number)-> std::string {return number->ReturnValue;} },
    { "flow_raw_value", "current raw value of meter readout", "gauge", [](NumberPost *number)-> std::string {return number->ReturnRawValue;} },
    { "flow_pre_value", "previous value of meter readout",    "gauge", [](NumberPost *number)-> std::string {return number->ReturnPreValue;} },
    { "flow_error",     "Error message text != 'no error'",   "gauge", [](NumberPost *number)-> std::string {return number->ErrorMessageText.compare("no error") == 0 ? "0" : "1";} },
};

std::string createSequenceMetrics(std::string prefix, const std::vector<NumberPost *> &numbers)
{
    std::string result;
    for (int i = 0; i<sizeof(sequenceMetrics)/sizeof(sequence_metric_t);i++) 
    {
        std::string res;
        for (const auto &number : numbers)
        {
            std::string value = sequenceMetrics[i].valueFunc(number); 
            if (value.find("N") != std::string::npos) {
                value = "NaN";
            }
            ESP_LOGD("METRICS", "metric=%s, name=%s, value = %s ",sequenceMetrics[i].name,number->name.c_str(), value.c_str());

            // only valid data is reported (https://github.com/OpenObservability/OpenMetrics/blob/main/specification/OpenMetrics.md#missing-data)
            if (value.length() > 0)
            {
                auto label = number->name;
                // except newline, double quote, and backslash (https://github.com/OpenObservability/OpenMetrics/blob/main/specification/OpenMetrics.md#abnf)
                // to keep it simple, these characters are just removed from the label
                replaceAll(label, "\\", "");
                replaceAll(label, "\"", "");
                replaceAll(label, "\n", "");

                res += prefix + "_" + sequenceMetrics[i].name + "{sequence=\"" + label + "\"} " + value + "\n";
            }
        }
        // prepend metadata if a valid metric was created
        if (res.length() > 0)
        {
            res = "# HELP " + prefix + "_" + sequenceMetrics[i].name + " " + sequenceMetrics[i].help + "\n"
                + "# TYPE " + prefix + "_" + sequenceMetrics[i].name + " " + sequenceMetrics[i].type + "\n"
                + res;
        }
        result += res;
    }

    return result;
}

/**
 * Generate the MetricFamily from all available sequences
 * @returns the string containing the text wire format of the MetricFamily
 **/
/*
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
*/