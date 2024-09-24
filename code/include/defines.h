#pragma once
#ifndef defines_h
#define defines_h

/////////////////////////////////////////////
////          Global definitions         ////
/////////////////////////////////////////////

    //********* debug options :  *************

    //can be set in platformio with -D OPTION_TO_ACTIVATE

    //#define DEBUG_DETAIL_ON 
    //#define DEBUG_DISABLE_BROWNOUT_DETECTOR
    //#define DEBUG_ENABLE_SYSINFO
    //#define DEBUG_ENABLE_PERFMON
    //#define DEBUG_HIMEM_MEMORY_CHECK
    // need [env:esp32cam-dev-himem]
    //=> CONFIG_SPIRAM_BANKSWITCH_ENABLE=y
    //=> CONFIG_SPIRAM_BANKSWITCH_RESERVE=4


    // use himem //https://github.com/jomjol/AI-on-the-edge-device/issues/1842
    #if (CONFIG_SPIRAM_BANKSWITCH_ENABLE)
        #define USE_HIMEM_IF_AVAILABLE 1
    #endif

    /* Uncomment this to generate task list with stack sizes using the /heap handler
        PLEASE BE AWARE: The following CONFIG parameters have to to be set in 
        sdkconfig.defaults before use of this function is possible!!
        CONFIG_FREERTOS_USE_TRACE_FACILITY=1
        CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=y
        CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID=y
    */
    // server_tflite.cpp
    //#define TASK_ANALYSIS_ON


    //Memory leak tracing
    //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/heap_debug.html#heap-information
    //need CONFIG_HEAP_TRACING_STANDALONE=y or #define CONFIG_HEAP_TRACING_STANDALONE
    //all setup is predifined in [env:esp32cam-dev-task-analysis]
    //#define HEAP_TRACING_MAIN_WIFI || HEAP_TRACING_MAIN_START //enable heap tracing per function in main.cpp
    //all defines in [env:esp32cam-dev-task-analysis]
    //#define HEAP_TRACING_MAIN_WIFI
    //#define HEAP_TRACING_MAIN_START
    //#define HEAP_TRACING_CLASS_FLOW_CNN_GENERAL_DO_ALING_AND_CUT

    /* Uncomment this to keep the logfile open for appending.
    * If commented out, the logfile gets opened/closed for each log measage (old behaviour) */
    // ClassLogFile
    //#define KEEP_LOGFILE_OPEN_FOR_APPENDING

  //****************************************

    //compiler optimization for esp-tflite-micro
    #define XTENSA
    //#define CONFIG_IDF_TARGET_ARCH_XTENSA     //not needed with platformio/espressif32 @ 5.2.0


    //ClassControllCamera
    #define CAM_LIVESTREAM_REFRESHRATE 500      // Camera livestream feature: Waiting time in milliseconds to refresh image
    // #define GRAYSCALE_AS_DEFAULT


    //server_GPIO
    #define __LEDGLOBAL


    //server_GPIO + server_file + SoftAP
    #define CONFIG_FILE "/sdcard/config/config.ini"
    #define CONFIG_FILE_BACKUP "/sdcard/config/config.bak"


    //interface_mqtt + read_wlanini
    #define __HIDE_PASSWORD


    //ClassFlowControll + Main + SoftAP
    #define WLAN_CONFIG_FILE "/sdcard/wlan.ini"


    //main
    #define __SD_USE_ONE_LINE_MODE__

    // server_file + Helper
     #define FILE_PATH_MAX (255) //Max length a file path can have on storage
    

    //server_file +(ota_page.html + upload_script.html)
    #define MAX_FILE_SIZE   (8000*1024) // 8 MB Max size of an individual file. Make sure this value is same as that set in upload_script.html and ota_page.html!
    #define MAX_FILE_SIZE_STR "8MB"
         
    #define LOGFILE_LAST_PART_BYTES 80 * 1024 // 80 kBytes  // Size of partial log file to return 

    #define SERVER_FILER_SCRATCH_BUFSIZE  4096 
    #define SERVER_HELPER_SCRATCH_BUFSIZE  4096
    #define SERVER_OTA_SCRATCH_BUFSIZE  1024 


    //server_file + server_help
    #define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)


    //server_ota
    #define HASH_LEN 32 // SHA-256 digest length
    #define OTA_URL_SIZE 256


    //ClassFlow + ClassFlowImage + server_tflite
    #define LOGFILE_TIME_FORMAT "%Y%m%d-%H%M%S"
    #define LOGFILE_TIME_FORMAT_DATE_EXTR substr(0, 8)
    #define LOGFILE_TIME_FORMAT_HOUR_EXTR substr(9, 2)


    //ClassFlowControll
    #define READOUT_TYPE_VALUE 0
    #define READOUT_TYPE_PREVALUE 1
    #define READOUT_TYPE_RAWVALUE 2
    #define READOUT_TYPE_ERROR 3


    //ClassFlowControll: Serve alg_roi.jpg from memory as JPG
    #define ALGROI_LOAD_FROM_MEM_AS_JPG // Load ALG_ROI.JPG as rendered JPG from RAM


    //ClassFlowMQTT
    #define LWT_TOPIC        "connection"
    #define LWT_CONNECTED    "connected"
    #define LWT_DISCONNECTED "connection lost"


    //ClassFlowPostProcessing
    #define PREVALUE_TIME_FORMAT_OUTPUT "%Y-%m-%dT%H:%M:%S%z"
    #define PREVALUE_TIME_FORMAT_INPUT "%d-%d-%dT%d:%d:%d"


    //CImageBasis
    #define HTTP_BUFFER_SENT 1024
    #define MAX_JPG_SIZE 128000

    //make_stb + stb_image_resize + stb_image_write + stb_image //do not work if not in make_stb.cpp
    //#define STB_IMAGE_IMPLEMENTATION
    //#define STB_IMAGE_WRITE_IMPLEMENTATION
    //#define STB_IMAGE_RESIZE_IMPLEMENTATION
    #define STBI_ONLY_JPEG // (save 2% of Flash, but breaks the alignment mark generation, see https://github.com/jomjol/AI-on-the-edge-device/issues/1721)


    //interface_influxdb
    #define MAX_HTTP_OUTPUT_BUFFER 2048


    //server_mqtt
    #define LWT_TOPIC        "connection"
    #define LWT_CONNECTED    "connected"
    #define LWT_DISCONNECTED "connection lost"


    // connect_wlan.cpp
    //******************************
    /* WIFI roaming functionalities 802.11k+v (uses ca. 6kB - 8kB internal RAM; if SCAN CACHE activated: + 1kB / beacon)
    PLEASE BE AWARE: The following CONFIG parameters have to to be set in 
    sdkconfig.defaults before use of this function is possible!!
    CONFIG_WPA_11KV_SUPPORT=y
    CONFIG_WPA_SCAN_CACHE=n
    CONFIG_WPA_MBO_SUPPORT=n
    CONFIG_WPA_11R_SUPPORT=n
    */
    //#define WLAN_USE_MESH_ROAMING   // 802.11v (BSS Transition Management) + 802.11k (Radio Resource Management) (ca. 6kB - 8kB internal RAM neccessary)
    //#define WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES  // Client can send query to AP requesting to roam (if RSSI lower than RSSI threshold)

    /* WIFI roaming only client triggered by scanning the channels after each round (only if RSSI < RSSIThreshold) and trigger a disconnect to switch AP */
    #define WLAN_USE_ROAMING_BY_SCANNING


    //ClassFlowCNNGeneral
    #define Analog_error 3
    #define AnalogToDigtalFehler 0.8
    #define Digital_Uncertainty 0.2
    #define DigitalBand 3
    #define Digital_Transition_Range_Predecessor 2
    #define Digital_Transition_Area_Predecessor 0.7 // 9.3 - 0.7
    #define Digital_Transition_Area_Forward 9.7 // Pre-run zero crossing only happens from approx. 9.7 onwards


    //#define DEBUG_DETAIL_ON 


/////////////////////////////////////////////
////      PSRAM Allocations              ////
/////////////////////////////////////////////
#define MAX_MODEL_SIZE            (unsigned int)(1.3 * 1024 * 1024) // Space for the currently largest model (1.1 MB) + some spare
#define TENSOR_ARENA_SIZE         800 * 1024 // Space for the Tensor Arena, (819200 Bytes)
#define IMAGE_SIZE                640 * 480 * 3 // Space for a extracted image (921600 Bytes)
/////////////////////////////////////////////
////      Conditionnal definitions       ////
/////////////////////////////////////////////


// ******* Board type   
#if defined(BOARD_WROVER_KIT) // WROVER-KIT PIN Map
	// SD card (operated with SDMMC peripheral)
	//-------------------------------------------------
	#define GPIO_SDCARD_CLK GPIO_NUM_14
	#define GPIO_SDCARD_CMD GPIO_NUM_15
	#define GPIO_SDCARD_D0  GPIO_NUM_2
	#ifndef __SD_USE_ONE_LINE_MODE__
		#define GPIO_SDCARD_D1 GPIO_NUM_4
		#define GPIO_SDCARD_D2 GPIO_NUM_12
		#define GPIO_SDCARD_D3 GPIO_NUM_13
	#else
		#define GPIO_SDCARD_D1 GPIO_NUM_NC
		#define GPIO_SDCARD_D2 GPIO_NUM_NC
		#define GPIO_SDCARD_D3 GPIO_NUM_13
	#endif

    #define CAM_PIN_PWDN     GPIO_NUM_NC  //power down is not used
    #define CAM_PIN_RESET    GPIO_NUM_NC  //software reset will be performed
    #define CAM_PIN_XCLK     GPIO_NUM_21
    #define CAM_PIN_SIOD     GPIO_NUM_26
    #define CAM_PIN_SIOC     GPIO_NUM_27

    #define CAM_PIN_D7       GPIO_NUM_35
    #define CAM_PIN_D6       GPIO_NUM_34
    #define CAM_PIN_D5       GPIO_NUM_39
    #define CAM_PIN_D4       GPIO_NUM_36
    #define CAM_PIN_D3       GPIO_NUM_19
    #define CAM_PIN_D2       GPIO_NUM_18
    #define CAM_PIN_D1       GPIO_NUM_5
    #define CAM_PIN_D0       GPIO_NUM_4
    #define CAM_PIN_VSYNC    GPIO_NUM_25
    #define CAM_PIN_HREF     GPIO_NUM_23
    #define CAM_PIN_PCLK     GPIO_NUM_22

    //Statusled + ClassControllCamera
    #define BLINK_GPIO GPIO_NUM_33              // PIN for red board LED, On the board the LED is on the IO2, but it is used for the SD
	
    //ClassControllCamera
    #define FLASH_GPIO GPIO_NUM_12              // PIN for flashlight LED
    #define USE_PWM_LEDFLASH                    // if __LEDGLOBAL is defined, a global variable is used for LED control, otherwise locally and each time a new

#elif defined(BOARD_M5STACK_PSRAM) // M5STACK PSRAM PIN Map
    #define CAM_PIN_PWDN     GPIO_NUM_NC
    #define CAM_PIN_RESET    GPIO_NUM_15
    #define CAM_PIN_XCLK     GPIO_NUM_27
    #define CAM_PIN_SIOD     GPIO_NUM_25
    #define CAM_PIN_SIOC     GPIO_NUM_23

    #define CAM_PIN_D7       GPIO_NUM_19
    #define CAM_PIN_D6       GPIO_NUM_36
    #define CAM_PIN_D5       GPIO_NUM_18
    #define CAM_PIN_D4       GPIO_NUM_39
    #define CAM_PIN_D3       GPIO_NUM_5
    #define CAM_PIN_D2       GPIO_NUM_34
    #define CAM_PIN_D1       GPIO_NUM_35
    #define CAM_PIN_D0       GPIO_NUM_32
    #define CAM_PIN_VSYNC    GPIO_NUM_22
    #define CAM_PIN_HREF     GPIO_NUM_26
    #define CAM_PIN_PCLK     GPIO_NUM_21

    //Statusled + ClassControllCamera
    #define BLINK_GPIO GPIO_NUM_33              // PIN for red board LED
	
    //ClassControllCamera
    #define FLASH_GPIO GPIO_NUM_4               // PIN for flashlight LED
    #define USE_PWM_LEDFLASH                    // if __LEDGLOBAL is defined, a global variable is used for LED control, otherwise locally and each time a new


#elif defined(BOARD_ESP32CAM_AITHINKER) // ESP32Cam (AiThinker) PIN Map
	// SD card (operated with SDMMC peripheral)
	//-------------------------------------------------
	#define GPIO_SDCARD_CLK GPIO_NUM_14
	#define GPIO_SDCARD_CMD GPIO_NUM_15
	#define GPIO_SDCARD_D0  GPIO_NUM_2
	#ifndef __SD_USE_ONE_LINE_MODE__
		#define GPIO_SDCARD_D1 GPIO_NUM_4
		#define GPIO_SDCARD_D2 GPIO_NUM_12
		#define GPIO_SDCARD_D3 GPIO_NUM_13
	#else
		#define GPIO_SDCARD_D1 GPIO_NUM_NC
		#define GPIO_SDCARD_D2 GPIO_NUM_NC
		#define GPIO_SDCARD_D3 GPIO_NUM_13
	#endif

    #define CAM_PIN_PWDN     GPIO_NUM_32
    #define CAM_PIN_RESET    GPIO_NUM_NC  //software reset will be performed
    #define CAM_PIN_XCLK     GPIO_NUM_0
    #define CAM_PIN_SIOD     GPIO_NUM_26
    #define CAM_PIN_SIOC     GPIO_NUM_27

    #define CAM_PIN_D7       GPIO_NUM_35
    #define CAM_PIN_D6       GPIO_NUM_34
    #define CAM_PIN_D5       GPIO_NUM_39
    #define CAM_PIN_D4       GPIO_NUM_36
    #define CAM_PIN_D3       GPIO_NUM_21
    #define CAM_PIN_D2       GPIO_NUM_19
    #define CAM_PIN_D1       GPIO_NUM_18
    #define CAM_PIN_D0       GPIO_NUM_5
    #define CAM_PIN_VSYNC    GPIO_NUM_25
    #define CAM_PIN_HREF     GPIO_NUM_23
    #define CAM_PIN_PCLK     GPIO_NUM_22

    //Statusled + ClassControllCamera
    #define BLINK_GPIO GPIO_NUM_33              // PIN for red board LED
	
    //ClassControllCamera
    #define FLASH_GPIO GPIO_NUM_4               // PIN for flashlight LED
    #define USE_PWM_LEDFLASH                    // if __LEDGLOBAL is defined, a global variable is used for LED control, otherwise locally and each time a new

#else
    #error "Board not selected"
#endif  //Board PIN Map


// ******* LED definition
#ifdef USE_PWM_LEDFLASH
    //// PWM für Flash-LED
    #define LEDC_TIMER              LEDC_TIMER_1 // LEDC_TIMER_0
    #define LEDC_MODE               LEDC_LOW_SPEED_MODE
    #define LEDC_OUTPUT_IO          FLASH_GPIO // Define the output GPIO
    #define LEDC_CHANNEL            LEDC_CHANNEL_1
    #define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
    //#define LEDC_DUTY               (195) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
    #define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#endif //USE_PWM_LEDFLASH


//softAP
#ifdef ENABLE_SOFTAP
    #define EXAMPLE_ESP_WIFI_SSID      "AI-on-the-Edge"
    #define EXAMPLE_ESP_WIFI_PASS      ""
    #define EXAMPLE_ESP_WIFI_CHANNEL   11
    #define EXAMPLE_MAX_STA_CONN       1
#endif // ENABLE_SOFTAP

#endif // ifndef defines_h
