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
static const char *TAG = "OTA";

esp_err_t handler_reboot(httpd_req_t *req);


static void infinite_loop(void)
{
    int i = 0;
    ESP_LOGI(TAG, "When a new firmware is available on the server, press the reset button to download it");
    while(1) {
        ESP_LOGI(TAG, "Waiting for a new firmware... %d", ++i);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}



static bool ota_update_task(std::string fn)
{
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA update");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become somehow corrupted.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);


    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
//    assert(update_partition != NULL);

    int binary_file_length = 0;

    // deal with all receive packet 
    bool image_header_was_checked = false;

    int data_read;     

    FILE* f = OpenFileAndWait(fn.c_str(), "rb");     // vorher  nur "r"

    if (f == NULL) { // File does not exist
        return false;
    }

    data_read = fread(ota_write_data, 1, BUFFSIZE, f);

    while (data_read > 0) {
        if (data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            return false;
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            ESP_LOGW(TAG, "New version is the same as invalid version.");
                            ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                            infinite_loop();
                        }
                    }

/*
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                        infinite_loop();
                    }
*/
                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                        return false;
                    }
                    ESP_LOGI(TAG, "esp_ota_begin succeeded");
                } else {
                    ESP_LOGE(TAG, "received package is not fit len");
                    return false;
                }
            }            
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                return false;
            }
            binary_file_length += data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_length);
        } else if (data_read == 0) {
           //
           // * As esp_http_client_read never returns negative error code, we rely on
           // * `errno` to check for underlying transport connectivity closure if any
           //
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
                break;
            }
        }
        data_read = fread(ota_write_data, 1, BUFFSIZE, f);
    }
    fclose(f);  

    ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        return false;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));

    }
//    ESP_LOGI(TAG, "Prepare to restart system!");
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
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}


static bool diagnostic(void)
{
    return true;
}

void CheckOTAUpdate(void)
{
    ESP_LOGI(TAG, "Start CheckOTAUpdateCheck...");

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
            ESP_LOGD(TAG, "CheckOTAUpdate Partition: ESP_OK");
            if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
                if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
                    // run diagnostic function ...
                    bool diagnostic_is_ok = diagnostic();
                    if (diagnostic_is_ok) {
                        ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution...");
                        esp_ota_mark_app_valid_cancel_rollback();
                    } else {
                        ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version...");
                        esp_ota_mark_app_invalid_rollback_and_reboot();
                    }
                }
            }            
            break;
        case ESP_ERR_INVALID_ARG:
            ESP_LOGD(TAG, "CheckOTAUpdate Partition: ESP_ERR_INVALID_ARG");
            break;
        case ESP_ERR_NOT_SUPPORTED:
            ESP_LOGD(TAG, "CheckOTAUpdate Partition: ESP_ERR_NOT_SUPPORTED");
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGD(TAG, "CheckOTAUpdate Partition: ESP_ERR_NOT_FOUND");
            break;
    }
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            // run diagnostic function ...
            bool diagnostic_is_ok = diagnostic();
            if (diagnostic_is_ok) {
                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution...");
                esp_ota_mark_app_valid_cancel_rollback();
            } else {
                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version...");
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

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "handler_ota_update");
    char _query[200];
    char _filename[100];
    char _valuechar[30];    
    std::string fn = "/sdcard/firmware/";
    bool _file_del = false;
    std::string _task = "";

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        ESP_LOGD(TAG, "Query: %s", _query);
        
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
            ESP_LOGD(TAG, "task is found: %s", _valuechar);
            _task = std::string(_valuechar);
        }

        if (httpd_query_key_value(_query, "file", _filename, 100) == ESP_OK)
        {
            fn.append(_filename);
            ESP_LOGD(TAG, "File: %s", fn.c_str());
        }
        if (httpd_query_key_value(_query, "delete", _filename, 100) == ESP_OK)
        {
            fn.append(_filename);
            _file_del = true;
            ESP_LOGD(TAG, "Delete Default File: %s", fn.c_str());
        }

    };

    if (_task.compare("emptyfirmwaredir") == 0)
    {
        ESP_LOGD(TAG, "Start empty directory /firmware");
        delete_all_in_directory("/sdcard/firmware");
        std::string zw = "firmware directory deleted - v2\n";
        ESP_LOGD(TAG, "%s", zw.c_str());
        printf("Ausgabe: %s\n", zw.c_str());
    
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, zw.c_str(), strlen(zw.c_str())); 
        /* Respond with an empty chunk to signal HTTP response completion */
        httpd_resp_send_chunk(req, NULL, 0);  

        ESP_LOGD(TAG, "Done empty directory /firmware");
        return ESP_OK;
    }

    if (_task.compare("update") == 0)
    {
        std::string filetype = toUpper(getFileType(fn));
        if (filetype.length() == 0)
        {
            std::string zw = "Update failed - no file specified (zip, bin, tfl, tlite)";
            httpd_resp_sendstr_chunk(req, zw.c_str());
            httpd_resp_sendstr_chunk(req, NULL);  
            return ESP_OK;        
        }


        if ((filetype == "TFLITE") || (filetype == "TFL"))
        {
            std::string out = "/sdcard/config/" + getFileFullFileName(fn);
            DeleteFile(out);
            CopyFile(fn, out);
            DeleteFile(fn);

            const char*  resp_str = "Neural Network File copied.";
            httpd_resp_sendstr_chunk(req, resp_str);
            httpd_resp_sendstr_chunk(req, NULL);  
            return ESP_OK;
        }


        if (filetype == "ZIP")
        {
            std::string in, out, outbin, zw, retfirmware;

            out = "/sdcard/html";
            outbin = "/sdcard/firmware";

            retfirmware = unzip_new(fn, out+"/", outbin+"/");

            if (retfirmware.length() > 0)
            {
                filetype = "BIN";
                fn = retfirmware;
            }
            else
            {
                zw = "Web Interface Update Successfull!\nNo reboot necessary.\n";
                httpd_resp_sendstr_chunk(req, zw.c_str());
                httpd_resp_sendstr_chunk(req, NULL);  
                return ESP_OK;        
            }
        }


        if (filetype == "BIN")
        {
            const char* resp_str; 

            KillTFliteTasks();
            gpio_handler_deinit();
            if (ota_update_task(fn))
            {
                std::string zw = "reboot\n";
                httpd_resp_sendstr_chunk(req, zw.c_str());
                httpd_resp_sendstr_chunk(req, NULL);  
                ESP_LOGD(TAG, "Send reboot");
                return ESP_OK;                
            }

            resp_str = "Error during Firmware Update!!!\nPlease check output of console.";
            httpd_resp_send(req, resp_str, strlen(resp_str));  

            #ifdef DEBUG_DETAIL_ON 
                LogFile.WriteHeapInfo("handler_ota_update - Done");    
            #endif

            return ESP_OK;
        }


        std::string zw = "Update failed - no valid file specified (zip, bin, tfl, tlite)!";
        httpd_resp_sendstr_chunk(req, zw.c_str());
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }


    if (_task.compare("unziphtml") == 0)
    {
        ESP_LOGD(TAG, "Task unziphtml");
        std::string in, out, zw;

        in = "/sdcard/firmware/html.zip";
        out = "/sdcard/html";

        delete_all_in_directory(out);

        unzip(in, out+"/");
        zw = "Web Interface Update Successfull!\nNo reboot necessary";
        httpd_resp_send(req, zw.c_str(), strlen(zw.c_str()));
        httpd_resp_sendstr_chunk(req, NULL);  
        return ESP_OK;        
    }

    if (_file_del)
    {
        ESP_LOGD(TAG, "Delete !! _file_del: %s", fn.c_str());
        struct stat file_stat;
        int _result = stat(fn.c_str(), &file_stat);
        ESP_LOGD(TAG, "Ergebnis %d\n", _result);
        if (_result == 0) {
            ESP_LOGD(TAG, "Deleting file : %s", fn.c_str());
            /* Delete file */
            unlink(fn.c_str());
        }
        else
        {
            ESP_LOGD(TAG, "File does not exist: %s", fn.c_str());
        }
        /* Respond with an empty chunk to signal HTTP response completion */
        std::string zw = "file deleted\n";
        ESP_LOGD(TAG, "%s", zw.c_str());
        httpd_resp_send(req, zw.c_str(), strlen(zw.c_str()));
        httpd_resp_send_chunk(req, NULL, 0);
        return ESP_OK;
    }

    string zw = "ota without parameter - should not be the case!";
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, zw.c_str(), strlen(zw.c_str())); 
    httpd_resp_send_chunk(req, NULL, 0);  

    ESP_LOGE(TAG, "ota without parameter - should not be the case!");

/*  
    const char* resp_str;    

    KillTFliteTasks();
    gpio_handler_deinit();
    if (ota_update_task(fn))
    {
        resp_str = "Firmware Update Successfull! You can restart now.";
    }
    else
    {
        resp_str = "Error during Firmware Update!!! Please check console output.";
    }

    httpd_resp_send(req, resp_str, strlen(resp_str));  

#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_ota_update - Done");    
#endif
*/

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
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Reboot triggered by Software (5s).");
    LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Reboot in 5sec");
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

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "handler_reboot");
    ESP_LOGI(TAG, "!!! System will restart within 5 sec!!!");
    const char* resp_str = "<body style='font-family: arial'> <h3 id=t></h3></body><script>var h='Rebooting!<br>The page will automatically reload in around 25..60s.<br>'; document.getElementById('t').innerHTML=h; setInterval(function (){h +='.'; document.getElementById('t').innerHTML=h; fetch(window.location.hostname,{mode: 'no-cors'}).then(r=>{parent.location.href=('/index.html');})}, 1000);</script>";
    httpd_resp_send(req, resp_str, strlen(resp_str)); 
    
    doReboot();

#ifdef DEBUG_DETAIL_ON 
    LogFile.WriteHeapInfo("handler_reboot - Done");    
#endif

    return ESP_OK;
}

void register_server_ota_sdcard_uri(httpd_handle_t server)
{
    ESP_LOGI(TAG, "Registering URI handlers");
    
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
