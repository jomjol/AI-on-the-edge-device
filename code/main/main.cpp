#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

// SD-Card ////////////////////
#include "nvs_flash.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
///////////////////////////////

#include "ClassLogFile.h"

#include "connect_wlan.h"
#include "read_wlanini.h"

#include "server_main.h"
#include "server_tflite.h"
#include "server_file.h"
#include "server_ota.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"
#include "server_main.h"
#include "server_camera.h"
#include "Helper.h"

extern const char* GIT_TAG;
extern const char* GIT_REV;
extern const char* GIT_BRANCH;
extern const char* BUILD_TIME;


#define __HIDE_PASSWORD

// #include "jomjol_WS2812Slow.h"
#include "SmartLeds.h"


#define __SD_USE_ONE_LINE_MODE__

#include "server_GPIO.h"


#define BLINK_GPIO GPIO_NUM_33

static const char *TAG = "MAIN";

//#define FLASH_GPIO GPIO_NUM_4

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
        .max_files = 7,                         // anstatt 5 (2022-09-21)
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

void task_NoSDBlink(void *pvParameter)
{
//    esp_rom_gpio_pad_select_gpio(BLINK_GPIO);

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
    string versionFormated = "Branch: '" + std::string(GIT_BRANCH) + "', Tag: '" + std::string(GIT_TAG) + \
            "', Revision: " + std::string(GIT_REV) +", Date/Time: " + std::string(BUILD_TIME);

    ESP_LOGI(TAG, "\n\n\n\n\n"); // Add mark on log to see when it restarted
    ESP_LOGD(TAG, "=============================================================================================");
    ESP_LOGD(TAG, "%s", versionFormated.c_str());
    ESP_LOGD(TAG, "=============================================================================================");
    ESP_LOGD(TAG, "Reset reason: %s", getResetReason().c_str());


    PowerResetCamera();
    esp_err_t cam = Camera.InitCam();
    Camera.LightOnOff(false);
    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "After camera initialization: sleep for: %ldms", (long) xDelay);
    vTaskDelay( xDelay );   


    if (!Init_NVS_SDCard())
    {
        xTaskCreate(&task_NoSDBlink, "task_NoSDBlink", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY+1, NULL);
        return;
    };

    CheckOTAUpdate();

    LogFile.CreateLogDirectories();
/*
    int mk_ret = mkdir("/sdcard/new_fd_mkdir", 0775);
    ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
    mk_ret = mkdir("/sdcard/new_fd_mkdir/test", 0775);
    ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
    MakeDir("/sdcard/test2");
    MakeDir("/sdcard/test2/intern");
*/


    char *ssid = NULL, *passwd = NULL, *hostname = NULL, *ip = NULL, *gateway = NULL, *netmask = NULL, *dns = NULL;
    LoadWlanFromFile("/sdcard/wlan.ini", ssid, passwd, hostname, ip, gateway, netmask, dns);

    if (ssid != NULL && passwd != NULL)
#ifdef __HIDE_PASSWORD
        ESP_LOGD(TAG, "WLan: %s, XXXXXX", ssid);
#else
        ESP_LOGD(TAG, "WLan: %s, %s", ssid, passwd);
#endif        

    else
        ESP_LOGD(TAG, "No SSID and PASSWORD set!!!");

    if (hostname != NULL)
        ESP_LOGD(TAG, "Hostename: %s", hostname);
    else
        ESP_LOGD(TAG, "Hostname not set");

    if (ip != NULL && gateway != NULL && netmask != NULL)
       ESP_LOGD(TAG, "Fixed IP: %s, Gateway %s, Netmask %s", ip, gateway, netmask);
    if (dns != NULL)
       ESP_LOGD(TAG, "DNS IP: %s", dns);


    wifi_init_sta(ssid, passwd, hostname, ip, gateway, netmask, dns);   


    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "main: sleep for: %ldms", (long) xDelay);
    vTaskDelay( xDelay );   
    setup_time();
    setBootTime();

    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=============================================================================================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================== Main Started ============================================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=============================================================================================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, versionFormated);
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Reset reason: " + getResetReason());

    std::string zw = gettimestring("%Y%m%d-%H%M%S");
    ESP_LOGD(TAG, "time %s", zw.c_str());

    size_t _hsize = getESPHeapSize();
    if (_hsize < 4000000)
    {
        std::string _zws = "Not enough PSRAM available. Expected 4.194.304 MByte - available: " + std::to_string(_hsize);
        _zws = _zws + "\nEither not initialzed, too small (2MByte only) or not present at all. Firmware cannot start!!";
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, _zws);
    } else {
        if (cam != ESP_OK) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to initialize camera module. "
                        "Check that your camera module is working and connected properly.");
        } else {
// Test Camera            
            camera_fb_t * fb = esp_camera_fb_get();
            if (!fb) {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Camera cannot be initialzed. "
                        "System will reboot.");
                doReboot();
            }
            esp_camera_fb_return(fb);   
            Camera.LightOnOff(false);
        }
    }



    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "main: sleep for: %ldms", (long) xDelay*10);
    vTaskDelay( xDelay ); 

    ESP_LOGD(TAG, "starting server");

    server = start_webserver();   
    register_server_camera_uri(server); 
    register_server_tflite_uri(server);
    register_server_file_uri(server, "/sdcard");
    register_server_ota_sdcard_uri(server);

    gpio_handler_create(server);

    ESP_LOGD(TAG, "vor reg server main");
    register_server_main_uri(server, "/sdcard");

    ESP_LOGD(TAG, "vor dotautostart");
    TFliteDoAutoStart();

}

