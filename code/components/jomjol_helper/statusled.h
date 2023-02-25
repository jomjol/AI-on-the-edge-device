#pragma once

#ifndef STATUSLED_H
#define STATUSLED_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


extern TaskHandle_t xHandle_task_StatusLED;

enum StatusLedSource {
	WLAN_CONN = 1,
    WLAN_INIT = 2,
    SDCARD_INIT = 3,
	SDCARD_CHECK = 4,
    CAM_INIT = 5,
    PSRAM_INIT = 6,
    TIME_CHECK = 7,
    AP_OR_OTA = 8
};

struct StatusLEDData {
    int iSourceBlinkCnt = 1;
    int iCodeBlinkCnt = 1;
    int iBlinkTime = 250;
    bool bInfinite = false;
    bool bProcessingRequest = false;
};

void StatusLED(StatusLedSource _eSource, int _iCode, bool _bInfinite);
void StatusLEDOff(void);

#endif //STATUSLED_H