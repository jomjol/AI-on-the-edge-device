//#include <string>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/event_groups.h"

//#include "driver/gpio.h"
//#include "sdkconfig.h"
//#include "esp_psram.h" // Comming in IDF 5.0, see https://docs.espressif.com/projects/esp-idf/en/v5.0-beta1/esp32/migration-guides/release-5.x/system.html?highlight=esp_psram_get_size
//#include "spiram.h"
#include "esp32/spiram.h"


// SD-Card ////////////////////
//#include "nvs_flash.h"
#include "esp_vfs_fat.h"
//#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
//#include "driver/sdmmc_defs.h"
///////////////////////////////


#include "ClassLogFile.h"

#include "connect_wlan.h"
#include "read_wlanini.h"

#include "server_main.h"
#include "server_tflite.h"
#include "server_file.h"
#include "server_ota.h"
#include "time_sntp.h"
//#include "ClassControllCamera.h"
#include "server_main.h"
#include "server_camera.h"
#ifdef ENABLE_MQTT
    #include "server_mqtt.h"
#endif //ENABLE_MQTT
//#include "Helper.h"
#include "../../include/defines.h"
//#include "server_GPIO.h"

#ifdef ENABLE_SOFTAP
    #include "softAP.h"
#endif //ENABLE_SOFTAP

#ifdef DISABLE_BROWNOUT_DETECTOR
    #include "soc/soc.h" 
    #include "soc/rtc_cntl_reg.h" 
#endif

extern const char* GIT_TAG;
extern const char* GIT_REV;
extern const char* GIT_BRANCH;
extern const char* BUILD_TIME;

extern std::string getFwVersion(void);
extern std::string getHTMLversion(void);
extern std::string getHTMLcommit(void);

static const char *TAG = "MAIN";

bool Init_NVS_SDCard()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
////////////////////////////////////////////////

    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:

#ifdef __SD_USE_ONE_LINE_MODE__
    slot_config.width = 1;
#endif

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(GPIO_NUM_2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
#ifndef __SD_USE_ONE_LINE_MODE__
    gpio_set_pull_mode(GPIO_NUM_4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
#endif
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 12,                         // previously -> 2022-09-21: 5, 2023-01-02: 7 
        .allocation_unit_size = 16 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return false;
    }

    sdmmc_card_print_info(stdout, card);
    SaveSDCardInfo(card);
    return true;
}

void task_MainInitError_blink(void *pvParameter)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);  

    
    TickType_t xDelay;
    xDelay = 100 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "SD-Card could not be inialized - STOP THE PROGRAMM HERE");

    while (1)
    {
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay( xDelay );   
        gpio_set_level(BLINK_GPIO, 0); 
        vTaskDelay( xDelay );   

    }
    vTaskDelete(NULL); //Delete this task if it exits from the loop above
}


extern "C" void app_main(void)
{
    TickType_t xDelay;
    
#ifdef DISABLE_BROWNOUT_DETECTOR
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
#endif

    ESP_LOGI(TAG, "\n\n\n\n\n"); // Add mark on log to see when it restarted
    
    PowerResetCamera();
    esp_err_t camStatus = Camera.InitCam();
    Camera.LightOnOff(false);
    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "After camera initialization: sleep for: %ldms", (long) xDelay);
    vTaskDelay( xDelay );   

    if (!Init_NVS_SDCard())
    {
        xTaskCreate(&task_MainInitError_blink, "task_MainInitError_blink", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY+1, NULL);
        return; // No way to continue without SD-Card!
    }

    setupTime();

    string versionFormated = getFwVersion() + ", Date/Time: " + std::string(BUILD_TIME) + \
        ", Web UI: " + getHTMLversion();

    if (std::string(GIT_TAG) != "") { // We are on a tag, add it as prefix
        string versionFormated = "Tag: '" + std::string(GIT_TAG) + "', " + versionFormated;
    }

    LogFile.CreateLogDirectories();
    MakeDir("/sdcard/demo");            // needed for demo mode


    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "==================== Startup ====================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, versionFormated);
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Reset reason: " + getResetReason());

    CheckIsPlannedReboot();
    CheckOTAUpdate();
    CheckUpdate();
    #ifdef ENABLE_SOFTAP
        CheckStartAPMode();          // if no wlan.ini and/or config.ini --> AP ist startet and this function does not exit anymore until reboot
    #endif

    char *ssid = NULL, *passwd = NULL, *hostname = NULL, *ip = NULL, *gateway = NULL, *netmask = NULL, *dns = NULL; int rssithreashold = 0;
    LoadWlanFromFile(WLAN_CONFIG_FILE, ssid, passwd, hostname, ip, gateway, netmask, dns, rssithreashold);

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "WLAN-Settings - RSSI-Threashold: " + to_string(rssithreashold));

    if (ssid != NULL && passwd != NULL)
#ifdef __HIDE_PASSWORD
        ESP_LOGD(TAG, "WLan: %s, XXXXXX", ssid);
#else
        ESP_LOGD(TAG, "WLan: %s, %s", ssid, passwd);
#endif        

    else
        ESP_LOGD(TAG, "No SSID and PASSWORD set!!!");

    if (hostname != NULL)
        ESP_LOGD(TAG, "Hostname: %s", hostname);
    else
        ESP_LOGD(TAG, "Hostname not set");

    if (ip != NULL && gateway != NULL && netmask != NULL)
       ESP_LOGD(TAG, "Fixed IP: %s, Gateway %s, Netmask %s", ip, gateway, netmask);
    if (dns != NULL)
       ESP_LOGD(TAG, "DNS IP: %s", dns);


    wifi_init_sta(ssid, passwd, hostname, ip, gateway, netmask, dns, rssithreashold);   


    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "main: sleep for: %ldms", (long) xDelay);
    vTaskDelay( xDelay );   

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "================== Main Started =================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================================");

    if (getHTMLcommit().substr(0, 7) != std::string(GIT_REV).substr(0, 7)) { // Compare the first 7 characters of both hashes
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, std::string("Web UI version (") + getHTMLcommit() + ") does not match firmware version (" + std::string(GIT_REV) + ") !");
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Please make sure to setup the SD-Card properly (check the wiki) or re-install using the update_*.zip!");    
    }

    std::string zw = getCurrentTimeString("%Y%m%d-%H%M%S");
    ESP_LOGD(TAG, "time %s", zw.c_str());

    /* Check if PSRAM can be initalized */
    esp_err_t ret;
    ret = esp_spiram_init();
    if (ret == ESP_FAIL) { // Failed to init PSRAM, most likely not available or broken
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to initialize PSRAM (" + std::to_string(ret) + ")!");
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Either your device misses the PSRAM chip or it is broken!");
        setSystemStatusFlag(SYSTEM_STATUS_PSRAM_BAD);
    }
    else { // PSRAM init ok
        /* Check if PSRAM provides at least 4 MB */
        size_t psram_size = esp_spiram_get_size();        
        // size_t psram_size = esp_psram_get_size(); // comming in IDF 5.0
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "The device has " + std::to_string(psram_size/1024/1024) + " MBytes of PSRAM");
        if (psram_size < (4*1024*1024)) { // PSRAM is below 4 MBytes
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "At least 4 MBytes are required!");
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Does the device really have a 4 Mbytes PSRAM?");
            setSystemStatusFlag(SYSTEM_STATUS_PSRAM_BAD);
        }
    }

    /* Check available Heap memory */
    size_t _hsize = getESPHeapSize();
    if (_hsize < 4000000) { // Check available Heap memory for a bit less than 4 MB (a test on a good device showed 4187558 bytes to be available)
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Not enough Heap memory available. Expected around 4 MBytes, but only " + std::to_string(_hsize) + " Bytes are available! That is not enough for this firmware!");
        setSystemStatusFlag(SYSTEM_STATUS_HEAP_TOO_SMALL);
    } else { // Heap memory is ok
        if (camStatus != ESP_OK) {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Failed to initialize camera module, retrying...");

            PowerResetCamera();
            esp_err_t camStatus = Camera.InitCam();
            Camera.LightOnOff(false);
            xDelay = 2000 / portTICK_PERIOD_MS;
            ESP_LOGD(TAG, "After camera initialization: sleep for: %ldms", (long) xDelay);
            vTaskDelay( xDelay ); 

            if (camStatus != ESP_OK) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to initialize camera module!");
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Check that your camera module is working and connected properly!");
                setSystemStatusFlag(SYSTEM_STATUS_CAM_BAD);
            }
        } else { // Test Camera    
            if (!Camera.testCamera()) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Camera Framebuffer cannot be initialized!");
                /* Easiest would be to simply restart here and try again,
                   how ever there seem to be systems where it fails at startup but still work corectly later.
                   Therefore we treat it still as successed! */
                   setSystemStatusFlag(SYSTEM_STATUS_CAM_FB_BAD);
            }
            else {
                Camera.LightOnOff(false);
            }
        }
    }

    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "main: sleep for: %ldms", (long) xDelay*10);
    vTaskDelay( xDelay ); 

    ESP_LOGD(TAG, "starting servers");

    server = start_webserver();   
    register_server_camera_uri(server); 
    register_server_tflite_uri(server);
    register_server_file_uri(server, "/sdcard");
    register_server_ota_sdcard_uri(server);
    #ifdef ENABLE_MQTT
        register_server_mqtt_uri(server);
    #endif //ENABLE_MQTT

    gpio_handler_create(server);

    ESP_LOGD(TAG, "Before reg server main");
    register_server_main_uri(server, "/sdcard");


    /* Testing */
    //setSystemStatusFlag(SYSTEM_STATUS_CAM_FB_BAD);
    //setSystemStatusFlag(SYSTEM_STATUS_PSRAM_BAD);

    /* Main Init has successed or only an error which allows to continue operation */
    if (getSystemStatus() == 0) { // No error flag is set
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Initialization completed successfully!");
        ESP_LOGD(TAG, "Before do autostart");
        TFliteDoAutoStart();
    }
    else if (isSetSystemStatusFlag(SYSTEM_STATUS_CAM_FB_BAD) || // Non critical errors occured, we try to continue...
        isSetSystemStatusFlag(SYSTEM_STATUS_NTP_BAD)) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Initialization completed with errors, but trying to continue...");
        ESP_LOGD(TAG, "Before do autostart");
        TFliteDoAutoStart();
    }
    else { // Any other error is critical and makes running the flow impossible.
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Initialization failed. Not starting flows!");
    }
}

