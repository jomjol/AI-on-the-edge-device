#pragma once

#ifndef OPENMETRICS_H
#define OPENMETRICS_H

#include <string>
#include <fstream>
#include <vector>

#include "ClassFlowDefineTypes.h"

std::string createMetric(const std::string &metricName, const std::string &help, const std::string &type, const std::string &value);
std::string createSequenceMetrics(std::string prefix, const std::vector<NumberPost *> &numbers);

#endif // OPENMETRICS_H
