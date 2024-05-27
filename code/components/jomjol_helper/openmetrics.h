#pragma once

#ifndef OPENMETRICS_H
#define OPENMETRICS_H

#include <string>
#include <fstream>
#include <vector>

#include "../jomjol_flowcontroll/ClassFlowDefineTypes.h"

using namespace std;

string createMetric(const string &metricName, const string &help, const string &type, const string &value);
string createSequenceMetrics(string prefix, std::vector<NumberPost*> *numbers);

#endif //OPENMETRICS_H