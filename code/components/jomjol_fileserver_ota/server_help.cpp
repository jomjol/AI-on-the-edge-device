#include "server_help.h"

#include <stdio.h>
#include <string.h>
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
#include "Helper.h"
#include "esp_http_server.h"
#include "../../include/defines.h"

static const char *TAG = "SERVER HELP";

char scratch[SERVER_HELPER_SCRATCH_BUFSIZE];

bool endsWith(std::string const &str, std::string const &suffix) 
{
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

esp_err_t send_file(httpd_req_t *req, std::string filename)
{
    std::string _filename_old = filename;
    struct stat file_stat;
    bool _gz_file_exists = false;

    ESP_LOGD(TAG, "old filename: %s", filename.c_str());
    std::string _filename_temp = std::string(filename) + ".gz";

    // Checks whether the file is available as .gz
    if (stat(_filename_temp.c_str(), &file_stat) == 0) {
        filename = _filename_temp;

        ESP_LOGD(TAG, "new filename: %s", filename.c_str());
        _gz_file_exists = true;
    }

    FILE *fd = fopen(filename.c_str(), "r");
    if (!fd)  {
        ESP_LOGE(TAG, "Failed to read file: %s", filename.c_str());
		
        /* Respond with 404 Error */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, get404());
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "Sending file: %s ...", filename.c_str());

    /* For all files with the following file extention tell the webbrowser to cache them for 12h */
    if (endsWith(filename, ".html") ||
        endsWith(filename, ".htm") ||
        endsWith(filename, ".xml") ||
        endsWith(filename, ".css") ||
        endsWith(filename, ".js") ||
        endsWith(filename, ".map") ||
        endsWith(filename, ".jpg") ||
        endsWith(filename, ".jpeg") ||
        endsWith(filename, ".ico") ||
        endsWith(filename, ".png") ||
        endsWith(filename, ".gif") ||
        // endsWith(filename, ".zip") ||
        endsWith(filename, ".gz"))	{
        if (filename == "/sdcard/html/setup.html") {
            httpd_resp_set_hdr(req, "Clear-Site-Data", "\"*\"");
            set_content_type_from_file(req, filename.c_str());
        }
        else if (_gz_file_exists) {
            httpd_resp_set_hdr(req, "Cache-Control", "max-age=43200");
            httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
            set_content_type_from_file(req, _filename_old.c_str());
        }
        else {
            httpd_resp_set_hdr(req, "Cache-Control", "max-age=43200");
            set_content_type_from_file(req, filename.c_str());
        }
    }
    else {
        set_content_type_from_file(req, filename.c_str());
    }

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = scratch;
    size_t chunksize;
	
    do  {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SERVER_HELPER_SCRATCH_BUFSIZE, fd);

        /* Send the buffer contents as HTTP response chunk */
        if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK)  {
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
    ESP_LOGD(TAG, "File sending complete");
	
    return ESP_OK;    
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

/* Set HTTP response content type according to file extension */
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        return httpd_resp_set_type(req, "application/x-pdf");
    }
    else if (IS_FILE_EXT(filename, ".htm")) {
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(filename, ".html")) {
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(filename, ".jpeg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(filename, ".jpg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(filename, ".gif")) {
        return httpd_resp_set_type(req, "image/gif");
    }
    else if (IS_FILE_EXT(filename, ".png")) {
        return httpd_resp_set_type(req, "image/png");
    }
    else if (IS_FILE_EXT(filename, ".ico")) {
        return httpd_resp_set_type(req, "image/x-icon");
    }
    else if (IS_FILE_EXT(filename, ".js")) {
        return httpd_resp_set_type(req, "application/javascript");
    }
    else if (IS_FILE_EXT(filename, ".css")) {
        return httpd_resp_set_type(req, "text/css");
    }
    else if (IS_FILE_EXT(filename, ".xml")) {
        return httpd_resp_set_type(req, "text/xml");
    }
    else if (IS_FILE_EXT(filename, ".zip")) {
        return httpd_resp_set_type(req, "application/x-zip");
    }
    else if (IS_FILE_EXT(filename, ".gz")) {
        return httpd_resp_set_type(req, "application/x-gzip");
    }

    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}
