#pragma once

#ifndef TIMESNTP_H
#define TIMESNTP_H


#include <string>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_sntp.h"

std::string getCurrentTimeString(const char * frm);
std::string ConvertTimeToString(time_t _time, const char * frm);


bool getTimeIsSet(void);
bool getTimeWasNotSetAtBoot(void);

bool getUseNtp(void);
bool setupTime();

bool time_manual_reset_sync(void);

extern int LocalTimeToUTCOffsetSeconds;


#endif //TIMESNTP_H