#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "sdkconfig.h"
#include "version.h"

#include "ClassLogFile.h"


//#include "esp_wifi.h"
//#include "protocol_examples_common.h"

#include "connect_wlan.h"

#include <esp_http_server.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"


#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"

#include "server_main.h"
#include "server_camera.h"
#include "server_tflite.h"
#include "server_file.h"
#include "server_ota.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "freertos/FreeRTOS.h"

// SD-Card
#include "nvs_flash.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "server_main.h"
#include "server_camera.h"
#include "ClassControllCamera.h"
#include "connect_wlan.h"
#include "time_sntp.h"

static const char *TAGMAIN = "connect_wlan_main";

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
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = { };
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;

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


extern "C" void app_main()
{
    printf("Do Reset Camera\n");
    PowerResetCamera();
//    LogFile.WriteToFile("Startsequence 01");
    Init_NVS_SDCard();
    LogFile.WriteToFile("Startsequence 02");
    CheckOTAUpdate();
    LogFile.WriteToFile("Startsequence 03");
    std::string ssid = "";
    std::string password = "";
    std::string hostname = "";

    LoadWlanFromFile("/sdcard/wlan.ini", ssid, password, hostname); 
    LogFile.WriteToFile("Startsequence 04");    
    printf("To use WLan: %s, %s\n", ssid.c_str(), password.c_str());
    printf("To set Hostename: %s\n", hostname.c_str());
   
    initialise_wifi(ssid, password, hostname);
    LogFile.WriteToFile("Startsequence 05");  

    TickType_t xDelay;
    xDelay = 2000 / portTICK_PERIOD_MS;
    printf("Autoflow: sleep for : %ldms\n", (long) xDelay);
    LogFile.WriteToFile("Startsequence 06");      
    vTaskDelay( xDelay );   
    LogFile.WriteToFile("Startsequence 07");  
    setup_time();
    LogFile.WriteToFile("======================== Main Started ================================");

    std::string zw = gettimestring("%Y%m%d-%H%M%S");
    printf("time %s\n", zw.c_str());    
    printf("libfive_git_version: %s\n", libfive_git_version());
    printf("libfive_git_revision: %s\n", libfive_git_revision());
    printf("libfive_git_branch: %s\n", libfive_git_branch());

    Camera.InitCam();
    Camera.LightOnOff(false); 
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
