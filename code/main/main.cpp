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
#include "server_camera.h"

static const char *TAGMAIN = "connect_wlan_main";


bool debug_detail_heap = true;

#define FLASH_GPIO GPIO_NUM_4

void Init_NVS_SDCard()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_LOGI(TAGMAIN, "Initializing SD card");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.flags = SDMMC_HOST_FLAG_1BIT;    
//    sdmmc_host_t host = SDMMC_HOST_SLOT_1();
//    host.flags = SDMMC_HOST_FLAG_1BIT;
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;  // 1 line SD mode
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = { };
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;

    gpio_set_pull_mode((gpio_num_t) 15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode((gpio_num_t) 2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes

    sdmmc_card_t* card;
    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAGMAIN, "Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAGMAIN, "Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.", ret);
        }
        return;
    }
    sdmmc_card_print_info(stdout, card);

	// Init the GPIO
    // Flash ausschalten
    gpio_pad_select_gpio(FLASH_GPIO);
    gpio_set_direction(FLASH_GPIO, GPIO_MODE_OUTPUT);  
    gpio_set_level(FLASH_GPIO, 0);   
}

extern "C" void app_main(void)
{
    printf("Do Reset Camera\n");
    PowerResetCamera();
    Camera.InitCam();
    Camera.LightOnOff(false); 

    Init_NVS_SDCard();
//    LogFile.WriteToFile("Startsequence 02");
    CheckOTAUpdate();
//    LogFile.WriteToFile("Startsequence 03");
    std::string ssid = "";
    std::string password = "";
    std::string hostname = "";
    std::string ip = "";
    std::string gw = "";
    std::string netmask = "";
    std::string dns = "";

//    LoadWlanFromFile("/sdcard/wlan.ini", ssid, password, hostname, ip, gw, netmask, dns); 
    LoadWlanFromFile("/sdcard/wlan.ini", ssid, password, hostname); 
    LoadNetConfigFromFile("/sdcard/wlan.ini", ip, gw, netmask, dns);

//    LogFile.WriteToFile("Startsequence 04");    
    printf("To use WLan: %s, %s\n", ssid.c_str(), password.c_str());
    printf("To set Hostename: %s\n", hostname.c_str());
    printf("Fixed IP: %s, Gateway %s, Netmask %s, DNS %s\n", ip.c_str(), gw.c_str(), netmask.c_str(), dns.c_str());
   
    if (ip.length() == 0 || gw.length() == 0 || netmask.length() == 0)
    {
        initialise_wifi(ssid, password, hostname);    
    }
    else
    {
        initialise_wifi_fixed_ip(ip, gw, netmask, ssid, password, hostname, dns);
    }

    printf("Netparameter: IP: %s - GW: %s - NetMask %s\n", getIPAddress().c_str(), getGW().c_str(), getNetMask().c_str());
    

//    LogFile.WriteToFile("Startsequence 05");  

    TickType_t xDelay;
    xDelay = 2000 / portTICK_PERIOD_MS;
    printf("Autoflow: sleep for : %ldms\n", (long) xDelay);
//    LogFile.WriteToFile("Startsequence 06");      
    vTaskDelay( xDelay );   
//    LogFile.WriteToFile("Startsequence 07");  
    setup_time();
    LogFile.WriteToFile("=============================================================================================");
    LogFile.WriteToFile("=================================== Main Started ============================================");
    LogFile.WriteToFile("=============================================================================================");
    LogFile.SwitchOnOff(false);

    std::string zw = gettimestring("%Y%m%d-%H%M%S");
    printf("time %s\n", zw.c_str());    

//    Camera.InitCam();
//    Camera.LightOnOff(false); 
    xDelay = 2000 / portTICK_PERIOD_MS;
    printf("Autoflow: sleep for : %ldms\n", (long) xDelay);
    vTaskDelay( xDelay ); 

    server = start_webserver();   
    register_server_camera_uri(server); 
    register_server_tflite_uri(server);
    register_server_file_uri(server, "/sdcard");
    register_server_ota_sdcard_uri(server);
    register_server_main_uri(server, "/sdcard");

    TFliteDoAutoStart();
}
