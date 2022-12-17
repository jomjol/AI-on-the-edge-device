#ifndef defines_h
#define defines_h

/////////////////////////////////////////////
////          Global definitions         ////
/////////////////////////////////////////////

    #define FLASH_GPIO GPIO_NUM_4
    #define BLINK_GPIO GPIO_NUM_33

    #define __HIDE_PASSWORD

    #define USE_PWM_LEDFLASH
    // if __LEDGLOBAL is defined, a global variable is used for LED control, otherwise locally and each time a new
    #define __LEDGLOBAL

    #define CAMERA_MODEL_AI_THINKER
    #define BOARD_ESP32CAM_AITHINKER

    #define CONFIG_FILE "/sdcard/config/config.ini"
    #define __SD_USE_ONE_LINE_MODE__

    /* Max length a file path can have on storage */
     #define FILE_PATH_MAX (255)
    /* Max size of an individual file. Make sure this
    * value is same as that set in upload_script.html */
    #define MAX_FILE_SIZE   (8000*1024) // 8 MB
    #define MAX_FILE_SIZE_STR "8MB"
    /* Scratch buffer size */
    #define SCRATCH_BUFSIZE  4096 
    /* Size of partial log file to return */
    #define LOGFILE_LAST_PART_BYTES SCRATCH_BUFSIZE * 20 /* 80 kBytes */

    //#define DEBUG_DETAIL_ON 


/////////////////////////////////////////////
////      Conditionnal definitions       ////
/////////////////////////////////////////////

//******* camera model 
#if defined(CAMERA_MODEL_WROVER_KIT)
    #define PWDN_GPIO_NUM    -1
    #define RESET_GPIO_NUM   -1
    #define XCLK_GPIO_NUM    21
    #define SIOD_GPIO_NUM    26
    #define SIOC_GPIO_NUM    27

    #define Y9_GPIO_NUM      35
    #define Y8_GPIO_NUM      34
    #define Y7_GPIO_NUM      39
    #define Y6_GPIO_NUM      36
    #define Y5_GPIO_NUM      19
    #define Y4_GPIO_NUM      18
    #define Y3_GPIO_NUM       5
    #define Y2_GPIO_NUM       4
    #define VSYNC_GPIO_NUM   25
    #define HREF_GPIO_NUM    23
    #define PCLK_GPIO_NUM    22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
    #define PWDN_GPIO_NUM     -1
    #define RESET_GPIO_NUM    15
    #define XCLK_GPIO_NUM     27
    #define SIOD_GPIO_NUM     25
    #define SIOC_GPIO_NUM     23

    #define Y9_GPIO_NUM       19
    #define Y8_GPIO_NUM       36
    #define Y7_GPIO_NUM       18
    #define Y6_GPIO_NUM       39
    #define Y5_GPIO_NUM        5
    #define Y4_GPIO_NUM       34
    #define Y3_GPIO_NUM       35
    #define Y2_GPIO_NUM       32
    #define VSYNC_GPIO_NUM    22
    #define HREF_GPIO_NUM     26
    #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
    #define PWDN_GPIO_NUM     GPIO_NUM_32
    #define RESET_GPIO_NUM    -1
    #define XCLK_GPIO_NUM      GPIO_NUM_0
    #define SIOD_GPIO_NUM     GPIO_NUM_26
    #define SIOC_GPIO_NUM     GPIO_NUM_27

    #define Y9_GPIO_NUM       GPIO_NUM_35
    #define Y8_GPIO_NUM       GPIO_NUM_34
    #define Y7_GPIO_NUM       GPIO_NUM_39
    #define Y6_GPIO_NUM       GPIO_NUM_36
    #define Y5_GPIO_NUM       GPIO_NUM_21
    #define Y4_GPIO_NUM       GPIO_NUM_19
    #define Y3_GPIO_NUM       GPIO_NUM_18
    #define Y2_GPIO_NUM        GPIO_NUM_5
    #define VSYNC_GPIO_NUM    GPIO_NUM_25
    #define HREF_GPIO_NUM     GPIO_NUM_23
    #define PCLK_GPIO_NUM     GPIO_NUM_22

#else
    #error "Camera model not selected"
#endif  //camera model

// ******* Board type   
#ifdef BOARD_WROVER_KIT // WROVER-KIT PIN Map

    #define CAM_PIN_PWDN -1  //power down is not used
    #define CAM_PIN_RESET -1 //software reset will be performed
    #define CAM_PIN_XCLK 21
    #define CAM_PIN_SIOD 26
    #define CAM_PIN_SIOC 27

    #define CAM_PIN_D7 35
    #define CAM_PIN_D6 34
    #define CAM_PIN_D5 39
    #define CAM_PIN_D4 36
    #define CAM_PIN_D3 19
    #define CAM_PIN_D2 18
    #define CAM_PIN_D1 5
    #define CAM_PIN_D0 4
    #define CAM_PIN_VSYNC 25
    #define CAM_PIN_HREF 23
    #define CAM_PIN_PCLK 22

#endif //// WROVER-KIT PIN Map

    
#ifdef BOARD_ESP32CAM_AITHINKER // ESP32Cam (AiThinker) PIN Map

    #define CAM_PIN_PWDN 32
    #define CAM_PIN_RESET -1 //software reset will be performed
    #define CAM_PIN_XCLK 0
    #define CAM_PIN_SIOD 26
    #define CAM_PIN_SIOC 27

    #define CAM_PIN_D7 35
    #define CAM_PIN_D6 34
    #define CAM_PIN_D5 39
    #define CAM_PIN_D4 36
    #define CAM_PIN_D3 21
    #define CAM_PIN_D2 19
    #define CAM_PIN_D1 18
    #define CAM_PIN_D0 5
    #define CAM_PIN_VSYNC 25
    #define CAM_PIN_HREF 23
    #define CAM_PIN_PCLK 22

#endif // ESP32Cam (AiThinker) PIN Map

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

#endif // ifndef defines_h