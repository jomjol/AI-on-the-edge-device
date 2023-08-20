#include <iostream>
#include <string>
#include <vector>
#include <regex>

//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/event_groups.h"

//#include "driver/gpio.h"
//#include "sdkconfig.h"
#include "esp_psram.h"
#include "esp_pm.h"

#include "psram.h"

#include "esp_chip_info.h"



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
#include "MainFlowControl.h"
#include "server_file.h"
#include "server_ota.h"
#include "time_sntp.h"
#include "configFile.h"
//#include "ClassControllCamera.h"
#include "server_main.h"
#include "server_camera.h"
#ifdef ENABLE_MQTT
    #include "server_mqtt.h"
#endif //ENABLE_MQTT
#include "Helper.h"
#include "statusled.h"
#include "sdcard_check.h"

#include "../../include/defines.h"
//#include "server_GPIO.h"

#ifdef ENABLE_SOFTAP
    #include "softAP.h"
#endif //ENABLE_SOFTAP

#ifdef DISABLE_BROWNOUT_DETECTOR
    #include "soc/soc.h" 
    #include "soc/rtc_cntl_reg.h" 
#endif

#ifdef DEBUG_ENABLE_SYSINFO
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 4, 0, 0 )
    #include "esp_sys.h"
#endif
#endif //DEBUG_ENABLE_SYSINFO

// define `gpio_pad_select_gpip` for newer versions of IDF
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0))
#include "esp_rom_gpio.h"
#define gpio_pad_select_gpio esp_rom_gpio_pad_select_gpio
#endif

#ifdef USE_HIMEM_IF_AVAILABLE
    #include "esp32/himem.h"
    #ifdef DEBUG_HIMEM_MEMORY_CHECK
        #include "himem_memory_check.h"
    #endif
#endif

//#ifdef CONFIG_HEAP_TRACING_STANDALONE
#if defined HEAP_TRACING_MAIN_WIFI || defined HEAP_TRACING_MAIN_START
    #include <esp_heap_trace.h>
    #define NUM_RECORDS 300
    static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be in internal RAM
#endif

extern const char* GIT_TAG;
extern const char* GIT_REV;
extern const char* GIT_BRANCH;
extern const char* BUILD_TIME;

extern std::string getFwVersion(void);
extern std::string getHTMLversion(void);
extern std::string getHTMLcommit(void);

std::vector<std::string> splitString(const std::string& str);
void migrateConfiguration(void);
bool setCpuFrequency(void);

static const char *TAG = "MAIN";


bool Init_NVS_SDCard()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_LOGD(TAG, "Using SDMMC peripheral");
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

    sdmmc_card_t* card;
    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount FAT filesystem on SD card. Check SD card filesystem (only FAT supported) or try another card");
            StatusLED(SDCARD_INIT, 1, true);
        } 
        else if (ret == 263) { // Error code: 0x107 --> usually: SD not found
            ESP_LOGE(TAG, "SD card init failed. Check if SD card is properly inserted into SD card slot or try another card");
            StatusLED(SDCARD_INIT, 2, true);
        }
        else {
            ESP_LOGE(TAG, "SD card init failed. Check error code or try another card");
            StatusLED(SDCARD_INIT, 3, true);
        }
        return false;
    }

    //sdmmc_card_print_info(stdout, card);  // With activated CONFIG_NEWLIB_NANO_FORMAT --> capacity not printed correctly anymore
    SaveSDCardInfo(card);
    return true;
}


extern "C" void app_main(void)
{
    //#ifdef CONFIG_HEAP_TRACING_STANDALONE
    #if defined HEAP_TRACING_MAIN_WIFI || defined HEAP_TRACING_MAIN_START
        //register a buffer to record the memory trace
        ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );
    #endif
        
    TickType_t xDelay;
        
    #ifdef DISABLE_BROWNOUT_DETECTOR
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    #endif

    #ifdef HEAP_TRACING_MAIN_START
        ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    #endif

    // ********************************************
    // Highlight start of app_main 
    // ********************************************
    ESP_LOGI(TAG, "\n\n\n\n================ Start app_main =================");
 

    // Init SD card
    // ********************************************
    if (!Init_NVS_SDCard())
    {
        ESP_LOGE(TAG, "Device init aborted!");
        return; // No way to continue without working SD card!
    }

    // SD card: Create log directories (if not already existing)
    // ********************************************
    LogFile.CreateLogDirectories(); // mandatory for logging + image saving

    // ********************************************
    // Highlight start of logfile logging
    // Default Log Level: INFO -> Everything which needs to be logged during boot should be have level INFO, WARN OR ERROR
    // ********************************************
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "==================== Start ======================");
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "=================================================");


    // Init external PSRAM
    // ********************************************
    esp_err_t PSRAMStatus = esp_psram_init();
    if (PSRAMStatus == ESP_FAIL) {  // ESP_FAIL -> Failed to init PSRAM
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "PSRAM init failed (" + std::to_string(PSRAMStatus) + ")! PSRAM not found or defective");
        setSystemStatusFlag(SYSTEM_STATUS_PSRAM_BAD);
        StatusLED(PSRAM_INIT, 1, true);
    }
    else { // ESP_OK -> PSRAM init OK --> continue to check PSRAM size
        size_t psram_size = esp_psram_get_size(); // size_t psram_size = esp_psram_get_size(); // comming in IDF 5.0
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "PSRAM size: " + std::to_string(psram_size) + " byte (" + std::to_string(psram_size/1024/1024) + 
                                               "MB / " + std::to_string(psram_size/1024/1024*8) + "MBit)");

        // Check PSRAM size
        // ********************************************
        if (psram_size < (4*1024*1024)) { // PSRAM is below 4 MBytes (32Mbit)
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "PSRAM size >= 4MB (32Mbit) is mandatory to run this application");
            setSystemStatusFlag(SYSTEM_STATUS_PSRAM_BAD);
            StatusLED(PSRAM_INIT, 2, true);
        }
        else { // PSRAM size OK --> continue to check heap size
            size_t _hsize = getESPHeapSize();
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Total heap: " + std::to_string(_hsize) + " byte");

            // Check heap memory
            // ********************************************
            if (_hsize < 4000000) { // Check available Heap memory for a bit less than 4 MB (a test on a good device showed 4187558 bytes to be available)
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Total heap >= 4000000 byte is mandatory to run this application");
                setSystemStatusFlag(SYSTEM_STATUS_HEAP_TOO_SMALL);
                StatusLED(PSRAM_INIT, 3, true);
            }
            else { // HEAP size OK --> continue to reserve shared memory block and check camera init
                /* Allocate static PSRAM memory regions */
                if (! reserve_psram_shared_region()) {
                    setSystemStatusFlag(SYSTEM_STATUS_HEAP_TOO_SMALL);
                    StatusLED(PSRAM_INIT, 3, true);
                }
                else { // OK
                    // Init camera
                    // ********************************************
                    PowerResetCamera();
                    esp_err_t camStatus = Camera.InitCam();
                    Camera.LightOnOff(false);

                    xDelay = 2000 / portTICK_PERIOD_MS;
                    ESP_LOGD(TAG, "After camera initialization: sleep for: %ldms", (long) xDelay * CONFIG_FREERTOS_HZ/portTICK_PERIOD_MS);
                    vTaskDelay( xDelay );


                    // Check camera init
                    // ********************************************
                    if (camStatus != ESP_OK) { // Camera init failed, retry to init
                        char camStatusHex[33];
                        sprintf(camStatusHex,"0x%02x", camStatus);
                        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Camera init failed (" + std::string(camStatusHex) + "), retrying...");

                        PowerResetCamera();
                        camStatus = Camera.InitCam();
                        Camera.LightOnOff(false);

                        xDelay = 2000 / portTICK_PERIOD_MS;
                        ESP_LOGD(TAG, "After camera initialization: sleep for: %ldms", (long) xDelay * CONFIG_FREERTOS_HZ/portTICK_PERIOD_MS);
                        vTaskDelay( xDelay ); 

                        if (camStatus != ESP_OK) { // Camera init failed again
                            sprintf(camStatusHex,"0x%02x", camStatus);
                            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Camera init failed (" + std::string(camStatusHex) +
                                                                    ")! Check camera module and/or proper electrical connection");
                            setSystemStatusFlag(SYSTEM_STATUS_CAM_BAD);
                            StatusLED(CAM_INIT, 1, true);
                        }
                    }
                    else { // ESP_OK -> Camera init OK --> continue to perform camera framebuffer check
                        // Camera framebuffer check
                        // ********************************************
                        if (!Camera.testCamera()) {
                            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Camera framebuffer check failed");
                            // Easiest would be to simply restart here and try again,
                            // how ever there seem to be systems where it fails at startup but still work correctly later.
                            // Therefore we treat it still as successed! */
                            setSystemStatusFlag(SYSTEM_STATUS_CAM_FB_BAD);
                            StatusLED(CAM_INIT, 2, false);
                        }
                        Camera.LightOnOff(false);   // make sure flashlight is off before start of flow

                        // Print camera infos
                        // ********************************************
                        char caminfo[50];
                        sensor_t * s = esp_camera_sensor_get();
                        sprintf(caminfo, "PID: 0x%02x, VER: 0x%02x, MIDL: 0x%02x, MIDH: 0x%02x", s->id.PID, s->id.VER, s->id.MIDH, s->id.MIDL);
                        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Camera info: " + std::string(caminfo));
                    }
                }
            }
        }
    }


    // SD card: basic R/W check
    // ********************************************
    int iSDCardStatus = SDCardCheckRW();
    if (iSDCardStatus < 0) {
        if (iSDCardStatus <= -1 && iSDCardStatus >= -2) { // write error
            StatusLED(SDCARD_CHECK, 1, true);
        }
        else if (iSDCardStatus <= -3 && iSDCardStatus >= -5) { // read error
            StatusLED(SDCARD_CHECK, 2, true);
        }
        else if (iSDCardStatus == -6) { // delete error
            StatusLED(SDCARD_CHECK, 3, true);
        }
        setSystemStatusFlag(SYSTEM_STATUS_SDCARD_CHECK_BAD); // reduced web interface going to be loaded
    }

    // Migrate parameter in config.ini to new naming (firmware 15.0 and newer)
    // ********************************************
    migrateConfiguration();

    // Init time (as early as possible, but SD card needs to be initialized)
    // ********************************************
    setupTime();    // NTP time service: Status of time synchronization will be checked after every round (server_tflite.cpp)


    // Set CPU Frequency
    // ********************************************
    setCpuFrequency();


    // SD card: Create further mandatory directories (if not already existing)
    // Correct creation of these folders will be checked with function "SDCardCheckFolderFilePresence"
    // ********************************************
    MakeDir("/sdcard/firmware");         // mandatory for OTA firmware update
    MakeDir("/sdcard/img_tmp");          // mandatory for setting up alignment marks
    MakeDir("/sdcard/demo");             // mandatory for demo mode

    // Check for updates
    // ********************************************
    CheckOTAUpdate();
    CheckUpdate();

    // Start SoftAP for initial remote setup
    // Note: Start AP if no wlan.ini and/or config.ini available, e.g. SD card empty; function does not exit anymore until reboot
    // ********************************************
    #ifdef ENABLE_SOFTAP
        CheckStartAPMode(); 
    #endif

    // SD card: Check presence of some mandatory folders / files
    // ********************************************
    if (!SDCardCheckFolderFilePresence()) {
        StatusLED(SDCARD_CHECK, 4, true);
        setSystemStatusFlag(SYSTEM_STATUS_FOLDER_CHECK_BAD); // reduced web interface going to be loaded
    }

    // Check version information
    // ********************************************
    std::string versionFormated = getFwVersion() + ", Date/Time: " + std::string(BUILD_TIME) + \
        ", Web UI: " + getHTMLversion();

    if (std::string(GIT_TAG) != "") { // We are on a tag, add it as prefix
        versionFormated = "Tag: '" + std::string(GIT_TAG) + "', " + versionFormated;
    }
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, versionFormated);

    if (getHTMLcommit().substr(0, 7) == "?")
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, std::string("Failed to read file html/version.txt to parse Web UI version"));
 
    if (getHTMLcommit().substr(0, 7) != std::string(GIT_REV).substr(0, 7)) { // Compare the first 7 characters of both hashes
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Web UI version (" + getHTMLcommit() + ") does not match firmware version (" + std::string(GIT_REV) + ")");
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Recommendation: Repeat installation using AI-on-the-edge-device__update__*.zip");    
    }

    // Check reboot reason
    // ********************************************
    CheckIsPlannedReboot();
    if (!getIsPlannedReboot() && (esp_reset_reason() == ESP_RST_PANIC)) {  // If system reboot was not triggered by user and reboot was caused by execption 
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Reset reason: " + getResetReason());
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Device was rebooted due to a software exception! Log level is set to DEBUG until the next reboot. "
                                               "Flow init is delayed by 5 minutes to check the logs or do an OTA update"); 
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Keep device running until crash occurs again and check logs after device is up again");
        LogFile.setLogLevel(ESP_LOG_DEBUG);
    }
    else {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Reset reason: " + getResetReason());
    }

    #ifdef HEAP_TRACING_MAIN_START
        ESP_ERROR_CHECK( heap_trace_stop() );
        heap_trace_dump(); 
    #endif
    
    #ifdef HEAP_TRACING_MAIN_WIFI
        ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
    #endif

    // Read WLAN parameter and start WIFI
    // ********************************************
    int iWLANStatus = LoadWlanFromFile(WLAN_CONFIG_FILE);
    if (iWLANStatus == 0) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "WLAN config loaded, init WIFI...");
        if (wifi_init_sta() != ESP_OK) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "WIFI init failed. Device init aborted!");
            StatusLED(WLAN_INIT, 3, true);
            return;
        }
    }
    else if (iWLANStatus == -1) {  // wlan.ini not available, potentially empty or content not readable
        StatusLED(WLAN_INIT, 1, true);
        return; // No way to continue without reading the wlan.ini
    }
    else if (iWLANStatus == -2) { // SSID or password not configured
        StatusLED(WLAN_INIT, 2, true);
        return; // No way to continue with empty SSID or password!
    }

    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "main: sleep for: %ldms", (long) xDelay * CONFIG_FREERTOS_HZ/portTICK_PERIOD_MS);
    vTaskDelay( xDelay );


    // manual reset the time
    // ********************************************
    if (!time_manual_reset_sync())
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Manual Time Sync failed during startup" );
    }



    // Set log level for wifi component to WARN level (default: INFO; only relevant for serial console)
    // ********************************************
    esp_log_level_set("wifi", ESP_LOG_WARN);
  
    #ifdef HEAP_TRACING_MAIN_WIFI
        ESP_ERROR_CHECK( heap_trace_stop() );
        heap_trace_dump(); 
    #endif   

    #ifdef DEBUG_ENABLE_SYSINFO
        #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 4, 0, 0 )
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Device Info : " + get_device_info() );
            ESP_LOGD(TAG, "Device infos %s", get_device_info().c_str());
        #endif
    #endif //DEBUG_ENABLE_SYSINFO

    #ifdef USE_HIMEM_IF_AVAILABLE
        #ifdef DEBUG_HIMEM_MEMORY_CHECK
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Himem mem check : " + himem_memory_check() );
            ESP_LOGD(TAG, "Himem mem check %s", himem_memory_check().c_str());
        #endif
    #endif
   


    // Print Device info
    // ********************************************
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Device info: CPU cores: " + std::to_string(chipInfo.cores) + 
                                           ", Chip revision: " + std::to_string(chipInfo.revision));
    
    // Print SD-Card info
    // ********************************************
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "SD card info: Name: " + getSDCardName() + ", Capacity: " + 
                        getSDCardCapacity() + "MB, Free: " + getSDCardFreePartitionSpace() + "MB");

    xDelay = 2000 / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "main: sleep for: %ldms", (long) xDelay * CONFIG_FREERTOS_HZ/portTICK_PERIOD_MS);
    vTaskDelay( xDelay ); 

    // Start webserver + register handler
    // ********************************************
    ESP_LOGD(TAG, "starting servers");

    server = start_webserver();   
    register_server_camera_uri(server); 
    register_server_main_flow_task_uri(server);
    register_server_file_uri(server, "/sdcard");
    register_server_ota_sdcard_uri(server);
    #ifdef ENABLE_MQTT
        register_server_mqtt_uri(server);
    #endif //ENABLE_MQTT

    gpio_handler_create(server);

    ESP_LOGD(TAG, "Before reg server main");
    register_server_main_uri(server, "/sdcard");

    // Only for testing purpose
    //setSystemStatusFlag(SYSTEM_STATUS_CAM_FB_BAD);
    //setSystemStatusFlag(SYSTEM_STATUS_PSRAM_BAD);

    // Check main init + start TFlite task
    // ********************************************
    if (getSystemStatus() == 0) { // No error flag is set
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Initialization completed successfully");
        InitializeFlowTask();
    }
    else if (isSetSystemStatusFlag(SYSTEM_STATUS_CAM_FB_BAD) || // Non critical errors occured, we try to continue...
             isSetSystemStatusFlag(SYSTEM_STATUS_NTP_BAD)) {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Initialization completed with non-critical errors!");
        InitializeFlowTask();
    }
    else { // Any other error is critical and makes running the flow impossible. Init is going to abort.
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Initialization failed. Flow task start aborted. Loading reduced web interface...");
    }
}


void migrateConfiguration(void) {
    bool migrated = false;

    if (!FileExists(CONFIG_FILE)) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Config file seems to be missing!");
        return;	
    }

    std::string section = "";
	std::ifstream ifs(CONFIG_FILE);
  	std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	
    /* Split config file it array of lines */
    std::vector<std::string> configLines = splitString(content);

    /* Process each line */
    for (int i = 0; i < configLines.size(); i++) {
        //ESP_LOGI(TAG, "Line %d: %s", i, configLines[i].c_str());

        if (configLines[i].find("[") != std::string::npos) { // Start of new section
            section = configLines[i];
            replaceString(section, ";", "", false); // Remove possible semicolon (just for the string comparison)
            //ESP_LOGI(TAG, "New section: %s", section.c_str());
        }

        /* Migrate parameters as needed
         * For the boolean parameters, we make them enabled all the time now:
         *  1. If they where disabled, set them to their default value
         *  2. Enable them
         * Notes:
         * The migration has some simplifications:
         *  - Case Sensitiveness must be like in the initial config.ini
         *  - No Whitespace after a semicollon
         *  - Only one whitespace before/after the equal sign
         */
        if (section == "[MakeImage]") {
            migrated = migrated | replaceString(configLines[i], "[MakeImage]", "[TakeImage]"); // Rename the section itself
        }

        if (section == "[MakeImage]" || section == "[TakeImage]") {
            migrated = migrated | replaceString(configLines[i], "LogImageLocation", "RawImagesLocation");
            migrated = migrated | replaceString(configLines[i], "LogfileRetentionInDays", "RawImagesRetention");

            migrated = migrated | replaceString(configLines[i], ";Demo = true", ";Demo = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";Demo", "Demo"); // Enable it

            migrated = migrated | replaceString(configLines[i], ";FixedExposure = true", ";FixedExposure = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";FixedExposure", "FixedExposure"); // Enable it
        }

        if (section == "[Alignment]") {
            migrated = migrated | replaceString(configLines[i], ";InitialMirror = true", ";InitialMirror = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";InitialMirror", "InitialMirror"); // Enable it

            migrated = migrated | replaceString(configLines[i], ";FlipImageSize = true", ";FlipImageSize = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";FlipImageSize", "FlipImageSize"); // Enable it
        }

        if (section == "[Digits]") {
            migrated = migrated | replaceString(configLines[i], "LogImageLocation", "ROIImagesLocation");
            migrated = migrated | replaceString(configLines[i], "LogfileRetentionInDays", "ROIImagesRetention");
        }

        if (section == "[Analog]") {
            migrated = migrated | replaceString(configLines[i], "LogImageLocation", "ROIImagesLocation");
            migrated = migrated | replaceString(configLines[i], "LogfileRetentionInDays", "ROIImagesRetention");
            migrated = migrated | replaceString(configLines[i], "ExtendedResolution", ";UNUSED_PARAMETER"); // This parameter is no longer used
        }

        if (section == "[PostProcessing]") {
            migrated = migrated | replaceString(configLines[i], ";PreValueUse = true", ";PreValueUse = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";PreValueUse", "PreValueUse"); // Enable it

            /* AllowNegativeRates has a <NUMBER> as prefix! */
            if (isInString(configLines[i], "AllowNegativeRates") && isInString(configLines[i], ";")) { // It is the parameter "AllowNegativeRates" and it is commented out
                migrated = migrated | replaceString(configLines[i], "true", "false"); // Set it to its default value
                migrated = migrated | replaceString(configLines[i], ";", ""); // Enable it
            }

            /* IgnoreLeadingNaN has a <NUMBER> as prefix! */
            if (isInString(configLines[i], "IgnoreLeadingNaN") && isInString(configLines[i], ";")) { // It is the parameter "IgnoreLeadingNaN" and it is commented out
                migrated = migrated | replaceString(configLines[i], "true", "false"); // Set it to its default value
                migrated = migrated | replaceString(configLines[i], ";", ""); // Enable it
            }

            /* ExtendedResolution has a <NUMBER> as prefix! */
            if (isInString(configLines[i], "ExtendedResolution") && isInString(configLines[i], ";")) { // It is the parameter "ExtendedResolution" and it is commented out
                migrated = migrated | replaceString(configLines[i], "true", "false"); // Set it to its default value
                migrated = migrated | replaceString(configLines[i], ";", ""); // Enable it
            }

            migrated = migrated | replaceString(configLines[i], ";ErrorMessage = true", ";ErrorMessage = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";ErrorMessage", "ErrorMessage"); // Enable it

            migrated = migrated | replaceString(configLines[i], ";CheckDigitIncreaseConsistency = true", ";CheckDigitIncreaseConsistency = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";CheckDigitIncreaseConsistency", "CheckDigitIncreaseConsistency"); // Enable it
        }

        if (section == "[MQTT]") {
            migrated = migrated | replaceString(configLines[i], "SetRetainFlag", "RetainMessages"); // First rename it, enable it with its default value
            migrated = migrated | replaceString(configLines[i], ";RetainMessages = true", ";RetainMessages = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";RetainMessages", "RetainMessages"); // Enable it

            migrated = migrated | replaceString(configLines[i], ";HomeassistantDiscovery = true", ";HomeassistantDiscovery = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";HomeassistantDiscovery", "HomeassistantDiscovery"); // Enable it

            if (configLines[i].rfind("Topic", 0) != std::string::npos)  // only if string starts with "Topic" (Was the naming in very old version)
            {
                migrated = migrated | replaceString(configLines[i], "Topic", "MainTopic");
            }
        }

        if (section == "[InfluxDB]") {
            /* Fieldname has a <NUMBER> as prefix! */
            if (isInString(configLines[i], "Fieldname")) { // It is the parameter "Fieldname"
                migrated = migrated | replaceString(configLines[i], "Fieldname", "Field"); // Rename it to Field
                migrated = migrated | replaceString(configLines[i], ";", ""); // Enable it
            }
        }

        if (section == "[InfluxDBv2]") {
            /* Fieldname has a <NUMBER> as prefix! */
            if (isInString(configLines[i], "Fieldname")) { // It is the parameter "Fieldname"
                migrated = migrated | replaceString(configLines[i], "Fieldname", "Field"); // Rename it to Field
                migrated = migrated | replaceString(configLines[i], ";", ""); // Enable it
            }
        }

        if (section == "[GPIO]") {

        }

        if (section == "[DataLogging]") {
            migrated = migrated | replaceString(configLines[i], "DataLogRetentionInDays", "DataFilesRetention");
            /* DataLogActive is true by default! */
            migrated = migrated | replaceString(configLines[i], ";DataLogActive = false", ";DataLogActive = true"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";DataLogActive", "DataLogActive"); // Enable it
        }

        if (section == "[AutoTimer]") {
            migrated = migrated | replaceString(configLines[i], "Intervall", "Interval");
            migrated = migrated | replaceString(configLines[i], ";AutoStart = true", ";AutoStart = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";AutoStart", "AutoStart"); // Enable it

        }

        if (section == "[Debug]") {
            migrated = migrated | replaceString(configLines[i], "Logfile ", "LogLevel "); // Whitespace needed so it does not match `LogfileRetentionInDays`
            /* LogLevel (resp. LogFile) was originally a boolean, but we switched it to an int
             * For both cases (true/false), we set it to level 2 (WARNING) */
            migrated = migrated | replaceString(configLines[i], "LogLevel = true", "LogLevel = 2");
            migrated = migrated | replaceString(configLines[i], "LogLevel = false", "LogLevel = 2");
            migrated = migrated | replaceString(configLines[i], "LogfileRetentionInDays", "LogfilesRetention");
        }

        if (section == "[System]") {
            migrated = migrated | replaceString(configLines[i], "RSSIThreashold", "RSSIThreshold");
            migrated = migrated | replaceString(configLines[i], "AutoAdjustSummertime", ";UNUSED_PARAMETER"); // This parameter is no longer used

            migrated = migrated | replaceString(configLines[i], ";SetupMode = true", ";SetupMode = false"); // Set it to its default value
            migrated = migrated | replaceString(configLines[i], ";SetupMode", "SetupMode"); // Enable it
        }
    }

    if (migrated) { // At least one replacement happened
        if (! RenameFile(CONFIG_FILE, CONFIG_FILE_BACKUP)) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to create backup of Config file!");
            return;
        }

        FILE* pfile = fopen(CONFIG_FILE, "w");        
        for (int i = 0; i < configLines.size(); i++) {
            fwrite(configLines[i].c_str() , configLines[i].length(), 1, pfile);
            fwrite("\n" , 1, 1, pfile);
        }
        fclose(pfile);
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Config file migrated. Saved backup to " + string(CONFIG_FILE_BACKUP));
    }
}


std::vector<std::string> splitString(const std::string& str) {
    std::vector<std::string> tokens;
 
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, '\n')) {
        tokens.push_back(token);
    }
 
    return tokens;
}



/*bool replace_all(std::string& s, std::string const& toReplace, std::string const& replaceWith) {
    std::string buf;
    std::size_t pos = 0;
    std::size_t prevPos;
    bool found = false;

    // Reserves rough estimate of final size of string.
    buf.reserve(s.size());

    while (true) {
        prevPos = pos;
        pos = s.find(toReplace, pos);
        if (pos == std::string::npos) {
            break;
        }
        found = true;
        buf.append(s, prevPos, pos - prevPos);
        buf += replaceWith;
        pos += toReplace.size();
    }

    buf.append(s, prevPos, s.size() - prevPos);
    s.swap(buf);

    return found;
}*/


bool setCpuFrequency(void) {
    ConfigFile configFile = ConfigFile(CONFIG_FILE); 
    string cpuFrequency = "160";
    esp_pm_config_esp32_t  pm_config; 

    if (!configFile.ConfigFileExists()){
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "No ConfigFile defined - exit setCpuFrequency()!");
        return false;
    }

    std::vector<std::string> splitted;
    std::string line = "";
    bool disabledLine = false;
    bool eof = false;


    /* Load config from config file */
    while ((!configFile.GetNextParagraph(line, disabledLine, eof) || 
            (line.compare("[System]") != 0)) && !eof) {}
    if (eof) {
        return false;
    }

    if (disabledLine) {
        return false;
    }

    while (configFile.getNextLine(&line, disabledLine, eof) && 
            !configFile.isNewParagraph(line)) {
        splitted = ZerlegeZeile(line);

        if (toUpper(splitted[0]) == "CPUFREQUENCY") {
            cpuFrequency = splitted[1];
            break;
        }
    }

    if (esp_pm_get_configuration(&pm_config) != ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to read CPU Frequency!");
        return false;
    }

    if (cpuFrequency == "160") { // 160 is the default
        // No change needed
    }
    else if (cpuFrequency == "240") {
        pm_config.max_freq_mhz = 240;
        pm_config.min_freq_mhz = pm_config.max_freq_mhz;
        if (esp_pm_configure(&pm_config) != ESP_OK) {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to set new CPU frequency!");
            return false;
        }
    }
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Unknown CPU frequency: " + cpuFrequency + "! "
                "It must be 160 or 240!");
        return false;
    }

    if (esp_pm_get_configuration(&pm_config) == ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, string("CPU frequency: ") + to_string(pm_config.max_freq_mhz) + " MHz");
    }

    return true;
}