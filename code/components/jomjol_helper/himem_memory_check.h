
// need [env:esp32cam-dev-himem]
//CONFIG_SPIRAM_BANKSWITCH_ENABLE=y
//CONFIG_SPIRAM_BANKSWITCH_RESERVE=4

#pragma once

#include "../../include/defines.h"

#ifdef DEBUG_HIMEM_MEMORY_CHECK

#ifndef HIMEM_MEMORY_CHECK_H
#define HIMEM_MEMORY_CHECK_H



//source : //source : https://github.com/espressif/esp-idf/blob/master/examples/system/himem/main/himem_example_main.c


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp32/himem.h"

#include <string>
#include "esp32/himem.h"


std::string himem_memory_check();

#endif //HIMEM_MEMORY_CHECK_H

#endif // DEBUG_HIMEM_MEMORY_CHECK
