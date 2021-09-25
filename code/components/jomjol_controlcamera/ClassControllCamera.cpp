#include "ClassControllCamera.h"
#include "ClassLogFile.h"

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "Helper.h"
#include "CImageBasis.h"

#include "server_ota.h"
#include "server_GPIO.h"


#define BOARD_ESP32CAM_AITHINKER


#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"

// #define DEBUG_DETAIL_ON


// ESP32Cam (AiThinker) PIN Map

#define CAM_PIN_PWDN (gpio_num_t) 32
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

static const char *TAGCAMERACLASS = "server_part_camera"; 

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
//    .xclk_freq_hz = 20000000,             // Orginalwert
    .xclk_freq_hz = 5000000,               // Test, um die Bildfehler los zu werden !!!!
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_VGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG
//    .frame_size = FRAMESIZE_UXGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    

    .jpeg_quality = 5, //0-63 lower number means higher quality
    .fb_count = 1       //if more than one, i2s runs in continuous mode. Use only with JPEG
};


#include "driver/ledc.h"

CCamera Camera;

#define FLASH_GPIO GPIO_NUM_4
#define BLINK_GPIO GPIO_NUM_33

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;


#define LEDC_LS_CH2_GPIO       (4)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_TEST_DUTY         (4000)

void test(){
    ledc_channel_config_t ledc_channel = { };

    ledc_channel.channel = LEDC_LS_CH2_CHANNEL;
    ledc_channel.duty       = 0;
    ledc_channel.gpio_num   = FLASH_GPIO;
    ledc_channel.speed_mode = LEDC_LS_MODE;
    ledc_channel.hpoint     = 0;
    ledc_channel.timer_sel  = LEDC_LS_TIMER;

    ledc_channel_config(&ledc_channel);

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, LEDC_TEST_DUTY);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
};



static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

bool CCamera::SetBrightnessContrastSaturation(int _brightness, int _contrast, int _saturation)
{
    bool result = false;
    sensor_t * s = esp_camera_sensor_get(); 
    if (_brightness > -100)
        _brightness = min(2, max(-2, _brightness));
    if (_contrast > -100)
        _contrast = min(2, max(-2, _contrast));
//    _saturation = min(2, max(-2, _saturation));

//    s->set_saturation(s, _saturation);
    if (_contrast > -100)
        s->set_contrast(s, _contrast);
    if (_brightness > -100)
        s->set_brightness(s, _brightness);

    if ((_brightness != brightness) && (_brightness > -100))
        result = true;
    if ((_contrast != contrast) && (_contrast > -100))
        result = true;
    if ((_saturation != saturation) && (_saturation > -100))
        result = true;
    
    if (_brightness > -100)
        brightness = _brightness;
    if (_contrast > -100)
        contrast = _contrast;
    if (_saturation > -100)
       saturation = _saturation;

    if (result && isFixedExposure)
        EnableAutoExposure(waitbeforepicture_org);

    return result;
}


void CCamera::SetQualitySize(int qual, framesize_t resol)
{
    sensor_t * s = esp_camera_sensor_get();   
    s->set_quality(s, qual);    
    s->set_framesize(s, resol); 
    ActualResolution = resol;
    ActualQuality = qual;

    if (resol == FRAMESIZE_QVGA)
    {
        image_height = 240;
        image_width = 320;             
    }
    if (resol == FRAMESIZE_VGA)
    {
        image_height = 480;
        image_width = 640;             
    }
    // No higher Mode than VGA, damit der Kameraspeicher ausreicht.
/*
    if (resol == FRAMESIZE_SVGA)
    {
        image_height = 600;
        image_width = 800;             
    }
    if (resol == FRAMESIZE_XGA)
    {
        image_height = 768;
        image_width = 1024;             
    }
    if (resol == FRAMESIZE_SXGA)
    {
        image_height = 1024;
        image_width = 1280;             
    }
    if (resol == FRAMESIZE_UXGA)
    {
        image_height = 1200;
        image_width = 1600;             
    }
*/
}


void CCamera::EnableAutoExposure(int flashdauer)
{
    LEDOnOff(true);
    if (flashdauer > 0)
        LightOnOff(true);
    const TickType_t xDelay = flashdauer / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAGCAMERACLASS, "Camera Capture Failed");
        LEDOnOff(false);
        LightOnOff(false);
        doReboot();
    }
    esp_camera_fb_return(fb);        

    sensor_t * s = esp_camera_sensor_get(); 
    s->set_gain_ctrl(s, 0);
    s->set_exposure_ctrl(s, 0);


    LEDOnOff(false);  
    LightOnOff(false);
    isFixedExposure = true;
    waitbeforepicture_org = flashdauer;
}



esp_err_t CCamera::CaptureToBasisImage(CImageBasis *_Image, int delay)
{
    string ftype;

    uint8_t *zwischenspeicher = NULL;


    LEDOnOff(true);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CCamera::CaptureToBasisImage - Start");
#endif

    if (delay > 0) 
    {
        LightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay( xDelay );
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CCamera::CaptureToBasisImage - After LightOn");
#endif

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAGCAMERACLASS, "CaptureToBasisImage: Camera Capture Failed");
        LEDOnOff(false);
        LightOnOff(false);
        doReboot();

        return ESP_FAIL;
    }

    int _size = fb->len;
    zwischenspeicher = (uint8_t*) malloc(_size);
    for (int i = 0; i < _size; ++i)
        *(zwischenspeicher + i) = *(fb->buf + i);
    esp_camera_fb_return(fb);        

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CCamera::CaptureToBasisImage - After fb_get");
#endif

    LEDOnOff(false);  

    if (delay > 0) 
        LightOnOff(false);
 
//    TickType_t xDelay = 1000 / portTICK_PERIOD_MS;     
//    vTaskDelay( xDelay );  // wait for power to recover
    
    uint8_t * buf = NULL;

    CImageBasis _zwImage;
    _zwImage.LoadFromMemory(zwischenspeicher, _size);
    free(zwischenspeicher);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CCamera::CaptureToBasisImage - After LoadFromMemory");
#endif

    stbi_uc* p_target;
    stbi_uc* p_source;    
    int channels = 3;
    int width = image_width;
    int height = image_height;

#ifdef DEBUG_DETAIL_ON
    std::string _zw = "Targetimage: " + std::to_string((int) _Image->rgb_image) + " Size: " + std::to_string(_Image->width) + ", " + std::to_string(_Image->height);
    _zw = _zw + " _zwImage: " + std::to_string((int) _zwImage.rgb_image)  + " Size: " + std::to_string(_zwImage.width) + ", " + std::to_string(_zwImage.height);
    LogFile.WriteToFile(_zw);
#endif

    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_target = _Image->rgb_image + (channels * (y * width + x));
            p_source = _zwImage.rgb_image + (channels * (y * width + x));
            p_target[0] = p_source[0];
            p_target[1] = p_source[1];
            p_target[2] = p_source[2];
        }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CCamera::CaptureToBasisImage - After Copy To Target");
#endif

    free(buf);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("CCamera::CaptureToBasisImage - Done");
#endif

    return ESP_OK;    
}


esp_err_t CCamera::CaptureToFile(std::string nm, int delay)
{
    string ftype;

     LEDOnOff(true);              // Abgeschaltet, um Strom zu sparen !!!!!!

    if (delay > 0) 
    {
        LightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay( xDelay );
    }

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAGCAMERACLASS, "CaptureToFile: Camera Capture Failed");
        LEDOnOff(false);
        LightOnOff(false);
        doReboot();

        return ESP_FAIL;
    }
    LEDOnOff(false);    

#ifdef DEBUG_DETAIL_ON    
    printf("w %d, h %d, size %d\n", fb->width, fb->height, fb->len);
#endif

    nm = FormatFileName(nm);

#ifdef DEBUG_DETAIL_ON
    printf("Save Camera to : %s\n", nm.c_str());
#endif

    ftype = toUpper(getFileType(nm));

#ifdef DEBUG_DETAIL_ON
    printf("Filetype: %s\n", ftype.c_str());
#endif

    uint8_t * buf = NULL;
    size_t buf_len = 0;   
    bool converted = false; 

    if (ftype.compare("BMP") == 0)
    {
        frame2bmp(fb, &buf, &buf_len);
        converted = true;
    }
    if (ftype.compare("JPG") == 0)
    {
        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, ActualQuality, &buf, &buf_len);
            converted = true;
            if(!jpeg_converted){
                ESP_LOGE(TAGCAMERACLASS, "JPEG compression failed");
            }
        } else {
            buf_len = fb->len;
            buf = fb->buf;
        }
    }

    FILE * fp = OpenFileAndWait(nm.c_str(), "wb");
    if (fp == NULL)  /* If an error occurs during the file creation */
    {
        fprintf(stderr, "fopen() failed for '%s'\n", nm.c_str());
    }
    else
    {
        fwrite(buf, sizeof(uint8_t), buf_len, fp); 
        fclose(fp);
    }    
    if (converted)
        free(buf);

    esp_camera_fb_return(fb);

    if (delay > 0) 
    {
        LightOnOff(false);
    }

    return ESP_OK;    
}


esp_err_t CCamera::CaptureToHTTP(httpd_req_t *req, int delay)
{
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();


    LEDOnOff(true);

    if (delay > 0) 
    {
        LightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay( xDelay );
    }


    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAGCAMERACLASS, "Camera capture failed");
        LEDOnOff(false);
        LightOnOff(false);
        httpd_resp_send_500(req);
//        doReboot();

        return ESP_FAIL;
    }

    LEDOnOff(false); 
    
    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=raw.jpg");
    }

    if(res == ESP_OK){
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    
    ESP_LOGI(TAGCAMERACLASS, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));

    if (delay > 0) 
    {
        LightOnOff(false);
    }

    return res;
}

void CCamera::LightOnOff(bool status)
{
    GpioHandler* gpioHandler = gpio_handler_get();
    if ((gpioHandler != NULL) && (gpioHandler->isEnabled())) {
        printf("Use gpioHandler flashLigh\n");
        gpioHandler->flashLightEnable(status);
    }  else {
        // Init the GPIO
        gpio_pad_select_gpio(FLASH_GPIO);
        /* Set the GPIO as a push/pull output */
        gpio_set_direction(FLASH_GPIO, GPIO_MODE_OUTPUT);  

        if (status)  
            gpio_set_level(FLASH_GPIO, 1);
        else
            gpio_set_level(FLASH_GPIO, 0);
    }
}

void CCamera::LEDOnOff(bool status)
{
	// Init the GPIO
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);  

    if (!status)  
        gpio_set_level(BLINK_GPIO, 1);
    else
        gpio_set_level(BLINK_GPIO, 0);      
}


void CCamera::GetCameraParameter(httpd_req_t *req, int &qual, framesize_t &resol)
{
    char _query[100];
    char _qual[10];
    char _size[10];

    resol = ActualResolution;
    qual = ActualQuality;


    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
        printf("Query: "); printf(_query); printf("\n");
        if (httpd_query_key_value(_query, "size", _size, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON   
            printf("Size: "); printf(_size); printf("\n");            
#endif
            if (strcmp(_size, "QVGA") == 0)
                resol = FRAMESIZE_QVGA;       // 320x240
            if (strcmp(_size, "VGA") == 0)
                resol = FRAMESIZE_VGA;      // 640x480
            if (strcmp(_size, "SVGA") == 0)
                resol = FRAMESIZE_SVGA;     // 800x600
            if (strcmp(_size, "XGA") == 0)
                resol = FRAMESIZE_XGA;      // 1024x768
            if (strcmp(_size, "SXGA") == 0)
                resol = FRAMESIZE_SXGA;     // 1280x1024
            if (strcmp(_size, "UXGA") == 0)
                 resol = FRAMESIZE_UXGA;     // 1600x1200   
        }
        if (httpd_query_key_value(_query, "quality", _qual, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON   
            printf("Quality: "); printf(_qual); printf("\n");
#endif
            qual = atoi(_qual);
                
            if (qual > 63)
                qual = 63;
            if (qual < 0)
                qual = 0;
        }
    };
}

framesize_t CCamera::TextToFramesize(const char * _size)
{
    if (strcmp(_size, "QVGA") == 0)
        return FRAMESIZE_QVGA;       // 320x240
    if (strcmp(_size, "VGA") == 0)
        return FRAMESIZE_VGA;      // 640x480
    if (strcmp(_size, "SVGA") == 0)
        return FRAMESIZE_SVGA;     // 800x600
    if (strcmp(_size, "XGA") == 0)
        return FRAMESIZE_XGA;      // 1024x768
    if (strcmp(_size, "SXGA") == 0)
        return FRAMESIZE_SXGA;     // 1280x1024
    if (strcmp(_size, "UXGA") == 0)
        return FRAMESIZE_UXGA;     // 1600x1200   
    return ActualResolution;
}


CCamera::CCamera()
{
#ifdef DEBUG_DETAIL_ON    
    printf("CreateClassCamera\n");
#endif
    brightness = -5;
    contrast = -5;
    saturation = -5;
    isFixedExposure = false;
}

esp_err_t CCamera::InitCam()
{
    if(CAM_PIN_PWDN != -1){
        // Init the GPIO
        gpio_pad_select_gpio(CAM_PIN_PWDN);
        /* Set the GPIO as a push/pull output */
        gpio_set_direction(CAM_PIN_PWDN, GPIO_MODE_OUTPUT);
        gpio_set_level(CAM_PIN_PWDN, 0);
    }

    printf("Init Camera\n");
    ActualQuality = camera_config.jpeg_quality;
    ActualResolution = camera_config.frame_size;
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAGCAMERACLASS, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}