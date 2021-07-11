#include "server_ota.h"

#include <string>
#include "string.h"

#include <esp_int_wdt.h>
#include <esp_task_wdt.h>


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event.h"
#include "esp_log.h"
#include <esp_ota_ops.h>
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include <nvs.h>
#include "nvs_flash.h"
#include "driver/gpio.h"
// #include "protocol_examples_common.h"
#include "errno.h"

#include <sys/stat.h>

#include "server_tflite.h"
#include "server_file.h"
#include "server_GPIO.h"

#include "ClassLogFile.h"

#include "Helper.h"


// #define DEBUG_DETAIL_ON 


#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */


/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };


#define OTA_URL_SIZE 256
static const char *TAGPARTOTA = "server_ota";


static void infinite_loop(void)
{
    int i = 0;
    ESP_LOGI(TAGPARTOTA, "When a new firmware is available on the server, press the reset button to download it");
    while(1) {
        ESP_LOGI(TAGPARTOTA, "Waiting for a new firmware ... %d", ++i);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}



static bool ota_update_task(std::string fn)
{
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAGPARTOTA, "Starting OTA update");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAGPARTOTA, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAGPARTOTA, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAGPARTOTA, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);


    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAGPARTOTA, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
//    assert(update_partition != NULL);

    int binary_file_length = 0;

    // deal with all receive packet 
    bool image_header_was_checked = false;

    int data_read;     

    FILE* f = OpenFileAndWait(fn.c_str(), "rb");     // vorher  nur "r"
    data_read = fread(ota_write_data, 1, BUFFSIZE, f);

    while (data_read > 0) {
        if (data_read < 0) {
            ESP_LOGE(TAGPARTOTA, "Error: SSL data read error");
            return false;
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    ESP_LOGI(TAGPARTOTA, "New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        ESP_LOGI(TAGPARTOTA, "Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        ESP_LOGI(TAGPARTOTA, "Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            ESP_LOGW(TAGPARTOTA, "New version is the same as invalid version.");
                            ESP_LOGW(TAGPARTOTA, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            ESP_LOGW(TAGPARTOTA, "The firmware has been rolled back to the previous version.");
                            infinite_loop();
                        }
                    }

/*
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(TAGPARTOTA, "Current running version is the same as a new. We will not continue the update.");
                        infinite_loop();
                    }
*/
                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAGPARTOTA, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                        return false;
                    }
                    ESP_LOGI(TAGPARTOTA, "esp_ota_begin succeeded");
                } else {
                    ESP_LOGE(TAGPARTOTA, "received package is not fit len");
                    return false;
                }
            }            
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                return false;
            }
            binary_file_length += data_read;
            ESP_LOGD(TAGPARTOTA, "Written image length %d", binary_file_length);
        } else if (data_read == 0) {
           //
           // * As esp_http_client_read never returns negative error code, we rely on
           // * `errno` to check for underlying transport connectivity closure if any
           //
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ESP_LOGE(TAGPARTOTA, "Connection closed, errno = %d", errno);
                break;
            }
        }
        data_read = fread(ota_write_data, 1, BUFFSIZE, f);
    }
    fclose(f);  

    ESP_LOGI(TAGPARTOTA, "Total Write binary data length: %d", binary_file_length);

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAGPARTOTA, "Image validation failed, image is corrupted");
        }
        ESP_LOGE(TAGPARTOTA, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        return false;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAGPARTOTA, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));

    }
//    ESP_LOGI(TAGPARTOTA, "Prepare to restart system!");
//    esp_restart();

    return true ;
}


static void print_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAGPARTOTA, "%s: %s", label, hash_print);
}


static bool diagnostic(void)
{
/*
    gpio_config_t io_conf;
    io_conf.intr_type    = (gpio_int_type_t) GPIO_PIN_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAGPARTOTA, "Diagnostics (5 sec)...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    bool diagnostic_is_ok = gpio_get_level(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);

    gpio_reset_pin(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);

    return diagnostic_is_ok;
*/
    return true;
}

void CheckOTAUpdate(void)
{
    ESP_LOGI(TAGPARTOTA, "Start CheckOTAUpdateCheck ...");
    printf("Start CheckOTAUpdateCheck ...\n");

    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for the partition table
    partition.address   = ESP_PARTITION_TABLE_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type      = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    esp_err_t res_stat_partition = esp_ota_get_state_partition(running, &ota_state);
    switch (res_stat_partition)
    {
        case ESP_OK:
            printf("CheckOTAUpdate Partition: ESP_OK\n");
            if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
                if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
                    // run diagnostic function ...
                    bool diagnostic_is_ok = diagnostic();
                    if (diagnostic_is_ok) {
                        ESP_LOGI(TAGPARTOTA, "Diagnostics completed successfully! Continuing execution ...");
                        printf("Diagnostics completed successfully! Continuing execution ...\n");
                        esp_ota_mark_app_valid_cancel_rollback();
                    } else {
                        ESP_LOGE(TAGPARTOTA, "Diagnostics failed! Start rollback to the previous version ...");
                        printf("Diagnostics failed! Start rollback to the previous version ...\n");
                        esp_ota_mark_app_invalid_rollback_and_reboot();
                    }
                }
            }            
            break;
        case ESP_ERR_INVALID_ARG:
            printf("CheckOTAUpdate Partition: ESP_ERR_INVALID_ARG\n");
            break;
        case ESP_ERR_NOT_SUPPORTED:
            printf("CheckOTAUpdate Partition: ESP_ERR_NOT_SUPPORTED\n");
            break;
        case ESP_ERR_NOT_FOUND:
            printf("CheckOTAUpdate Partition: ESP_ERR_NOT_FOUND\n");
            break;
    }
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            // run diagnostic function ...
            bool diagnostic_is_ok = diagnostic();
            if (diagnostic_is_ok) {
                ESP_LOGI(TAGPARTOTA, "Diagnostics completed successfully! Continuing execution ...");
                printf("Diagnostics completed successfully! Continuing execution ...\n");
                esp_ota_mark_app_valid_cancel_rollback();
            } else {
                ESP_LOGE(TAGPARTOTA, "Diagnostics failed! Start rollback to the previous version ...");
                printf("Diagnostics failed! Start rollback to the previous version ...\n");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
}



esp_err_t handler_ota_update(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON     
    LogFile.WriteHeapInfo("handler_ota_update - Start");    
#endif

    LogFile.WriteToFile("handler_ota_update");    
    char _query[200];
    char _filename[30];
    char _valuechar[30];    
    std::string fn = "/sdcard/firmware/";
    bool _file_del = false;
    std::string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        printf("Query: "); printf(_query); printf("\n");
        
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
            printf("task is found"); printf(_valuechar); printf("\n"); 
            _task = std::string(_valuechar);
        }

        if (httpd_query_key_value(_query, "file", _filename, 30) == ESP_OK)
        {
            fn.append(_filename);
            printf("File: "); printf(fn.c_str()); printf("\n");            
        }
        if (httpd_query_key_value(_query, "delete", _filename, 30) == ESP_OK)
        {
            fn.append(_filename);
            _file_del = true;
            printf("Delete Default File: "); printf(fn.c_str()); printf("\n");            
        }

    };

    if (_task.compare("unziphtml") == 0)
    {
        std::string in, out, zw;

        in = "/sdcard/firmware/html.zip";
        out = "/sdcard/html/";

        delete_all_in_directory(out);

        unzip(in, out);
        zw = "HTML Update Successfull!<br><br>No reboot necessary";
        httpd_resp_sendstr_chunk(req, zw.c_str());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }



    if (_file_del)
    {
        struct stat file_stat;
        if (stat(fn.c_str(), &file_stat) != -1) {
            printf("Deleting file : %s", fn.c_str());
            /* Delete file */
            unlink(fn.c_str());
        }
        /* Respond with an empty chunk to signal HTTP response completion */
        httpd_resp_send_chunk(req, NULL, 0);
        return ESP_OK;
    }

    const char* resp_str;    

    KillTFliteTasks();
    gpio_handler_deinit();
    if (ota_update_task(fn))
    {
        resp_str = "Firmware Update Successfull!<br><br>You can restart now.";
    }
    else
    {
        resp_str = "Error during Firmware Update!!!<br><br>Please check output of console.";
    }

    httpd_resp_send(req, resp_str, strlen(resp_str));  

#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_ota_update - Done");    
#endif

    return ESP_OK;
};

void hard_restart() {
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}

void task_reboot(void *pvParameter)
{
    while(1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        esp_restart();
        hard_restart();
    }

    vTaskDelete(NULL); //Delete this task if it exits from the loop above
}

void doReboot(){
    ESP_LOGI(TAGPARTOTA, "Reboot in 5sec");
    LogFile.WriteToFile("Reboot in 5sec");
    xTaskCreate(&task_reboot, "reboot", configMINIMAL_STACK_SIZE * 64, NULL, 10, NULL);
    // KillTFliteTasks(); // kills itself 
    gpio_handler_destroy();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    esp_restart();
    hard_restart();
}


esp_err_t handler_reboot(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON     
    LogFile.WriteHeapInfo("handler_reboot - Start");
#endif    

    LogFile.WriteToFile("handler_reboot");
    ESP_LOGI(TAGPARTOTA, "!!! System will restart within 5 sec!!!");
    const char* resp_str = "!!! System will restart within 5 sec!!!";
    httpd_resp_send(req, resp_str, strlen(resp_str)); 
    
    doReboot();

#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_reboot - Done");    
#endif

    return ESP_OK;
}

void register_server_ota_sdcard_uri(httpd_handle_t server)
{
    ESP_LOGI(TAGPARTOTA, "server_ota - Registering URI handlers");
    
    httpd_uri_t camuri = { };
    camuri.method    = HTTP_GET;
    camuri.uri       = "/ota";
    camuri.handler   = handler_ota_update;
    camuri.user_ctx  = (void*) "Do OTA";    
    httpd_register_uri_handler(server, &camuri);

    camuri.method    = HTTP_GET;
    camuri.uri       = "/reboot";
    camuri.handler   = handler_reboot;
    camuri.user_ctx  = (void*) "Reboot";    
    httpd_register_uri_handler(server, &camuri);

}
