#include "../components/jomjol_controlcamera/CameraClockConfig.h"
#include <cassert>
#include <sstream>
#include <iostream>

static int clockFrom(const std::string &text, int fallback = 20)
{
    std::istringstream config(text);
    return readCameraClockMHz(config, fallback);
}

int main()
{
    assert(clockFrom("") == 20);
    assert(clockFrom("", 10) == 10);
    for (int mhz : {8, 10, 16, 20})
        assert(clockFrom("[TakeImage]\nCamXclkFreqMhz = " + std::to_string(mhz)) == mhz);
    assert(clockFrom("[takeimage]\r\n  camxclkfreqmhz = 8  \r\n") == 8);
    assert(clockFrom(";[TakeImage]\nCamXclkFreqMhz = 8\n") == 20);
    assert(clockFrom("[TakeImage]\n;CamXclkFreqMhz = 8\n") == 20);
    assert(clockFrom("[System]\nCamXclkFreqMhz = 8\n") == 20);
    assert(clockFrom("[TakeImage]\n;[System]\nCamXclkFreqMhz = 8\n") == 20);
    for (const auto &value : {"", "0", "2", "-8", "8.0", "8bad", "999999999999999999"})
        assert(clockFrom(std::string("[TakeImage]\nCamXclkFreqMhz = ") + value) == 20);
    assert(clockFrom("[TakeImage]\nCamXclkFreqMhz = 8\nCamXclkFreqMhz = 16\n") == 16);
    assert(clockFrom("[TakeImage]\nCamXclkFreqMhz = 8\nCamXclkFreqMhz = bad\n") == 8);
    assert(clockFrom("[TakeImage]\nCamXclkFreqMhz = bad\n", 10) == 10);
    assert(clockFrom("[TakeImage]\n#CamXclkFreqMhz = 8\n") == 20);
    assert(clockFrom("[TakeImage]\nCamXclkFreqMhz = 8\n[System]\nCamXclkFreqMhz = 16\n") == 8);
    assert(clockFrom("[TakeImage]\nCamXclkFreqMhz = 8\n;[System]\nCamXclkFreqMhz = 16\n") == 8);
    assert(clockFrom("CamXclkFreqMhz = 8\n") == 20);
    std::istringstream unavailable;
    unavailable.setstate(std::ios::failbit);
    assert(readCameraClockMHz(unavailable, 16) == 16);
    std::cout << "26 camera clock boot checks passed\n";
}
