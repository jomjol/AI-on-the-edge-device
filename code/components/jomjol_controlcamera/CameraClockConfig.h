#pragma once

#include <algorithm>
#include <cctype>
#include <istream>
#include <string>

// Read before camera initialization; a commented section or value keeps the build default.
inline int readCameraClockMHz(std::istream &config, int defaultMHz)
{
    const auto trimClockText = [](std::string text) {
        const auto first = text.find_first_not_of(" \t\r\n");
        return first == std::string::npos ? std::string() :
            text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
    };
    const auto upperClockText = [](std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return text;
    };
    bool takeImage = false;
    int mhz = defaultMHz;
    std::string line;
    while (std::getline(config, line))
    {
        line = trimClockText(line);
        if (line.empty()) continue;
        // A disabled section ends the preceding active section too.
        if (line.front() == '[' || line.compare(0, 2, ";[") == 0)
        {
            takeImage = upperClockText(line) == "[TAKEIMAGE]";
            continue;
        }
        if (!takeImage || line.front() == ';' || line.front() == '#') continue;
        const auto equals = line.find('=');
        if (equals == std::string::npos ||
            upperClockText(trimClockText(line.substr(0, equals))) != "CAMXCLKFREQMHZ") continue;
        const auto value = trimClockText(line.substr(equals + 1));
        if (value == "8" || value == "10" || value == "16" || value == "20")
            mhz = std::stoi(value);
    }
    return mhz;
}
