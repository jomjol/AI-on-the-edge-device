#pragma once

#include "../../include/defines.h"

#ifdef DEBUG_ENABLE_SYSINFO

#ifndef ESP_SYS_H
#define ESP_SYS_H


#include <string>


// Device libraries (ESP-IDF)
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_heap_caps.h>

// for esp_spiram_get_size
extern "C" {
    #include <esp32/spiram.h>
    #ifdef USE_HIMEM_IF_AVAILABLE
        #include <esp32/himem.h>
    #endif
}



    void Restart();
    char *GetChipModel();
    uint8_t GetChipCoreCount();
    uint16_t GetChipRevision();
    uint32_t  GetChipfeatures();
    uint32_t GetFreeHeap();
    uint32_t GetLeastHeapFreeSinceBoot();

    std::string get_device_info();

    size_t getFreeMemoryInternal();
    size_t getFreeMemorySPIRAM();
    size_t getLargestFreeBlockInternal();
    size_t getLargestFreeBlockSPIRAM();
    size_t getMinEverFreeMemInternal();
    size_t getMinEverFreeMemSPIRAM();
    #ifdef USE_HIMEM_IF_AVAILABLE
        size_t getHimemTotSpace();
        size_t getHimemFreeSpace();
        size_t getHimemReservedArea();
    #endif


#endif //ESP_SYS_H

#endif // DEBUG_ENABLE_SYSINFO
