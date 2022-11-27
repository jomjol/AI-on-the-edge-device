/* HTTP File Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include "server_file.h"


#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <dirent.h>
#ifdef __cplusplus
}
#endif

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include <esp_spiffs.h>
#include "esp_http_server.h"

#include "defines.h"
#include "ClassLogFile.h"

#include "server_tflite.h"

#include "server_help.h"
#include "interface_mqtt.h"
#include "server_GPIO.h"

#include "Helper.h"
#include "miniz.h"

static const char *TAG = "OTA FILE";

/* Max length a file path can have on storage */
// #define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
#define FILE_PATH_MAX (255)

/* Max size of an individual file. Make sure this
 * value is same as that set in upload_script.html */
#define MAX_FILE_SIZE   (8000*1024) // 8 MB
#define MAX_FILE_SIZE_STR "8MB"


/* Scratch buffer size */
#define SCRATCH_BUFSIZE  4096 

/* Size of partial log file to return */
#define LOGFILE_LAST_PART_BYTES SCRATCH_BUFSIZE * 20 /* 80 kBytes */

struct file_server_data {
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};


#include <iostream>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

string SUFFIX_ZW = "_0xge";


static esp_err_t send_logfile(httpd_req_t *req, bool send_full_file);
static esp_err_t send_datafile(httpd_req_t *req, bool send_full_file);


esp_err_t get_numbers_file_handler(httpd_req_t *req)
{
    std::string ret = tfliteflow.getNumbersName();

//    ESP_LOGI(TAG, "Result get_numbers_file_handler: %s", ret.c_str());

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "text/plain");

    httpd_resp_sendstr_chunk(req, ret.c_str());
    httpd_resp_sendstr_chunk(req, NULL);

    return ESP_OK;
}


esp_err_t get_data_file_handler(httpd_req_t *req)
{
    struct dirent *entry;

    std::string _filename, _fileext;
    size_t pos = 0;
    
    const char verz_name[] = "/sdcard/log/data";
    ESP_LOGD(TAG, "Suche data files in /sdcard/log/data");

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "text/plain");

    DIR *dir = opendir(verz_name);
    while ((entry = readdir(dir)) != NULL) 
    {
        _filename = std::string(entry->d_name);
        ESP_LOGD(TAG, "File: %s", _filename.c_str());

        // ignore all files with starting dot (hidden files)
        if (_filename.rfind(".", 0) == 0) {
            continue;
        }

        _fileext = _filename;
        pos = _fileext.find_last_of(".");
        if (pos != std::string::npos)
            _fileext = _fileext.erase(0, pos + 1);

        ESP_LOGD(TAG, " Extension: %s", _fileext.c_str());

        if (_fileext == "csv")
        {
            _filename = _filename + "\t";
            httpd_resp_sendstr_chunk(req, _filename.c_str());
        }
    }
    closedir(dir);

    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}


esp_err_t get_tflite_file_handler(httpd_req_t *req)
{
    struct dirent *entry;

    std::string _filename, _fileext;
    size_t pos = 0;
    
    const char verz_name[] = "/sdcard/config";
    ESP_LOGD(TAG, "Suche TFLITE in /sdcard/config/");

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_type(req, "text/plain");

    DIR *dir = opendir(verz_name);
    while ((entry = readdir(dir)) != NULL) 
    {
        _filename = std::string(entry->d_name);
        ESP_LOGD(TAG, "File: %s", _filename.c_str());

        // ignore all files with starting dot (hidden files)
        if (_filename.rfind(".", 0) == 0) {
            continue;
        }

        _fileext = _filename;
        pos = _fileext.find_last_of(".");
        if (pos != std::string::npos)
            _fileext = _fileext.erase(0, pos + 1);

        ESP_LOGD(TAG, " Extension: %s", _fileext.c_str());

        if ((_fileext == "tfl") || (_fileext == "tflite"))
        {
            _filename = "/config/" + _filename + "\t";
            httpd_resp_sendstr_chunk(req, _filename.c_str());
        }
    }
    closedir(dir);

    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}


/* Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories */
static esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath, const char* uripath, bool readonly)
{
    char entrypath[FILE_PATH_MAX];
    char entrysize[16];
    const char *entrytype;

    struct dirent *entry;
    struct stat entry_stat;

    char dirpath_corrected[FILE_PATH_MAX];
    strcpy(dirpath_corrected, dirpath);

    file_server_data * server_data = (file_server_data *) req->user_ctx;
    if ((strlen(dirpath_corrected)-1) > strlen(server_data->base_path))      // if dirpath is not mountpoint, the last "\" needs to be removed
        dirpath_corrected[strlen(dirpath_corrected)-1] = '\0';

    DIR *dir = opendir(dirpath_corrected);

    const size_t dirpath_len = strlen(dirpath);
    ESP_LOGD(TAG, "Dirpath: <%s>, Pathlength: %d", dirpath, dirpath_len);

    /* Retrieve the base path of file storage to construct the full path */
    strlcpy(entrypath, dirpath, sizeof(entrypath));
    ESP_LOGD(TAG, "entrypath: <%s>", entrypath);

    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
        return ESP_FAIL;
    }

    /* Send HTML file header */
    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><body>");

/////////////////////////////////////////////////
    if (!readonly) {
        FILE *fd = OpenFileAndWait("/sdcard/html/upload_script.html", "r");
        char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
        size_t chunksize;
        do {
            chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);
            //        ESP_LOGD(TAG, "Chunksize %d", chunksize);
            if (chunksize > 0){
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                return ESP_FAIL;
                }
            }
        } while (chunksize != 0);
        fclose(fd);
        //    ESP_LOGI(TAG, "File sending complete");
    }
///////////////////////////////

    std::string _zw = std::string(dirpath);
    _zw = _zw.substr(8, _zw.length() - 8);
    _zw = "/delete/" + _zw + "?task=deldircontent"; 


    /* Send file-list table definition and column labels */
    httpd_resp_sendstr_chunk(req,
        "<table class=\"fixed\" border=\"1\">"
        "<col width=\"800px\" /><col width=\"300px\" /><col width=\"300px\" /><col width=\"100px\" />"
        "<thead><tr><th>Name</th><th>Type</th><th>Size (Bytes)</th>");
    if (!readonly) {
        httpd_resp_sendstr_chunk(req, "<th>Delete<br>"
            "<form method=\"post\" action=\"");
        httpd_resp_sendstr_chunk(req, _zw.c_str());
        httpd_resp_sendstr_chunk(req,
            "\"><button type=\"submit\">DELETE ALL!</button></form>"
            "</th></tr>");
    }
    httpd_resp_sendstr_chunk(req, "</thead><tbody>\n");

    /* Iterate over all files / folders and fetch their names and sizes */
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp("wlan.ini", entry->d_name) != 0 )        // wlan.ini soll nicht angezeigt werden!
        {
            entrytype = (entry->d_type == DT_DIR ? "directory" : "file");

            strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
            ESP_LOGD(TAG, "Entrypath: %s", entrypath);
            if (stat(entrypath, &entry_stat) == -1) {
                ESP_LOGE(TAG, "Failed to stat %s : %s", entrytype, entry->d_name);
                continue;
            }
            sprintf(entrysize, "%ld", entry_stat.st_size);
            ESP_LOGI(TAG, "Found %s : %s (%s bytes)", entrytype, entry->d_name, entrysize);

            /* Send chunk of HTML file containing table entries with file name and size */
            httpd_resp_sendstr_chunk(req, "<tr><td><a href=\"");
            httpd_resp_sendstr_chunk(req, "/fileserver");
            httpd_resp_sendstr_chunk(req, uripath);
            httpd_resp_sendstr_chunk(req, entry->d_name);
            if (entry->d_type == DT_DIR) {
                httpd_resp_sendstr_chunk(req, "/");
            }
            httpd_resp_sendstr_chunk(req, "\">");
            httpd_resp_sendstr_chunk(req, entry->d_name);
            httpd_resp_sendstr_chunk(req, "</a></td><td>");
            httpd_resp_sendstr_chunk(req, entrytype);
            httpd_resp_sendstr_chunk(req, "</td><td>");
            httpd_resp_sendstr_chunk(req, entrysize);
            if (!readonly) {
                httpd_resp_sendstr_chunk(req, "</td><td>");
                httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/delete");
                httpd_resp_sendstr_chunk(req, uripath);
                httpd_resp_sendstr_chunk(req, entry->d_name);
                httpd_resp_sendstr_chunk(req, "\"><button type=\"submit\">Delete</button></form>");
            }
            httpd_resp_sendstr_chunk(req, "</td></tr>\n");
        }
    }
    closedir(dir);

    /* Finish the file list table */
    httpd_resp_sendstr_chunk(req, "</tbody></table>");

    /* Send remaining chunk of HTML file to complete it */
    httpd_resp_sendstr_chunk(req, "</body></html>");

    /* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)


static esp_err_t logfileact_get_full_handler(httpd_req_t *req) {
    return send_logfile(req, true);
}


static esp_err_t logfileact_get_last_part_handler(httpd_req_t *req) {
    return send_logfile(req, false);
}

static esp_err_t datafileact_get_full_handler(httpd_req_t *req) {
    return send_datafile(req, true);
}


static esp_err_t datafileact_get_last_part_handler(httpd_req_t *req) {
    return send_datafile(req, false);
}

static esp_err_t send_datafile(httpd_req_t *req, bool send_full_file)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "data_get_last_part_handler");
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    //struct stat file_stat;
    ESP_LOGD(TAG, "uri: %s", req->uri);

    const char* filename = ""; 

    std::string currentfilename = LogFile.GetCurrentFileNameData();

    ESP_LOGD(TAG, "uri: %s, filename: %s, filepath: %s", req->uri, filename, filepath);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    fd = OpenFileAndWait(currentfilename.c_str(), "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

//    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", &filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    if (!send_full_file) { // Send only last part of file
        ESP_LOGD(TAG, "Sending last %d bytes of the actual datafile!", LOGFILE_LAST_PART_BYTES);

        /* Adapted from https://www.geeksforgeeks.org/implement-your-own-tail-read-last-n-lines-of-a-huge-file/ */
        if (fseek(fd, 0, SEEK_END)) {
            ESP_LOGE(TAG, "Failed to get to end of file!");
            return ESP_FAIL;
        }
        else {
            long pos = ftell(fd); // Number of bytes in the file
            ESP_LOGI(TAG, "File contains %ld bytes", pos);

            if (fseek(fd, pos - std::min((long)LOGFILE_LAST_PART_BYTES, pos), SEEK_SET)) { // Go LOGFILE_LAST_PART_BYTES bytes back from EOF
                ESP_LOGE(TAG, "Failed to go back %ld bytes within the file!", std::min((long)LOGFILE_LAST_PART_BYTES, pos));
                return ESP_FAIL;
            }
        }

        /* Find end of line */
        while (1) {
            if (fgetc(fd) == '\n') {
                break;
            }
        }
    }

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        /* Send the buffer contents as HTTP response chunk */
        if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
            fclose(fd);
            ESP_LOGE(TAG, "File sending failed!");
            /* Abort sending file */
            httpd_resp_sendstr_chunk(req, NULL);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
            return ESP_FAIL;
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


static esp_err_t send_logfile(httpd_req_t *req, bool send_full_file)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "log_get_last_part_handler");
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    //struct stat file_stat;
    ESP_LOGI(TAG, "uri: %s", req->uri);

    const char* filename = ""; 

    std::string currentfilename = LogFile.GetCurrentFileName();

    ESP_LOGD(TAG, "uri: %s, filename: %s, filepath: %s", req->uri, filename, filepath);


    fd = OpenFileAndWait(currentfilename.c_str(), "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

//    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", &filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    if (!send_full_file) { // Send only last part of file
        ESP_LOGD(TAG, "Sending last %d bytes of the actual logfile!", LOGFILE_LAST_PART_BYTES);

        /* Adapted from https://www.geeksforgeeks.org/implement-your-own-tail-read-last-n-lines-of-a-huge-file/ */
        if (fseek(fd, 0, SEEK_END)) {
            ESP_LOGE(TAG, "Failed to get to end of file!");
            return ESP_FAIL;
        }
        else {
            long pos = ftell(fd); // Number of bytes in the file
            ESP_LOGI(TAG, "File contains %ld bytes", pos);

            if (fseek(fd, pos - std::min((long)LOGFILE_LAST_PART_BYTES, pos), SEEK_SET)) { // Go LOGFILE_LAST_PART_BYTES bytes back from EOF
                ESP_LOGE(TAG, "Failed to go back %ld bytes within the file!", std::min((long)LOGFILE_LAST_PART_BYTES, pos));
                return ESP_FAIL;
            }
        }

        /* Find end of line */
        while (1) {
            if (fgetc(fd) == '\n') {
                break;
            }
        }
    }

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        /* Send the buffer contents as HTTP response chunk */
        if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
            fclose(fd);
            ESP_LOGE(TAG, "File sending failed!");
            /* Abort sending file */
            httpd_resp_sendstr_chunk(req, NULL);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
            return ESP_FAIL;
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


/* Handler to download a file kept on the server */
static esp_err_t download_get_handler(httpd_req_t *req)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "download_get_handler");
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;
    ESP_LOGD(TAG, "uri: %s", req->uri);

    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri  + sizeof("/fileserver") - 1, sizeof(filepath));    

    ESP_LOGD(TAG, "uri: %s, filename: %s, filepath: %s", req->uri, filename, filepath);

//    filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
//                                             req->uri, sizeof(filepath));


    if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* If name has trailing '/', respond with directory contents */
    if (filename[strlen(filename) - 1] == '/') {
        bool readonly = false;
        size_t buf_len = httpd_req_get_url_query_len(req) + 1;
        if (buf_len > 1) {
            char buf[buf_len];
            if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query => %s", buf);
                char param[32];
                /* Get value of expected key from query string */
                if (httpd_query_key_value(buf, "readonly", param, sizeof(param)) == ESP_OK) {
                    ESP_LOGI(TAG, "Found URL query parameter => readonly=%s", param);
                    readonly = param && strcmp(param,"true")==0;
                }
            }
        }

        ESP_LOGD(TAG, "uri: %s, filename: %s, filepath: %s", req->uri, filename, filepath);
        return http_resp_dir_html(req, filepath, filename, readonly);
    }

    std::string testwlan = toUpper(std::string(filename));

    if ((stat(filepath, &file_stat) == -1) || (testwlan.compare("/WLAN.INI") == 0 )) {  // wlan.ini soll nicht angezeigt werden!

        /* If file not present on SPIFFS check if URI
         * corresponds to one of the hardcoded paths */
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    fd = OpenFileAndWait(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        /* Send the buffer contents as HTTP response chunk */
        if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
            fclose(fd);
            ESP_LOGE(TAG, "File sending failed!");
            /* Abort sending file */
            httpd_resp_sendstr_chunk(req, NULL);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
            return ESP_FAIL;
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File successfully sent");

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/* Handler to upload a file onto the server */
static esp_err_t upload_post_handler(httpd_req_t *req)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "upload_post_handler");
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    /* Skip leading "/upload" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri + sizeof("/upload") - 1, sizeof(filepath));
    if (!filename) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/') {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == 0) {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        return ESP_FAIL;
    }

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE) {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File size must be less than "
                            MAX_FILE_SIZE_STR "!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    fd = OpenFileAndWait(filepath, "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Receiving file : %s...", filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf = ((struct file_server_data *)req->user_ctx)->scratch;
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0) {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        /* Write buffer content to file on storage */
        if (received && (received != fwrite(buf, 1, received, fd))) {
            /* Couldn't write everything to file!
             * Storage may be full? */
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File write failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
            return ESP_FAIL;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    fclose(fd);
    ESP_LOGI(TAG, "File reception complete");

    std::string directory = std::string(filepath);
	size_t zw = directory.find("/");
	size_t found = zw;
	while (zw != std::string::npos)
	{
		zw = directory.find("/", found+1);  
		if (zw != std::string::npos)
			found = zw;
	}

    int start_fn = strlen(((struct file_server_data *)req->user_ctx)->base_path);
    ESP_LOGD(TAG, "Directory: %s, start_fn: %d, found: %d", directory.c_str(), start_fn, found);
	directory = directory.substr(start_fn, found - start_fn + 1);
    directory = "/fileserver" + directory;
//    ESP_LOGD(TAG, "Directory danach 2: %s", directory.c_str());

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", directory.c_str());

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", directory.c_str());
    httpd_resp_sendstr(req, "File uploaded successfully");

/*
    if (strcmp(filepath, CONFIG_FILE) == 0) {
        ESP_LOGD(TAG, "New config found. Reload handler.");
        gpio_handler_deinit();
        MQTTdestroy();
    }
*/

    return ESP_OK;
}

/* Handler to delete a file from the server */
static esp_err_t delete_post_handler(httpd_req_t *req)
{
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "delete_post_handler");
    char filepath[FILE_PATH_MAX];
    struct stat file_stat;


//////////////////////////////////////////////////////////////
    char _query[200];
    char _valuechar[30];    
    std::string fn = "/sdcard/firmware/";
    std::string _task;
    std::string directory;
    std::string zw; 

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        ESP_LOGD(TAG, "Query: %s", _query);
        
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
            ESP_LOGD(TAG, "task is found: %s", _valuechar);
            _task = std::string(_valuechar);
        }
    }

    if (_task.compare("deldircontent") == 0)
    {
        /* Skip leading "/delete" from URI to get filename */
        /* Note sizeof() counts NULL termination hence the -1 */
        const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                                req->uri  + sizeof("/delete") - 1, sizeof(filepath));
        if (!filename) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
            return ESP_FAIL;
        }
        zw = std::string(filename);
        zw = zw.substr(0, zw.length()-1);
        directory = "/fileserver" + zw + "/";
        zw = "/sdcard" + zw;
        ESP_LOGD(TAG, "Directory to delete: %s", zw.c_str());

        delete_all_in_directory(zw);
//        directory = std::string(filepath);
//        directory = "/fileserver" + directory;
        ESP_LOGD(TAG, "Location after delete directory content: %s", directory.c_str());
        /* Redirect onto root to see the updated file list */
//        httpd_resp_set_status(req, "303 See Other");
//        httpd_resp_set_hdr(req, "Location", directory.c_str());
//        httpd_resp_sendstr(req, "File deleted successfully");
//        return ESP_OK;        
    }
    else
    {
        /* Skip leading "/delete" from URI to get filename */
        /* Note sizeof() counts NULL termination hence the -1 */
        const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                                req->uri  + sizeof("/delete") - 1, sizeof(filepath));
        if (!filename) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
            return ESP_FAIL;
        }

        /* Filename cannot have a trailing '/' */
        if (filename[strlen(filename) - 1] == '/') {
            ESP_LOGE(TAG, "Invalid filename : %s", filename);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
            return ESP_FAIL;
        }

        if (stat(filepath, &file_stat) == -1) {
            ESP_LOGE(TAG, "File does not exist : %s", filename);
            /* Respond with 400 Bad Request */
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Deleting file : %s", filename);
        /* Delete file */
        unlink(filepath);

        directory = std::string(filepath);
        size_t zw = directory.find("/");
        size_t found = zw;
        while (zw != std::string::npos)
        {
            zw = directory.find("/", found+1);  
            if (zw != std::string::npos)
                found = zw;
        }

        int start_fn = strlen(((struct file_server_data *)req->user_ctx)->base_path);
        ESP_LOGD(TAG, "Directory: %s, start_fn: %d, found: %d", directory.c_str(), start_fn, found);
        directory = directory.substr(start_fn, found - start_fn + 1);
        directory = "/fileserver" + directory;
        ESP_LOGD(TAG, "Directory danach 4: %s", directory.c_str());
    }
    




//////////////////////////////////////////////////////////////

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", directory.c_str());
    httpd_resp_sendstr(req, "File successfully deleted");
    return ESP_OK;
}


void delete_all_in_directory(std::string _directory)
{
    struct dirent *entry;
    DIR *dir = opendir(_directory.c_str());
    std::string filename;

    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", _directory.c_str());
        return;
    }

    /* Iterate over all files / folders and fetch their names and sizes */
    while ((entry = readdir(dir)) != NULL) {
        if (!(entry->d_type == DT_DIR)){
            if (strcmp("wlan.ini", entry->d_name) != 0){                    // auf wlan.ini soll nicht zugegriffen werden !!!
                filename = _directory + "/" + std::string(entry->d_name);
                ESP_LOGI(TAG, "Deleting file : %s", filename.c_str());
                /* Delete file */
                unlink(filename.c_str());    
            }
        };
    }
    closedir(dir);
}

std::string unzip_new(std::string _in_zip_file, std::string _target_zip, std::string _target_bin, std::string _main)
{
    int i, sort_iter;
    mz_bool status;
    size_t uncomp_size;
    mz_zip_archive zip_archive;
    void* p;
    char archive_filename[64];
    std::string zw, ret = "";
    std::string directory = "";
//    static const char* s_Test_archive_filename = "testhtml.zip";

    ESP_LOGD(TAG, "miniz.c version: %s", MZ_VERSION);
    ESP_LOGD(TAG, "Zipfile: %s", _in_zip_file.c_str());
//    ESP_LOGD(TAG, "Target Dir ZIP: %s", _target_zip.c_str());
//    ESP_LOGD(TAG, "Target Dir BIN: %s", _target_bin.c_str());
//    ESP_LOGD(TAG, "Target Dir main: %s", _main.c_str());

    // Now try to open the archive.
    memset(&zip_archive, 0, sizeof(zip_archive));
    status = mz_zip_reader_init_file(&zip_archive, _in_zip_file.c_str(), 0);
    if (!status)
    {
        ESP_LOGD(TAG, "mz_zip_reader_init_file() failed!");
        return ret;
    }

    // Get and print information about each file in the archive.
    int numberoffiles = (int)mz_zip_reader_get_num_files(&zip_archive);
    for (sort_iter = 0; sort_iter < 2; sort_iter++)
    {
        memset(&zip_archive, 0, sizeof(zip_archive));
        status = mz_zip_reader_init_file(&zip_archive, _in_zip_file.c_str(), sort_iter ? MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY : 0);
        if (!status)
        {
            ESP_LOGD(TAG, "mz_zip_reader_init_file() failed!");
            return ret;
        }

        for (i = 0; i < numberoffiles; i++)
        {
            mz_zip_archive_file_stat file_stat;
            mz_zip_reader_file_stat(&zip_archive, i, &file_stat);
            sprintf(archive_filename, file_stat.m_filename);
            
            if (!file_stat.m_is_directory) {
            // Try to extract all the files to the heap.
            p = mz_zip_reader_extract_file_to_heap(&zip_archive, archive_filename, &uncomp_size, 0);
                if (!p)
                {
                    ESP_LOGE(TAG, "mz_zip_reader_extract_file_to_heap() failed on file %s", archive_filename);
                    mz_zip_reader_end(&zip_archive);
                    return ret;
                }
            
                // Save to File.
                zw = std::string(archive_filename);
                ESP_LOGD(TAG, "Rohfilename: %s", zw.c_str());

                if (toUpper(zw) == "FIRMWARE.BIN")
                {
                    zw = _target_bin + zw;
                    ret = zw;
                }
                else
                {
                    std::string _dir = getDirectory(zw);

                    if (_dir.length() > 0)
                    {
                        zw = _main + zw;
                    }
                    else
                    {
                        zw = _target_zip + zw;
                    }

                }
            
                string filename_zw = zw + SUFFIX_ZW;

                ESP_LOGI(TAG, "Filename to extract: %s, Zwischenfilename: %s", zw.c_str(), filename_zw.c_str());

                // extrahieren in zwischendatei
                DeleteFile(filename_zw);
                FILE* fpTargetFile = OpenFileAndWait(filename_zw.c_str(), "wb");
                uint writtenbytes = fwrite(p, 1, (uint)uncomp_size, fpTargetFile);
                fclose(fpTargetFile);
                
                bool isokay = true;

                if (writtenbytes == (uint)uncomp_size)
                {
                    isokay = true;
                }
                else
                {
                    isokay = false;
                    ESP_LOGD(TAG, "ERROR in writting extracted file (function fwrite) extracted file \"%s\", size %u", archive_filename, (uint)uncomp_size);
                }

                DeleteFile(zw);
                if (!isokay)
                    ESP_LOGE(TAG, "ERROR in fwrite \"%s\", size %u", archive_filename, (uint)uncomp_size);
                isokay = isokay && RenameFile(filename_zw, zw);
                if (!isokay)
                    ESP_LOGE(TAG, "ERROR in Rename \"%s\" to \"%s\"", filename_zw.c_str(), zw.c_str());
//                isokay = isokay && DeleteFile(filename_zw);
//                if (!isokay)
//                    ESP_LOGE(TAG, "ERROR in Delete \"%s\"", filename_zw.c_str());

                if (isokay)
                    ESP_LOGI(TAG, "Successfully extracted file \"%s\", size %u", archive_filename, (uint)uncomp_size);
                else
                {
                    ESP_LOGE(TAG, "ERROR in extracting file \"%s\", size %u", archive_filename, (uint)uncomp_size);
                    ret = "ERROR";
                }
                mz_free(p);
            }
        }

        // Close the archive, freeing any resources it was using
        mz_zip_reader_end(&zip_archive);
    }

    ESP_LOGD(TAG, "Success.");
    return ret;
}

void unzip(std::string _in_zip_file, std::string _target_directory){
    int i, sort_iter;
    mz_bool status;
    size_t uncomp_size;
    mz_zip_archive zip_archive;
    void* p;
    char archive_filename[64];
    std::string zw;
//    static const char* s_Test_archive_filename = "testhtml.zip";

    ESP_LOGD(TAG, "miniz.c version: %s", MZ_VERSION);
    ESP_LOGD(TAG, "Zipfile: %s", _in_zip_file.c_str());
    ESP_LOGD(TAG, "Target Dir: %s", _target_directory.c_str());

    // Now try to open the archive.
    memset(&zip_archive, 0, sizeof(zip_archive));
    status = mz_zip_reader_init_file(&zip_archive, _in_zip_file.c_str(), 0);
    if (!status)
    {
        ESP_LOGD(TAG, "mz_zip_reader_init_file() failed!");
        return;
    }

    // Get and print information about each file in the archive.
    int numberoffiles = (int)mz_zip_reader_get_num_files(&zip_archive);
    for (sort_iter = 0; sort_iter < 2; sort_iter++)
    {
        memset(&zip_archive, 0, sizeof(zip_archive));
        status = mz_zip_reader_init_file(&zip_archive, _in_zip_file.c_str(), sort_iter ? MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY : 0);
        if (!status)
        {
            ESP_LOGD(TAG, "mz_zip_reader_init_file() failed!");
            return;
        }

        for (i = 0; i < numberoffiles; i++)
        {
            mz_zip_archive_file_stat file_stat;
            mz_zip_reader_file_stat(&zip_archive, i, &file_stat);
            sprintf(archive_filename, file_stat.m_filename);
 
            // Try to extract all the files to the heap.
            p = mz_zip_reader_extract_file_to_heap(&zip_archive, archive_filename, &uncomp_size, 0);
            if (!p)
            {
                ESP_LOGD(TAG, "mz_zip_reader_extract_file_to_heap() failed!");
                mz_zip_reader_end(&zip_archive);
                return;
            }

            // Save to File.
            zw = std::string(archive_filename);
            zw = _target_directory + zw;
            ESP_LOGD(TAG, "Filename to extract: %s", zw.c_str());
            FILE* fpTargetFile = OpenFileAndWait(zw.c_str(), "wb");
            fwrite(p, 1, (uint)uncomp_size, fpTargetFile);
            fclose(fpTargetFile);

            ESP_LOGD(TAG, "Successfully extracted file \"%s\", size %u", archive_filename, (uint)uncomp_size);
            //            ESP_LOGD(TAG, "File data: \"%s\"", (const char*)p);

            // We're done.
            mz_free(p);
        }

        // Close the archive, freeing any resources it was using
        mz_zip_reader_end(&zip_archive);
    }

    ESP_LOGD(TAG, "Success.");
}



void register_server_file_uri(httpd_handle_t server, const char *base_path)
{
    static struct file_server_data *server_data = NULL;

    /* Validate file storage base path */
    if (!base_path) {
//    if (!base_path || strcmp(base_path, "/spiffs") != 0) {
        ESP_LOGE(TAG, "File server base_path not set");
//        return ESP_ERR_INVALID_ARG;
    }

    if (server_data) {
        ESP_LOGE(TAG, "File server already started");
//        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = (file_server_data *) calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
//        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));



    /* URI handler for getting uploaded files */
//    char zw[sizeof(serverprefix)+1];
//    strcpy(zw, serverprefix);
//    zw[strlen(serverprefix)] = '*';
//    zw[strlen(serverprefix)+1] = '\0';    
//    ESP_LOGD(TAG, "zw: %s", zw);
    httpd_uri_t file_download = {
        .uri       = "/fileserver*",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = download_get_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_download);


    httpd_uri_t file_datafileact = {
        .uri       = "/datafileact",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = datafileact_get_full_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_datafileact);


    httpd_uri_t file_datafile_last_part_handle = {
        .uri       = "/data",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = datafileact_get_last_part_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_datafile_last_part_handle);

    httpd_uri_t file_logfileact = {
        .uri       = "/logfileact",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = logfileact_get_full_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_logfileact);


    httpd_uri_t file_logfile_last_part_handle = {
        .uri       = "/log",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = logfileact_get_last_part_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_logfile_last_part_handle);


    /* URI handler for uploading files to server */
    httpd_uri_t file_upload = {
        .uri       = "/upload/*",   // Match all URIs of type /upload/path/to/file
        .method    = HTTP_POST,
        .handler   = upload_post_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_upload);

    /* URI handler for deleting files from server */
    httpd_uri_t file_delete = {
        .uri       = "/delete/*",   // Match all URIs of type /delete/path/to/file
        .method    = HTTP_POST,
        .handler   = delete_post_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_delete);

}
