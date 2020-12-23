#include "ClassLogFile.h"
#include "time_sntp.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "Helper.h"

static const char *TAG = "log";

ClassLogFile LogFile("/sdcard/log/message", "log_%Y-%m-%d.txt");

void ClassLogFile::WriteToDedicatedFile(std::string _fn, std::string info, bool _time)
{
    FILE* pFile;
    std::string zwtime;

    if (!doLogFile){
        return;
    }

//    pFile = OpenFileAndWait(_fn.c_str(), "a"); 
    pFile = fopen(_fn.c_str(), "a+");
    printf("Logfile opened: %s\n", _fn.c_str());

    if (pFile!=NULL) {
        if (_time)
        {
            time_t rawtime;
            struct tm* timeinfo;
            char buffer[80];

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S", timeinfo);

            zwtime = std::string(buffer);
            info = zwtime + ": " + info;
        }
        fputs(info.c_str(), pFile);
        fputs("\n", pFile);

        fclose(pFile);    
    } else {
        ESP_LOGI(TAG, "Can't open log file %s", _fn.c_str());
    }
}

void ClassLogFile::SwitchOnOff(bool _doLogFile){
    doLogFile = _doLogFile;
};

void ClassLogFile::SetRetention(unsigned short _retentionInDays){
    retentionInDays = _retentionInDays;
};

void ClassLogFile::WriteToFile(std::string info, bool _time)
{
/*
    struct stat path_stat;
    if (stat(logroot.c_str(), &path_stat) != 0) {
        ESP_LOGI(TAG, "Create log folder: %s", logroot.c_str());
        if (mkdir_r(logroot.c_str(), S_IRWXU) == -1)  {
            ESP_LOGI(TAG, "Can't create log foolder");
        }
    }
*/
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[30];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 30, logfile.c_str(), timeinfo);
    std::string logpath = logroot + "/" + buffer; 
    
    WriteToDedicatedFile(logpath, info, _time);
}

std::string ClassLogFile::GetCurrentFileName()
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[60];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 60, logfile.c_str(), timeinfo);
    std::string logpath = logroot + "/" + buffer; 

    return logpath;
}

void ClassLogFile::RemoveOld()
{
    if (retentionInDays == 0) {
        return;
    }

    time_t rawtime;
    struct tm* timeinfo;
    char cmpfilename[30];

    time(&rawtime);
    rawtime = addDays(rawtime, -retentionInDays);
    timeinfo = localtime(&rawtime);
    
    strftime(cmpfilename, 30, logfile.c_str(), timeinfo);
    //ESP_LOGE(TAG, "log file name to compare: %s", cmpfilename);

    DIR *dir = opendir(logroot.c_str());
    if (!dir) {
        ESP_LOGI(TAG, "Failed to stat dir : %s", logroot.c_str());
        return;
    }

    struct dirent *entry;
    int deleted = 0;
    int notDeleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            //ESP_LOGI(TAG, "list log file : %s %s", entry->d_name, cmpfilename);
            if ((strlen(entry->d_name) == strlen(cmpfilename)) && (strcmp(entry->d_name, cmpfilename) < 0)) {
                ESP_LOGI(TAG, "delete log file : %s", entry->d_name);
                std::string filepath = logroot + "/" + entry->d_name; 
                if (unlink(filepath.c_str()) == 0) {
                    deleted ++;
                } else {
                    ESP_LOGE(TAG, "can't delete file : %s", entry->d_name);
                }
            } else {
                notDeleted ++;
            }
        }
    }
    ESP_LOGI(TAG, "%d older log files deleted. %d current log files not deleted.", deleted, notDeleted);
    closedir(dir);
}

ClassLogFile::ClassLogFile(std::string _logroot, std::string _logfile)
{
    logroot = _logroot;
    logfile =  _logfile;
    doLogFile = true;
    retentionInDays = 10;
}
