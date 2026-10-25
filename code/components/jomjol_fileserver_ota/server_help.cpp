#include "server_help.h"

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "miniz.h"

#include "Helper.h"
#include "ClassLogFile.h"
#include "../../include/defines.h"

static const char *TAG = "SERVER HELP";

string SUFFIX_ZW = "_tmp";

char scratch[SERVER_HELPER_SCRATCH_BUFSIZE];

bool endsWith(std::string const &str, std::string const &suffix)
{
    if (str.length() < suffix.length())
    {
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
    if (stat(_filename_temp.c_str(), &file_stat) == 0)
    {
        filename = _filename_temp;

        ESP_LOGD(TAG, "new filename: %s", filename.c_str());
        _gz_file_exists = true;
    }

    FILE *fd = fopen(filename.c_str(), "r");
    if (!fd)
    {
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
        endsWith(filename, ".gz"))
    {
        if (filename == "/sdcard/html/setup.html")
        {
            httpd_resp_set_hdr(req, "Clear-Site-Data", "\"*\"");
            set_content_type_from_file(req, filename.c_str());
        }
        else if (_gz_file_exists)
        {
            httpd_resp_set_hdr(req, "Cache-Control", "max-age=43200");
            httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
            set_content_type_from_file(req, _filename_old.c_str());
        }
        else
        {
            httpd_resp_set_hdr(req, "Cache-Control", "max-age=43200");
            set_content_type_from_file(req, filename.c_str());
        }
    }
    else
    {
        set_content_type_from_file(req, filename.c_str());
    }

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = scratch;
    size_t chunksize;

    do
    {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SERVER_HELPER_SCRATCH_BUFSIZE, fd);

        /* Send the buffer contents as HTTP response chunk */
        if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK)
        {
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
const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest)
    {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash)
    {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize)
    {
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
    if (IS_FILE_EXT(filename, ".pdf"))
    {
        return httpd_resp_set_type(req, "application/x-pdf");
    }
    else if (IS_FILE_EXT(filename, ".htm"))
    {
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(filename, ".html"))
    {
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(filename, ".jpeg"))
    {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(filename, ".jpg"))
    {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(filename, ".gif"))
    {
        return httpd_resp_set_type(req, "image/gif");
    }
    else if (IS_FILE_EXT(filename, ".png"))
    {
        return httpd_resp_set_type(req, "image/png");
    }
    else if (IS_FILE_EXT(filename, ".ico"))
    {
        return httpd_resp_set_type(req, "image/x-icon");
    }
    else if (IS_FILE_EXT(filename, ".js"))
    {
        return httpd_resp_set_type(req, "application/javascript");
    }
    else if (IS_FILE_EXT(filename, ".css"))
    {
        return httpd_resp_set_type(req, "text/css");
    }
    else if (IS_FILE_EXT(filename, ".xml"))
    {
        return httpd_resp_set_type(req, "text/xml");
    }
    else if (IS_FILE_EXT(filename, ".zip"))
    {
        return httpd_resp_set_type(req, "application/x-zip");
    }
    else if (IS_FILE_EXT(filename, ".gz"))
    {
        return httpd_resp_set_type(req, "application/x-gzip");
    }

    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

struct unzip_context_t
{
    FILE *file;
};

static size_t unzip_write_callback(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
    (void)file_ofs;

    unzip_context_t *ctx = (unzip_context_t *)pOpaque;
    if (!ctx || !ctx->file)
    {
        return 0;
    }

    size_t written = fwrite(pBuf, 1, n, ctx->file);
    if (written != n)
    {
        return 0;
    }

    return written;
}

static bool unzip_extract_file(mz_zip_archive *zip_archive, int i, const std::string &output_file, const mz_zip_archive_file_stat &file_stat)
{
    DeleteFile(output_file);

    FILE *pFile = fopen(output_file.c_str(), "wb");
    if (!pFile)
    {
        return false;
    }

    setvbuf(pFile, NULL, _IOFBF, SERVER_FILER_SCRATCH_BUFSIZE);

    unzip_context_t ctx;
    ctx.file = pFile;

    mz_bool status = mz_zip_reader_extract_to_callback(zip_archive, i, unzip_write_callback, &ctx, 0);

    fclose(pFile);

    if (!status)
    {
        DeleteFile(output_file);
        return false;
    }

    return true;
}

void unzip_file(std::string _in_zip_file, std::string _target_directory)
{
    mz_zip_archive zip_archive;

    ESP_LOGD(TAG, "Zipfile: %s", _in_zip_file.c_str());
    ESP_LOGD(TAG, "Target Dir: %s", _target_directory.c_str());

    memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_reader_init_file(&zip_archive, _in_zip_file.c_str(), 0))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "mz_zip_reader_init_file() failed!");
        return;
    }

    int numberoffiles = (int)mz_zip_reader_get_num_files(&zip_archive);

    for (int i = 0; i < numberoffiles; i++)
    {
        mz_zip_archive_file_stat file_stat;

        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
        {
            continue;
        }

        if (file_stat.m_is_directory)
        {
            continue;
        }

        std::string out_file = _target_directory + std::string(file_stat.m_filename);

        ESP_LOGD(TAG, "Extract: %s", out_file.c_str());

        bool ok = unzip_extract_file(&zip_archive, i, out_file, file_stat);

        if (!ok)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Extract failed: " + std::string(file_stat.m_filename));
            continue;
        }
    }

    mz_zip_reader_end(&zip_archive);
}

std::string unzip_firmware(std::string _in_zip_file, std::string _html_tmp, std::string _html_final, std::string _target_bin, std::string _main, bool _initial_setup)
{
    mz_zip_archive zip_archive;

    ESP_LOGD(TAG, "Zipfile: %s", _in_zip_file.c_str());

    memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_reader_init_file(&zip_archive, _in_zip_file.c_str(), 0))
    {
        ESP_LOGD(TAG, "mz_zip_reader_init_file() failed!");
        return "";
    }

    int numberoffiles = (int)mz_zip_reader_get_num_files(&zip_archive);

    std::string ret = "";

    for (int i = 0; i < numberoffiles; i++)
    {
        mz_zip_archive_file_stat file_stat;

        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
        {
            continue;
        }

        if (file_stat.m_is_directory)
        {
            continue;
        }

        std::string path(file_stat.m_filename);

        std::string logical_path;

        if (toUpper(path) == "FIRMWARE.BIN")
        {
            logical_path = _target_bin + path;
            ret = logical_path;
        }
        else
        {
            std::string dir = getDirectory(path);

            if (dir == "config-initial" && !_initial_setup)
            {
                continue;
            }

            std::string modified = path;
            std::string _s1 = "config-initial";
            std::string _dir_new = "config";
            FindReplace(modified, _s1, _dir_new);

            if (!dir.empty())
            {
                logical_path = _main + modified;
            }
            else
            {
                logical_path = _html_tmp + modified;
            }
        }

        if (logical_path.find(_html_final) == 0)
        {
            FindReplace(logical_path, _html_final, _html_tmp);
        }

        std::string temp_file = logical_path + SUFFIX_ZW;

        std::string folder = temp_file.substr(0, temp_file.find_last_of('/'));
        MakeDir(folder);

        ESP_LOGI(TAG, "Extract: %s", temp_file.c_str());

        bool ok = unzip_extract_file(&zip_archive, i, temp_file, file_stat);

        if (!ok)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Extract failed: " + std::string(file_stat.m_filename));
            ret = "ERROR";
            continue;
        }

        DeleteFile(logical_path);

        if (!RenameFile(temp_file, logical_path))
        {
            DeleteFile(temp_file);
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Rename failed: " + temp_file);
            ret = "ERROR";
            continue;
        }
    }

    mz_zip_reader_end(&zip_archive);

    return ret;
}
