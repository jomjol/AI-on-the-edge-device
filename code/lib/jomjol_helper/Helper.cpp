//#pragma warning(disable : 4996)

#include "Helper.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <esp_log.h>

//#define ISWINDOWS_TRUE
#define PATH_MAX_STRING_SIZE 256

using namespace std;

std::string FormatFileName(std::string input)
{
#ifdef ISWINDOWS_TRUE
    input.erase(0, 1);
    std::string os = "/";
    std::string ns = "\\";
    FindReplace(input, os, ns);
#endif
    return input;
}




void FindReplace(std::string& line, std::string& oldString, std::string& newString) {
    const size_t oldSize = oldString.length();

    // do nothing if line is shorter than the string to find
    if (oldSize > line.length()) return;

    const size_t newSize = newString.length();
    for (size_t pos = 0; ; pos += newSize) {
        // Locate the substring to replace
        pos = line.find(oldString, pos);
        if (pos == std::string::npos) return;
        if (oldSize == newSize) {
            // if they're same size, use std::string::replace
            line.replace(pos, oldSize, newString);
        }
        else {
            // if not same size, replace by erasing and inserting
            line.erase(pos, oldSize);
            line.insert(pos, newString);
        }
    }
}




bool ctype_space(const char c, string adddelimiter)
{
	if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 11)
	{
		return true;
	}
	if (adddelimiter.find(c) != string::npos)
		return true;

	return false;
}

string trim(string istring, string adddelimiter)
{
	bool trimmed = false;

	if (ctype_space(istring[istring.length() - 1], adddelimiter))
	{
		istring.erase(istring.length() - 1);
		trimmed = true;
	}

	if (ctype_space(istring[0], adddelimiter))
	{
		istring.erase(0, 1);
		trimmed = true;
	}

	if ((trimmed == false) || (istring.size() == 0))
	{
		return istring;
	}
	else
	{
		return trim(istring, adddelimiter);
	}
}

size_t findDelimiterPos(string input, string delimiter)
{
	size_t pos = std::string::npos;
	size_t zw;
	string akt_del;

	for (int anz = 0; anz < delimiter.length(); ++anz)
	{
		akt_del = delimiter[anz];
		if ((zw = input.find(akt_del)) != std::string::npos)
		{
			if (pos != std::string::npos)
			{
				if (zw < pos)
					pos = zw;
			}
			else
				pos = zw;
		}
	}
	return pos;
}


void CopyFile(string input, string output)
{
	input = FormatFileName(input);
	output = FormatFileName(output);

	if (toUpper(input).compare("/SDCARD/WLAN.INI") == 0)
	{
		printf("wlan.ini kann nicht kopiert werden!\n");
		return;
	}

	char cTemp;
	FILE* fpSourceFile = fopen(input.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		printf("File %s existiert nicht!\n", input.c_str());
		return;
	}

	FILE* fpTargetFile = fopen(output.c_str(), "wb");

	// Code Section

	// Read From The Source File - "Copy"
	while (fread(&cTemp, 1, 1, fpSourceFile) == 1)
	{
		// Write To The Target File - "Paste"
		fwrite(&cTemp, 1, 1, fpTargetFile);
	}

	// Close The Files
	fclose(fpSourceFile);
	fclose(fpTargetFile);
}


string getFileType(string filename)
{
	int lastpos = filename.find(".", 0);
	int neu_pos;
	while ((neu_pos = filename.find(".", lastpos + 1)) > -1)
	{
		lastpos = neu_pos;
	}

	string zw = filename.substr(lastpos + 1, filename.size() - lastpos);
	zw = toUpper(zw);

	return zw;
}

/* recursive mkdir */
int mkdir_r(const char *dir, const mode_t mode) {
    char tmp[PATH_MAX_STRING_SIZE];
    char *p = NULL;
    struct stat sb;
    size_t len;
    
    /* copy path */
    len = strnlen (dir, PATH_MAX_STRING_SIZE);
    if (len == 0 || len == PATH_MAX_STRING_SIZE) {
        return -1;
    }
    memcpy (tmp, dir, len);
    tmp[len] = '\0';

    /* remove trailing slash */
    if(tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    /* check if path exists and is a directory */
    if (stat (tmp, &sb) == 0) {
        if (S_ISDIR (sb.st_mode)) {
            return 0;
        }
    }
    
    /* recursive mkdir */
    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            /* test path */
            if (stat(tmp, &sb) != 0) {
                /* path does not exist - create directory */
                if (mkdir(tmp, mode) < 0) {
                    return -1;
                }
            } else if (!S_ISDIR(sb.st_mode)) {
                /* not a directory */
                return -1;
            }
            *p = '/';
        }
    }
    /* test path */
    if (stat(tmp, &sb) != 0) {
        /* path does not exist - create directory */
        if (mkdir(tmp, mode) < 0) {
            return -1;
        }
    } else if (!S_ISDIR(sb.st_mode)) {
        /* not a directory */
        return -1;
    }
    return 0;
}

string toUpper(string in)
{
	for (int i = 0; i < in.length(); ++i)
		in[i] = toupper(in[i]);
	
	return in;
}

// CPU Temp
extern "C" uint8_t temprature_sens_read();
float temperatureRead()
{
    return (temprature_sens_read() - 32) / 1.8;
}

time_t addDays(time_t startTime, int days) {
	struct tm* tm = localtime(&startTime);
	tm->tm_mday += days;
	return mktime(tm);
}

int removeFolder(const char* folderPath, const char* logTag) {
	ESP_LOGI(logTag, "Delete folder %s", folderPath);

	DIR *dir = opendir(folderPath);
    if (!dir) {
        ESP_LOGI(logTag, "Failed to stat dir : %s", folderPath);
        return -1;
    }

    struct dirent *entry;
    int deleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        std::string path = string(folderPath) + "/" + entry->d_name;
		if (entry->d_type == DT_REG) {
			if (unlink(path.c_str()) == 0) {
				deleted ++;
			} else {
				ESP_LOGE(logTag, "can't delete file : %s", path.c_str());
			}
        } else if (entry->d_type == DT_DIR) {
			deleted += removeFolder(path.c_str(), logTag);
		}
    }
    
    closedir(dir);
	if (rmdir(folderPath) != 0) {
		ESP_LOGE(logTag, "can't delete file : %s", folderPath);
	}
	ESP_LOGI(logTag, "%d older log files in folder %s deleted.", deleted, folderPath);

	return deleted;
}
