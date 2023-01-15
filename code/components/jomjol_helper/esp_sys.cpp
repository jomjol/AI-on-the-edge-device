#include "../../include/defines.h"

#ifdef DEBUG_ENABLE_SYSINFO

#include "esp_sys.h"

#include <string>


#include "esp_chip_info.h"


void Restart() {
    esp_restart();
}

//source : https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv416esp_chip_model_t

//https://github.com/espressif/esp-idf/blob/8464186e67e34b417621df6b6f1f289a6c60b859/components/esp_hw_support/include/esp_chip_info.h
/*
typedef enum {
    CHIP_ESP32  = 1, //!< ESP32
    CHIP_ESP32S2 = 2, //!< ESP32-S2
    CHIP_ESP32S3 = 9, //!< ESP32-S3
    CHIP_ESP32C3 = 5, //!< ESP32-C3
    CHIP_ESP32H4 = 6, //!< ESP32-H4
    CHIP_ESP32C2 = 12, //!< ESP32-C2
    CHIP_ESP32C6 = 13, //!< ESP32-C6
    CHIP_ESP32H2 = 16, //!< ESP32-H2
    CHIP_POSIX_LINUX = 999, //!< The code is running on POSIX/Linux simulator
} esp_chip_model_t;
*/

char* GetChipModel(){
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    switch((int)chipInfo.model) {
        case 0 : return (char*)"ESP8266";
        case (int)esp_chip_model_t::CHIP_ESP32 : return (char*)"ESP32";
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)        
        case (int)esp_chip_model_t::CHIP_ESP32S2 : return (char*)"ESP32-S2";
        case (int)esp_chip_model_t::CHIP_ESP32S3 : return (char*)"ESP32-S3";
        case (int)esp_chip_model_t::CHIP_ESP32C3 : return (char*)"ESP32-C3";
        case 6 : return (char*)"ESP32-H4";
        case 12 : return (char*)"ESP32-C2";
        case 13 : return (char*)"ESP32-C6";
        //case (int)esp_chip_model_t::CHIP_ESP32H4 : return (char*)"ESP32-H4";
        //case (int)esp_chip_model_t::CHIP_ESP32C2 : return (char*)"ESP32-C2";
        //case (int)esp_chip_model_t::CHIP_ESP32C6 : return (char*)"ESP32-C6";
        //case (int)esp_chip_model_t::CHIP_ESP32H2 : return (char*)"ESP32-H2";
        case 16 : return (char*)"ESP32-H2";
        //case (int)esp_chip_model_t::CHIP_POSIX_LINUX : return (char*)"CHIP_POSIX_LINUX";

#endif
    }
    return (char*)"Chip Unknown";
}

uint8_t GetChipCoreCount() {
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    return chipInfo.cores;
}

uint16_t GetChipRevision() {
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    return chipInfo.revision;
}

uint32_t  GetChipfeatures() {
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    return chipInfo.features;
}


uint32_t GetFreeHeap() {
    return esp_get_free_heap_size();
}

uint32_t GetLeastHeapFreeSinceBoot() {
    return esp_get_minimum_free_heap_size();
}


std::string get_device_info()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    std::string espInfoResultStr = "";
    char aMsgBuf[40];

    espInfoResultStr += "Device Info:";
    espInfoResultStr += "---------------\n";
    espInfoResultStr += "Chip Model: " + std::string(GetChipModel()) +"\n";
    sprintf(aMsgBuf,"Chip Revision: %d\n", chip_info.revision);
    espInfoResultStr += std::string(aMsgBuf);
    sprintf(aMsgBuf,"CPU Cores: %d\n", chip_info.cores);
    espInfoResultStr += std::string(aMsgBuf);
    sprintf(aMsgBuf,"Flash Memory: %dMB\n", spi_flash_get_chip_size()/(1024*1024));
    espInfoResultStr += std::string(aMsgBuf);
    if(chip_info.features & CHIP_FEATURE_WIFI_BGN)
        //espInfoResultStr += "Base MAC: " + std::string(getMac()) +"\n";
        espInfoResultStr += "ESP-IDF version: " + std::string(esp_get_idf_version()) +"\n";
    if((chip_info.features & CHIP_FEATURE_WIFI_BGN) || (chip_info.features & CHIP_FEATURE_BT) ||
       (chip_info.features & CHIP_FEATURE_BLE) || (chip_info.features & CHIP_FEATURE_EMB_FLASH))
    {
        espInfoResultStr += "Characteristics:\n";
        if(chip_info.features & CHIP_FEATURE_WIFI_BGN)
            espInfoResultStr += "    WiFi 2.4GHz\n";
        if(chip_info.features & CHIP_FEATURE_BT)
            espInfoResultStr += "    Bluetooth Classic\n";
        if(chip_info.features & CHIP_FEATURE_BLE)
            espInfoResultStr += "    Bluetooth Low Energy\n";
        if(chip_info.features & CHIP_FEATURE_EMB_FLASH)
            espInfoResultStr += "    Embedded Flash memory\n";
        else
           espInfoResultStr += "    External Flash memory\n";
    }

    #ifdef USE_HIMEM_IF_AVAILABLE
        sprintf(aMsgBuf,"spiram size %u\n", esp_spiram_get_size());
        espInfoResultStr += std::string(aMsgBuf);
        sprintf(aMsgBuf,"himem free %u\n", esp_himem_get_free_size());
        espInfoResultStr += std::string(aMsgBuf);
        sprintf(aMsgBuf,"himem phys %u\n", esp_himem_get_phys_size());
        espInfoResultStr += std::string(aMsgBuf);
        sprintf(aMsgBuf,"himem reserved %u\n", esp_himem_reserved_area_size());
        espInfoResultStr += std::string(aMsgBuf);
    #endif
    
    return espInfoResultStr; 
}


size_t getFreeMemoryInternal(){ //Current Free Memory
    return heap_caps_get_free_size(MALLOC_CAP_8BIT) - heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
}

size_t getFreeMemorySPIRAM(){ //Current Free Memory
    return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
}


size_t getLargestFreeBlockInternal(){ //Largest Free Block
    return heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

size_t getLargestFreeBlockSPIRAM(){ //Largest Free Block
    return heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
}


size_t getMinEverFreeMemInternal(){ //Min. Ever Free Size
    return heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

size_t getMinEverFreeMemSPIRAM(){ //Min. Ever Free Size
    return heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
}

#ifdef USE_HIMEM_IF_AVAILABLE
    size_t getHimemTotSpace(){ 
        return esp_himem_get_phys_size();
    }

    size_t getHimemFreeSpace(){ 
        return esp_himem_get_free_size();
    }

    size_t getHimemReservedArea(){ 
        return esp_himem_reserved_area_size();
    }
#endif

#endif //DEBUG_ENABLE_SYSINFO
