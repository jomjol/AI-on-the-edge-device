#include "sdcard_check.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "esp_rom_crc.h" 
#include "ClassLogFile.h"

static const char *TAG = "SDCARD";

int SDCardCheckRW(void)
{
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Basic R/W check started...");
    FILE* pFile = NULL;
    int iCRCMessage = 0;
   
    pFile = fopen("/sdcard/sdcheck.txt","w");
    if (pFile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Basic R/W check: (E1) No able to open file to write");
        return -1;
    } 
    else {
        std::string sMessage = "This message is used for a SD-Card basic check!";
        iCRCMessage = esp_rom_crc16_le(0, (uint8_t*)sMessage.c_str(), sMessage.length());
        if (fwrite(sMessage.c_str(), sMessage.length(), 1, pFile) == 0 ) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Basic R/W check: (E2) Not able to write file");
            fclose(pFile);
            unlink("/sdcard/sdcheck.txt");
            return -2;
        }
        fclose(pFile); 
    }

    pFile = fopen("/sdcard/sdcheck.txt","r");
    if (pFile == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Basic R/W check: (E3) Not able to open file to read back");
        unlink("/sdcard/sdcheck.txt");
        return -3;
    } 
    else {
        char cReadBuf[50];
        if (fgets(cReadBuf, sizeof(cReadBuf), pFile) == 0) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Basic R/W check: (E4) Not able to read file back");
            fclose(pFile);
            unlink("/sdcard/sdcheck.txt");
            return -4;
        }
        else {
            if (esp_rom_crc16_le(0, (uint8_t*)cReadBuf, strlen(cReadBuf)) != iCRCMessage) {                 
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Basic R/W check: (E5) Read back, but wrong CRC");
                fclose(pFile);
                unlink("/sdcard/sdcheck.txt");
                return -5;
            }
        }      
        fclose(pFile);
    }

    if (unlink("/sdcard/sdcheck.txt") != 0) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Basic R/W check: (E6) Unable to delete the file");
        return -6;
    }

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Basic R/W check successful");
    return 0;
}


bool SDCardCheckFolderStructure()
{
    struct stat sb;
    bool bRetval = true;

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Folder structure check started...");
    /* check if path exists: config */
    if (stat("/sdcard/config", &sb) != 0) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Folder structure check: (W1) Folder /config not found");
        bRetval = false;
    }

    /* check if path exists: html */
    if (stat("/sdcard/html", &sb) != 0) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Folder structure check: (W2) Folder /html not found");
        bRetval = false;
    }

    /* check if file exists: wlan.ini */
    if (stat("/sdcard/wlan.ini", &sb) != 0) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Folder structure check: (W3) File /wlan.ini not found");
        bRetval = false;
    }

    /* check if file exists: config.ini */
    if (stat("/sdcard/config/config.ini", &sb) != 0) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Folder structure check: (W4) File /config/config.ini not found");
        bRetval = false;
    }

    /* check if file exists: index.html */
    if (stat("/sdcard/html/index.html", &sb) != 0) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Folder structure check: (W5) File /html/index.html not found");
        bRetval = false;
    }

    if (bRetval)
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Folder structure check successful");
    
    return bRetval;
}