#pragma once
#ifndef defines_h
#define defines_h

/////////////////////////////////////////////
////          Global definitions         ////
/////////////////////////////////////////////

//********* debug options :  *************

// can be set in platformio with -D OPTION_TO_ACTIVATE

// #define DEBUG_DETAIL_ON
// #define DEBUG_DISABLE_BROWNOUT_DETECTOR
// #define DEBUG_ENABLE_SYSINFO
// #define DEBUG_ENABLE_PERFMON


// Memory leak tracing
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/heap_debug.html#heap-information
// need CONFIG_HEAP_TRACING_STANDALONE=y or #define CONFIG_HEAP_TRACING_STANDALONE
// all setup is predifined in [env:esp32cam-dev-task-analysis]
// #define HEAP_TRACING_MAIN_NETWORK || HEAP_TRACING_MAIN_START //enable heap tracing per function in main.cpp
// all defines in [env:esp32cam-dev-task-analysis]
// #define HEAP_TRACING_MAIN_NETWORK
// #define HEAP_TRACING_MAIN_START
// #define HEAP_TRACING_CLASS_FLOW_CNN_GENERAL_DO_ALING_AND_CUT

/* Uncomment this to keep the logfile open for appending.
 * If commented out, the logfile gets opened/closed for each log measage (old behaviour) */
// ClassLogFile
// #define KEEP_LOGFILE_OPEN_FOR_APPENDING

//****************************************

// compiler optimization for esp-tflite-micro
#define XTENSA
// #define CONFIG_IDF_TARGET_ARCH_XTENSA     //not needed with platformio/espressif32 @ 5.2.0


// ClassControllCamera
#define CAM_LIVESTREAM_REFRESHRATE 500 // Camera livestream feature: Waiting time in milliseconds to refresh image
// #define GRAYSCALE_AS_DEFAULT

// littlefs
#define WWW_PARTITION_PATH  "/www"
#define WWW_PARTITION_LABEL "www"

// server_GPIO + server_file + connect_wifi_ap
#define CONFIG_FILE        "/sdcard/config/config.ini"
#define CONFIG_FILE_BACKUP "/sdcard/config/config.bak"


// interface_mqtt + read_network_ini
#define __HIDE_PASSWORD
#define STRING_ENCRYPTED_LABEL "**##**"

// ClassFlowControll + Main + connect_wifi_ap
#define ENABLE_SOFTAP

#define WLAN_CONFIG_FILE "/sdcard/wlan.ini"
#define NETWORK_CONFIG_FILE "/sdcard/network.ini"


// main
#define __SD_USE_ONE_LINE_MODE__

// server_file + Helper
#define FILE_PATH_MAX (255) // Max length a file path can have on storage


// server_file +(ota_page.html + upload_script.html)
#define MAX_FILE_SIZE     (8000 * 1024) // 8 MB Max size of an individual file. Make sure this value is same as that set in upload_script.html and ota_page.html!
#define MAX_FILE_SIZE_STR "8MB"

#define LOGFILE_LAST_PART_BYTES 80 * 1024 // 80 kBytes  // Size of partial log file to return

#define SERVER_FILER_SCRATCH_BUFSIZE  4096
#define SERVER_HELPER_SCRATCH_BUFSIZE 4096
#define SERVER_OTA_SCRATCH_BUFSIZE    1024


// server_file + server_help
#define IS_FILE_EXT(filename, ext) (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)


// server_ota
#define HASH_LEN     32 // SHA-256 digest length
#define OTA_URL_SIZE 256


// ClassFlow + ClassFlowImage + server_tflite
#define LOGFILE_TIME_FORMAT           "%Y%m%d-%H%M%S"
#define LOGFILE_TIME_FORMAT_DATE_EXTR substr(0, 8)
#define LOGFILE_TIME_FORMAT_HOUR_EXTR substr(9, 2)


// ClassFlowControll
#define READOUT_TYPE_VALUE    0
#define READOUT_TYPE_PREVALUE 1
#define READOUT_TYPE_RAWVALUE 2
#define READOUT_TYPE_ERROR    3


// ClassFlowControll: Serve alg_roi.jpg from memory as JPG
#define ALGROI_LOAD_FROM_MEM_AS_JPG // Load ALG_ROI.JPG as rendered JPG from RAM


// ClassFlowMQTT
#define LWT_TOPIC        "connection"
#define LWT_CONNECTED    "connected"
#define LWT_DISCONNECTED "connection lost"


// ClassFlowPostProcessing
#define PREVALUE_TIME_FORMAT_OUTPUT "%Y-%m-%dT%H:%M:%S%z"
#define PREVALUE_TIME_FORMAT_INPUT  "%d-%d-%dT%d:%d:%d"


// CImageBasis
#define HTTP_BUFFER_SENT 1024
#define MAX_JPG_SIZE     128000

// make_stb + stb_image_resize + stb_image_write + stb_image //do not work if not in make_stb.cpp
// #define STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #define STB_IMAGE_RESIZE_IMPLEMENTATION
// #define STBI_ONLY_JPEG // (save 2% of Flash, but breaks the alignment mark generation, see
// https://github.com/jomjol/AI-on-the-edge-device/issues/1721)


// interface_influxdb
#define MAX_HTTP_OUTPUT_BUFFER 2048


// connect_wifi_sta.cpp
//******************************
/* WIFI roaming functionalities 802.11k+v (uses ca. 6kB - 8kB internal RAM; if SCAN CACHE activated: + 1kB / beacon)
PLEASE BE AWARE: The following CONFIG parameters have to to be set in
sdkconfig.defaults before use of this function is possible!!
CONFIG_WPA_11KV_SUPPORT=y
CONFIG_WPA_SCAN_CACHE=n
CONFIG_WPA_MBO_SUPPORT=n
CONFIG_WPA_11R_SUPPORT=n
*/
// #define WLAN_USE_MESH_ROAMING   // 802.11v (BSS Transition Management) + 802.11k (Radio Resource Management) (ca. 6kB - 8kB internal RAM
// neccessary) #define WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES  // Client can send query to AP requesting to roam (if RSSI
// lower than RSSI threshold)

/* WIFI roaming only client triggered by scanning the channels after each round (only if RSSI < RSSIThreshold) and trigger a disconnect to
 * switch AP */
#define WLAN_USE_ROAMING_BY_SCANNING


// ClassFlowCNNGeneral
#define Analog_error 3

#define AnalogToDigtalFehler               0.8
#define Digit_Uncertainty                  0.2
#define DigitBand                          3
#define Digit_Transition_Range_Predecessor 2
#define Digit_Transition_Area_Predecessor  0.7 // 9.3 - 0.7
#define Digit_Transition_Area_Forward      9.7 // Pre-run zero crossing only happens from approx. 9.7 onwards

// #define DEBUG_DETAIL_ON


/////////////////////////////////////////////
////      PSRAM Allocations              ////
/////////////////////////////////////////////
#define MAX_MODEL_SIZE    (unsigned int)(1.3 * 1024 * 1024) // Space for the currently largest model (1.1 MB) + some spare
#define TENSOR_ARENA_SIZE 800 * 1024                        // Space for the Tensor Arena, (819200 Bytes)
#define IMAGE_SIZE        640 * 480 * 3                     // Space for a extracted image (921600 Bytes)
/////////////////////////////////////////////
////      Conditionnal definitions       ////
/////////////////////////////////////////////


// ******* Board type
#if defined(BOARD_ESP32CAM_AITHINKER) // ESP32Cam (AiThinker) PIN Map
#define BoardType "ESP32CAM"

// Uart
//-------------------------------------------------
#define UART_PORT_NUM     UART_NUM_0
#define UART_TX_GPIO_NUM  GPIO_NUM_1
#define UART_RX_GPIO_NUM  GPIO_NUM_3
#define UART_RTS_GPIO_NUM GPIO_NUM_NC
#define UART_CTS_GPIO_NUM GPIO_NUM_NC
#define UART_BUFFER_SIZE  2048

// SD card (operated with SDMMC peripheral)
//-------------------------------------------------
#define GPIO_SDCARD_CLK GPIO_NUM_14
#define GPIO_SDCARD_CMD GPIO_NUM_15
#define GPIO_SDCARD_D0  GPIO_NUM_2
#ifndef __SD_USE_ONE_LINE_MODE__
#error "Board not suppotred SD_FOUR_LINE_MODE"
// #define GPIO_SDCARD_D1       GPIO_NUM_4
// #define GPIO_SDCARD_D2       GPIO_NUM_12
// #define GPIO_SDCARD_D3       GPIO_NUM_13
#else
#define GPIO_SDCARD_D1 GPIO_NUM_NC
#define GPIO_SDCARD_D2 GPIO_NUM_NC
#define GPIO_SDCARD_D3 GPIO_NUM_13
#endif

// Camera (suppotred OV2640 or OV5640)
//-------------------------------------------------
#define CAM_PIN_PWDN  GPIO_NUM_32
#define CAM_PIN_RESET GPIO_NUM_NC // software reset will be performed

#define CAM_PIN_XCLK GPIO_NUM_0
#define CAM_PIN_SIOD GPIO_NUM_26
#define CAM_PIN_SIOC GPIO_NUM_27

#define CAM_PIN_D7 GPIO_NUM_35
#define CAM_PIN_D6 GPIO_NUM_34
#define CAM_PIN_D5 GPIO_NUM_39
#define CAM_PIN_D4 GPIO_NUM_36
#define CAM_PIN_D3 GPIO_NUM_21
#define CAM_PIN_D2 GPIO_NUM_19
#define CAM_PIN_D1 GPIO_NUM_18
#define CAM_PIN_D0 GPIO_NUM_5

#define CAM_PIN_VSYNC GPIO_NUM_25
#define CAM_PIN_HREF  GPIO_NUM_23
#define CAM_PIN_PCLK  GPIO_NUM_22

#define CAM_XCLK_TIMER   LEDC_TIMER_0
#define CAM_XCLK_CHANNEL LEDC_CHANNEL_0

// GpioHandler
#define GPIO_IO1 GPIO_NUM_4
#define GPIO_IO2 GPIO_NUM_12
#define GPIO_IO3 GPIO_NUM_13
#define GPIO_IO4 GPIO_NUM_NC

// Statusled + ClassControllCamera
#define BLINK_GPIO GPIO_NUM_33 // PIN for red board LED
#define BLINK_GPIO_INVERT

// ClassControllCamera
#define FLASH_GPIO GPIO_NUM_4 // PIN for flashlight LED
#define FLASH_MODE GPIO_PIN_MODE_OUTPUT_PWM


#elif defined(BOARD_WROVER_KIT_ESP32) // WROVER-KIT PIN Map
#define BoardType         "WROVER-KIT"

// Uart
//-------------------------------------------------
#define UART_PORT_NUM     UART_NUM_0
#define UART_TX_GPIO_NUM  GPIO_NUM_1
#define UART_RX_GPIO_NUM  GPIO_NUM_3
#define UART_RTS_GPIO_NUM GPIO_NUM_NC
#define UART_CTS_GPIO_NUM GPIO_NUM_NC
#define UART_BUFFER_SIZE  2048

// SD card (operated with SDMMC peripheral)
//-------------------------------------------------
#define GPIO_SDCARD_CLK   GPIO_NUM_14
#define GPIO_SDCARD_CMD   GPIO_NUM_15
#define GPIO_SDCARD_D0    GPIO_NUM_2
#ifndef __SD_USE_ONE_LINE_MODE__
#error "Board not suppotred SD_FOUR_LINE_MODE"
// #define GPIO_SDCARD_D1       GPIO_NUM_4
// #define GPIO_SDCARD_D2       GPIO_NUM_12
// #define GPIO_SDCARD_D3       GPIO_NUM_13
#else
#define GPIO_SDCARD_D1 GPIO_NUM_NC
#define GPIO_SDCARD_D2 GPIO_NUM_NC
#define GPIO_SDCARD_D3 GPIO_NUM_13
#endif

#define CAM_PIN_PWDN  GPIO_NUM_NC // power down is not used
#define CAM_PIN_RESET GPIO_NUM_NC // software reset will be performed

#define CAM_PIN_XCLK GPIO_NUM_21
#define CAM_PIN_SIOD GPIO_NUM_26
#define CAM_PIN_SIOC GPIO_NUM_27

#define CAM_PIN_D7 GPIO_NUM_35
#define CAM_PIN_D6 GPIO_NUM_34
#define CAM_PIN_D5 GPIO_NUM_39
#define CAM_PIN_D4 GPIO_NUM_36
#define CAM_PIN_D3 GPIO_NUM_19
#define CAM_PIN_D2 GPIO_NUM_18
#define CAM_PIN_D1 GPIO_NUM_5
#define CAM_PIN_D0 GPIO_NUM_4

#define CAM_PIN_VSYNC GPIO_NUM_25
#define CAM_PIN_HREF  GPIO_NUM_23
#define CAM_PIN_PCLK  GPIO_NUM_22

#define CAM_XCLK_TIMER   LEDC_TIMER_0
#define CAM_XCLK_CHANNEL LEDC_CHANNEL_0

// GpioHandler
#define GPIO_IO1         GPIO_NUM_12
#define GPIO_IO2         GPIO_NUM_13
#define GPIO_IO3         GPIO_NUM_33
#define GPIO_IO4         GPIO_NUM_NC

// Statusled + ClassControllCamera
#define BLINK_GPIO       GPIO_NUM_NC // PIN for red board LED, On the board the LED is on the IO2, but it is used for the SD

// ClassControllCamera
#define FLASH_GPIO       GPIO_NUM_12 // PIN for flashlight LED
#define FLASH_MODE       GPIO_PIN_MODE_OUTPUT_PWM


#elif defined(BOARD_FREENOVE_ESP32S3)
#define BoardType         "ESP32-S3-CAM"

// Uart
//-------------------------------------------------
#define UART_PORT_NUM     UART_NUM_0
#define UART_TX_GPIO_NUM  GPIO_NUM_43
#define UART_RX_GPIO_NUM  GPIO_NUM_44
#define UART_RTS_GPIO_NUM GPIO_NUM_NC
#define UART_CTS_GPIO_NUM GPIO_NUM_NC
#define UART_BUFFER_SIZE  2048

// SD card (operated with SDMMC peripheral)
//-------------------------------------------------
#define GPIO_SDCARD_CLK   GPIO_NUM_39 // MTCK, GPIO39, CLK_OUT3, SUBSPICS1
#define GPIO_SDCARD_CMD   GPIO_NUM_38 // GPIO38, FSPIWP, SUBSPIWP
#define GPIO_SDCARD_D0    GPIO_NUM_40 // MTDO, GPIO40, CLK_OUT2

#ifndef __SD_USE_ONE_LINE_MODE__
#define GPIO_SDCARD_D1 GPIO_NUM_41 // MTDI, GPIO41, CLK_OUT1
#define GPIO_SDCARD_D2 GPIO_NUM_14 // RTC_GPIO14, GPIO14, TOUCH14, ADC2_CH3, FSPIWP, FSPIDQS, SUBSPIWP
#define GPIO_SDCARD_D3 GPIO_NUM_47 // SPICLK_P_DIFF, GPIO47, SUBSPICLK_P_DIFF
#else
#define GPIO_SDCARD_D1 GPIO_NUM_NC
#define GPIO_SDCARD_D2 GPIO_NUM_NC
#define GPIO_SDCARD_D3 GPIO_NUM_47
#endif

// Camera (suppotred OV2640 or OV5640)
//-------------------------------------------------
#define CAM_PIN_PWDN  GPIO_NUM_NC
#define CAM_PIN_RESET GPIO_NUM_NC // software reset will be performed

#define CAM_PIN_XCLK GPIO_NUM_15
#define CAM_PIN_SIOD GPIO_NUM_4
#define CAM_PIN_SIOC GPIO_NUM_5

#define CAM_PIN_D0 GPIO_NUM_11 // Y2_GPIO_NUM
#define CAM_PIN_D1 GPIO_NUM_9  // Y3_GPIO_NUM
#define CAM_PIN_D2 GPIO_NUM_8  // Y4_GPIO_NUM
#define CAM_PIN_D3 GPIO_NUM_10 // Y5_GPIO_NUM
#define CAM_PIN_D4 GPIO_NUM_12 // Y6_GPIO_NUM
#define CAM_PIN_D5 GPIO_NUM_18 // Y7_GPIO_NUM
#define CAM_PIN_D6 GPIO_NUM_17 // Y8_GPIO_NUM
#define CAM_PIN_D7 GPIO_NUM_16 // Y9_GPIO_NUM

#define CAM_PIN_VSYNC GPIO_NUM_6
#define CAM_PIN_HREF  GPIO_NUM_7
#define CAM_PIN_PCLK  GPIO_NUM_13

#define CAM_XCLK_TIMER          LEDC_TIMER_0
#define CAM_XCLK_CHANNEL        LEDC_CHANNEL_0

// GpioHandler
#define GPIO_IO1                GPIO_NUM_2
#define GPIO_IO2                GPIO_NUM_21
#define GPIO_IO3                GPIO_NUM_48
#define GPIO_IO4                GPIO_NUM_NC

// Statusled + ClassControllCamera
#define BLINK_GPIO              GPIO_NUM_2  // PIN for red board LED
// #define BLINK_GPIO_INVERT

// ClassControllCamera
#define FLASH_GPIO              GPIO_NUM_48 // PIN for flashlight LED
#define FLASH_MODE              GPIO_PIN_MODE_WS281X
#define FLASH_SMARTLED_TYPE     LED_WS2812
#define FLASH_SMARTLED_COLOR    Rgb{127, 127, 127}
#define FLASH_SMARTLED_QUANTITY 1

#elif defined(BOARD_ESP32_S3_ETH_V1)
#define BoardType         "ESP32-S3-ETH"

// Uart
//-------------------------------------------------
#define UART_PORT_NUM     UART_NUM_0
#define UART_TX_GPIO_NUM  GPIO_NUM_43
#define UART_RX_GPIO_NUM  GPIO_NUM_44
#define UART_RTS_GPIO_NUM GPIO_NUM_NC
#define UART_CTS_GPIO_NUM GPIO_NUM_NC
#define UART_BUFFER_SIZE  2048

// SD card (operated with SDMMC peripheral)
//-------------------------------------------------
#define GPIO_SDCARD_CLK   GPIO_NUM_40
#define GPIO_SDCARD_CMD   GPIO_NUM_42
#define GPIO_SDCARD_D0    GPIO_NUM_41

#ifndef __SD_USE_ONE_LINE_MODE__
#error "Board not suppotred SD_FOUR_LINE_MODE"
#else
#define GPIO_SDCARD_D1 GPIO_NUM_NC
#define GPIO_SDCARD_D2 GPIO_NUM_NC
#define GPIO_SDCARD_D3 GPIO_NUM_3
#endif

// SPI_ETHERNET_W5500
//-------------------------------------------------
#define PER_ENABLE GPIO_NUM_46
#define ETH_SPI_EN GPIO_NUM_45

#define ETH_SPI_NUM 1

#define ETH_SPI_HOST      SPI2_HOST
#define ETH_SPI_CLOCK_MHZ 20 // SPI clock speed (MHz) (range 5 - 80, default 16)

#define ETH_SPI_POLLING0_MS 10                    // Set SPI Ethernet module polling period (default 10)
#define ETH_SPI_PHY_ADDR0   ESP_ETH_PHY_ADDR_AUTO // PHY address, set -1 to enable PHY address detection at initialization stage

#define ETH_SPI_CS0_GPIO      GPIO_NUM_39
#define ETH_SPI_INT0_GPIO     GPIO_NUM_38
#define ETH_SPI_PHY_RST0_GPIO GPIO_NUM_NC

#define ETH_SPI_MISO_GPIO GPIO_NUM_14
#define ETH_SPI_MOSI_GPIO GPIO_NUM_1
#define ETH_SPI_SCLK_GPIO GPIO_NUM_21

// Camera (suppotred OV2640 or OV5640)
//-------------------------------------------------
#define CAM_PIN_PWDN      GPIO_NUM_NC
#define CAM_PIN_RESET     GPIO_NUM_NC // software reset will be performed

#define CAM_PIN_XCLK GPIO_NUM_15
#define CAM_PIN_SIOD GPIO_NUM_4
#define CAM_PIN_SIOC GPIO_NUM_5

#define CAM_PIN_D0 GPIO_NUM_11 // Y2_GPIO_NUM
#define CAM_PIN_D1 GPIO_NUM_9  // Y3_GPIO_NUM
#define CAM_PIN_D2 GPIO_NUM_8  // Y4_GPIO_NUM
#define CAM_PIN_D3 GPIO_NUM_10 // Y5_GPIO_NUM
#define CAM_PIN_D4 GPIO_NUM_12 // Y6_GPIO_NUM
#define CAM_PIN_D5 GPIO_NUM_18 // Y7_GPIO_NUM
#define CAM_PIN_D6 GPIO_NUM_17 // Y8_GPIO_NUM
#define CAM_PIN_D7 GPIO_NUM_16 // Y9_GPIO_NUM

#define CAM_PIN_VSYNC GPIO_NUM_6
#define CAM_PIN_HREF  GPIO_NUM_7
#define CAM_PIN_PCLK  GPIO_NUM_13

#define CAM_XCLK_TIMER          LEDC_TIMER_0
#define CAM_XCLK_CHANNEL        LEDC_CHANNEL_0

// GpioHandler
#define GPIO_IO1                GPIO_NUM_33
#define GPIO_IO2                GPIO_NUM_34
#define GPIO_IO3                GPIO_NUM_35
#define GPIO_IO4                GPIO_NUM_36

// Statusled + ClassControllCamera
#define BLINK_GPIO              GPIO_NUM_NC // PIN for red board LED

// ClassControllCamera
#define FLASH_GPIO              GPIO_NUM_47 // PIN for flashlight LED
#define FLASH_MODE              GPIO_PIN_MODE_WS281X
#define FLASH_SMARTLED_TYPE     LED_WS2812
#define FLASH_SMARTLED_COLOR    Rgb{127, 127, 127}
#define FLASH_SMARTLED_QUANTITY 4

#elif defined(BOARD_ESP32_S3_ETH_V2)
#define BoardType         "ESP32-S3-ETH"

// Uart
//-------------------------------------------------
#define UART_PORT_NUM     UART_NUM_0
#define UART_TX_GPIO_NUM  GPIO_NUM_43
#define UART_RX_GPIO_NUM  GPIO_NUM_44
#define UART_RTS_GPIO_NUM GPIO_NUM_NC
#define UART_CTS_GPIO_NUM GPIO_NUM_NC
#define UART_BUFFER_SIZE  2048

// SD card (operated with SDMMC peripheral)
//-------------------------------------------------
#define GPIO_SDCARD_CLK   GPIO_NUM_40
#define GPIO_SDCARD_CMD   GPIO_NUM_42
#define GPIO_SDCARD_D0    GPIO_NUM_41

#ifndef __SD_USE_ONE_LINE_MODE__
#error "Board not suppotred SD_FOUR_LINE_MODE"
#else
#define GPIO_SDCARD_D1 GPIO_NUM_NC
#define GPIO_SDCARD_D2 GPIO_NUM_NC
#define GPIO_SDCARD_D3 GPIO_NUM_3
#endif

// SPI_ETHERNET_W5500
//-------------------------------------------------
#define PER_ENABLE GPIO_NUM_46
#define ETH_SPI_EN GPIO_NUM_45

#define ETH_SPI_NUM 1

#define ETH_SPI_HOST      SPI2_HOST
#define ETH_SPI_CLOCK_MHZ 20 // SPI clock speed (MHz) (range 5 - 80, default 16)

#define ETH_SPI_POLLING0_MS 10                    // Set SPI Ethernet module polling period (default 10)
#define ETH_SPI_PHY_ADDR0   ESP_ETH_PHY_ADDR_AUTO // PHY address, set -1 to enable PHY address detection at initialization stage

#define ETH_SPI_CS0_GPIO      GPIO_NUM_39
#define ETH_SPI_INT0_GPIO     GPIO_NUM_38
#define ETH_SPI_PHY_RST0_GPIO GPIO_NUM_NC

#define ETH_SPI_MISO_GPIO GPIO_NUM_14
#define ETH_SPI_MOSI_GPIO GPIO_NUM_1
#define ETH_SPI_SCLK_GPIO GPIO_NUM_21

// Camera (suppotred OV2640 or OV5640)
//-------------------------------------------------
#define CAM_PIN_PWDN      GPIO_NUM_NC
#define CAM_PIN_RESET     GPIO_NUM_NC // software reset will be performed

#define CAM_PIN_XCLK GPIO_NUM_15
#define CAM_PIN_SIOD GPIO_NUM_4
#define CAM_PIN_SIOC GPIO_NUM_5

#define CAM_PIN_D0 GPIO_NUM_11 // Y2_GPIO_NUM
#define CAM_PIN_D1 GPIO_NUM_9  // Y3_GPIO_NUM
#define CAM_PIN_D2 GPIO_NUM_8  // Y4_GPIO_NUM
#define CAM_PIN_D3 GPIO_NUM_10 // Y5_GPIO_NUM
#define CAM_PIN_D4 GPIO_NUM_47 // Y6_GPIO_NUM
#define CAM_PIN_D5 GPIO_NUM_18 // Y7_GPIO_NUM
#define CAM_PIN_D6 GPIO_NUM_17 // Y8_GPIO_NUM
#define CAM_PIN_D7 GPIO_NUM_16 // Y9_GPIO_NUM

#define CAM_PIN_VSYNC GPIO_NUM_6
#define CAM_PIN_HREF  GPIO_NUM_7
#define CAM_PIN_PCLK  GPIO_NUM_13

#define CAM_XCLK_TIMER          LEDC_TIMER_0
#define CAM_XCLK_CHANNEL        LEDC_CHANNEL_0

// GpioHandler
#define GPIO_IO1                GPIO_NUM_33
#define GPIO_IO2                GPIO_NUM_34
#define GPIO_IO3                GPIO_NUM_35
#define GPIO_IO4                GPIO_NUM_36

// Statusled + ClassControllCamera
#define BLINK_GPIO              GPIO_NUM_48 // PIN for red board LED

// ClassControllCamera
#define FLASH_GPIO              GPIO_NUM_12 // PIN for flashlight LED
#define FLASH_MODE              GPIO_PIN_MODE_WS281X
#define FLASH_SMARTLED_TYPE     LED_WS2812
#define FLASH_SMARTLED_COLOR    Rgb{127, 127, 127}
#define FLASH_SMARTLED_QUANTITY 4

#else
#error "Board not selected"
#endif // Board PIN Map

// ******* LED definition
//// PWM für Flash-LED
#define LEDC_TIMER         LEDC_TIMER_1
#define LEDC_CHANNEL       LEDC_CHANNEL_1
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES      LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY     (5000)            // Frequency in Hertz. Set frequency at 5 kHz
#define LEDC_OUTPUT_INVERT (0)               // Enable (1) or disable (0) gpio output invert

// softAP
#define ESP_WIFI_AP_SSID         "AI-on-the-Edge"
#define ESP_WIFI_AP_IP           "192.168.4.1"
#define ESP_WIFI_AP_PASS         ""
#define ESP_WIFI_AP_CHANNEL      1
#define ESP_WIFI_AP_MAX_STA_CONN 1

#endif // ifndef defines_h
