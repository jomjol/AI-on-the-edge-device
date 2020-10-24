#include "ClassControllCamera.h"

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "Helper.h"
#include "CFindTemplate.h"

#include "camera_define.h"

#include "driver/ledc.h"

CCamera Camera;


#define FLASH_GPIO GPIO_NUM_4

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;



///////////////////////////////////////////////////////////////////////////////////////////////////////
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




////////////////////////////////////////////////////////////////////////////////////////////////////////






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


void CCamera::SetQualitySize(int qual, framesize_t resol)
{
    sensor_t * s = esp_camera_sensor_get();   
    s->set_quality(s, qual);    
    s->set_framesize(s, resol); 
    ActualResolution = resol;
    ActualQuality = qual;
}


esp_err_t CCamera::CaptureToFile(std::string nm, int delay)
{
//    nm =  "/sdcard/josef_zw.bmp";
    string ftype;

    if (delay > 0) 
    {
        LightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay( xDelay );
    }

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAGCAMERACLASS, "Camera Capture Failed");
        return ESP_FAIL;
    }
    printf("w %d, h %d, size %d\n", fb->width, fb->height, fb->len);

    nm = FormatFileName(nm);
    printf("Save Camera to : %s\n", nm.c_str());
    ftype = toUpper(getFileType(nm));
    printf("Filetype: %s\n", ftype.c_str());

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

    FILE * fp = fopen(nm.c_str(), "wb");
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

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAGCAMERACLASS, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
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
    return res;
}

void CCamera::LightOnOff(bool status)
{
	// Init the GPIO
    gpio_pad_select_gpio(FLASH_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(FLASH_GPIO, GPIO_MODE_OUTPUT);  

    if (status)  
        gpio_set_level(FLASH_GPIO, 1);
    else
        gpio_set_level(FLASH_GPIO, 0);      
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
            printf("Size: "); printf(_size); printf("\n");            
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
            printf("Quality: "); printf(_qual); printf("\n");
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
    printf("CreateClassCamera\n");
}

esp_err_t CCamera::InitCam()
{
    printf("Init Flash\n");
    //power up the camera if PWDN pin is defined
    if(PWDN_GPIO_NUM != -1){
        // Init the GPIO
        gpio_pad_select_gpio(PWDN_GPIO_NUM);
        /* Set the GPIO as a push/pull output */
        gpio_set_direction(PWDN_GPIO_NUM, GPIO_MODE_OUTPUT);
        gpio_set_level(PWDN_GPIO_NUM, 0);
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