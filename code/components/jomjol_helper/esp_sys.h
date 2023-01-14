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



    void Restart();
    char *GetChipModel();
    uint8_t GetChipCoreCount();
    uint16_t GetChipRevision();
    uint32_t  GetChipfeatures();
    uint32_t GetFreeHeap();
    uint32_t GetLeastHeapFreeSinceBoot();

/*
    bool CHIP_FEATURE_EMB_FLASH; //Chip has embedded flash memory.
    bool CHIP_FEATURE_WIFI_BGN; //Chip has 2.4GHz WiFi.
    bool CHIP_FEATURE_BLE; //Chip has Bluetooth LE.
    bool CHIP_FEATURE_BT; //Chip has Bluetooth Classic.
    bool CHIP_FEATURE_IEEE802154; //Chip has IEEE 802.15.4 (Zigbee/Thread)
    bool CHIP_FEATURE_EMB_PSRAM; //Chip has embedded psram.
*/

    std::string get_device_info();

    size_t getFreeMemoryInternal();
    size_t  getFreeMemorySPIRAM();
    size_t  getLargestFreeBlockInternal();
    size_t  getLargestFreeBlockSPIRAM();
    size_t  getMinEverFreeMemInternal();
    size_t  getMinEverFreeMemSPIRAM();

#endif //ESP_SYS_H

#endif // DEBUG_ENABLE_SYSINFO