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

#include "server_tflite.h"
#include "server_file.h"
#include "server_ota.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"
#include "server_main.h"

#define __SD_USE_ONE_LINE_MODE__

#ifdef __SD_USE_ONE_LINE_MODE__
#include "server_GPIO.h"
#endif


#define BLINK_GPIO GPIO_NUM_33

static const char *TAGMAIN = "connect_wlan_main";

#define FLASH_GPIO GPIO_NUM_4

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
    gpio_set_pull_mode(GPIO_NUM_4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
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

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);


	// Init the GPIO
    // Flash ausschalten
    gpio_pad_select_gpio(FLASH_GPIO);
    gpio_set_direction(FLASH_GPIO, GPIO_MODE_OUTPUT);  
    gpio_set_level(FLASH_GPIO, 0);   

    return true;
}

void task_NoSDBlink(void *pvParameter)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);  
    
    TickType_t xDelay;
    xDelay = 100 / portTICK_PERIOD_MS;
    printf("SD-Card could not be inialized - STOP THE PROGRAMM HERE\n");

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
    if (!Init_NVS_SDCard())
    {
        xTaskCreate(&task_NoSDBlink, "task_NoSDBlink", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY+1, NULL);
        return;
    };
    CheckOTAUpdate();

    Camera.InitCam();

    LoadWlanFromFile("/sdcard/wlan.ini"); 
    ConnectToWLAN();
    printf("\nNetparameter: IP: %s - GW: %s - NetMask %s\n", getIPAddress().c_str(), getGW().c_str(), getNetMask().c_str());
    
    TickType_t xDelay;
    xDelay = 2000 / portTICK_PERIOD_MS;
//    printf("Autoflow: sleep for : %ldms\n", (long) xDelay);
//    LogFile.WriteToFile("Startsequence 06");      
    vTaskDelay( xDelay );   
//    LogFile.WriteToFile("Startsequence 07");  
    setup_time();
    LogFile.WriteToFile("=============================================================================================");
    LogFile.WriteToFile("=================================== Main Started ============================================");
    LogFile.WriteToFile("=============================================================================================");
    LogFile.SwitchOnOff(false);

//    std::string zw = gettimestring("%Y%m%d-%H%M%S");
//    printf("time %s\n", zw.c_str());    

    xDelay = 2000 / portTICK_PERIOD_MS;
    printf("Autoflow: sleep for : %ldms\n", (long) xDelay);
    vTaskDelay( xDelay ); 

    server = start_webserver();   
    register_server_tflite_uri(server);
    register_server_file_uri(server, "/sdcard");
    register_server_ota_sdcard_uri(server);

#ifdef __SD_USE_ONE_LINE_MODE__
    register_server_GPIO_uri(server);
#endif    

    register_server_main_uri(server, "/sdcard");

    TFliteDoAutoStart();
}