#include "ClassLogFile.h"
#include "time_sntp.h"
#include "esp_log.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <dirent.h>
#ifdef __cplusplus
}
#endif

#include "Helper.h"

static const char *TAG = "log";

ClassLogFile LogFile("/sdcard/log/message", "log_%Y-%m-%d.txt", "/sdcard/log/data", "data_%Y-%m-%d.txt");

void ClassLogFile::WriteHeapInfo(std::string _id)
{
    std::string _zw = "\t" + _id;
    if (loglevel > ESP_LOG_WARN) 
        _zw =  _zw + "\t" + getESPHeapInfo();

    WriteToFile(ESP_LOG_DEBUG, _zw);
}


std::string ClassLogFile::getESPHeapInfo(){
	string espInfoResultStr = "";
	char aMsgBuf[80];
    
	multi_heap_info_t aMultiHead_info ;
	heap_caps_get_info (&aMultiHead_info,MALLOC_CAP_8BIT);
	size_t aFreeHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT);
	size_t aMinFreeHeadSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
	size_t aMinFreeHeapSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
	size_t aHeapLargestFreeBlockSize = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
	sprintf(aMsgBuf,"Free Heap Size: \t%ld", (long) aFreeHeapSize);
	size_t aFreeSPIHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_SPIRAM);
 	size_t aFreeInternalHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_INTERNAL);
	size_t aMinFreeInternalHeapSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_INTERNAL);

	sprintf(aMsgBuf,"\tHeap:\t%ld", (long) aFreeHeapSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf,"\tMin Free:\t%ld", (long) aMinFreeHeapSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf,"\tlarg. Block: \t%ld", (long) aHeapLargestFreeBlockSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf,"\tSPI Heap:\t%ld", (long) aFreeSPIHeapSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf,"\tMin Free Heap Size:\t%ld", (long) aMinFreeHeadSize);
	sprintf(aMsgBuf,"\tNOT_SPI Heap:\t%ld", (long) (aFreeHeapSize - aFreeSPIHeapSize));
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf,"\tlargest Block Size: \t%ld", (long) aHeapLargestFreeBlockSize);
	sprintf(aMsgBuf,"\tInternal Heap:\t%ld", (long) (aFreeInternalHeapSize));
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf,"\tInternal Min Heap free:\t%ld", (long) (aMinFreeInternalHeapSize));
	espInfoResultStr += string(aMsgBuf);
	return 	espInfoResultStr;
}

void ClassLogFile::WriteToData(std::string _ReturnRawValue, std::string _ReturnValue, std::string _ReturnPreValue, std::string _ErrorMessageText, std::string _digital, std::string _analog)
{
    ESP_LOGD(TAG, "Start WriteToData");
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[30];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 30, datafile.c_str(), timeinfo);
    std::string logpath = dataroot + "/" + buffer; 
    
    FILE* pFile;
    std::string zwtime;

    if (!doLogFile){
        return;
    }

    ESP_LOGD(TAG, "Datalogfile: %s", logpath.c_str());
    pFile = fopen(logpath.c_str(), "a+");

    if (pFile!=NULL) {
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, 80, "%Y-%m-%dT%H:%M:%S", timeinfo);

        zwtime = std::string(buffer) + ":\t";
        fputs(zwtime.c_str(), pFile);
        fputs(_ReturnRawValue.c_str(), pFile);
        fputs("\t", pFile);
        fputs(_ReturnValue.c_str(), pFile);
        fputs("\t", pFile);
        fputs(_ReturnPreValue.c_str(), pFile);
        fputs("\t", pFile);
        fputs(_ErrorMessageText.c_str(), pFile);
        fputs("\t", pFile);
        fputs(_digital.c_str(), pFile);
        fputs("\t", pFile);
        fputs(_analog.c_str(), pFile);
        fputs("\t", pFile);
        fputs("\n", pFile);

        fclose(pFile);    
    } else {
        ESP_LOGE(TAG, "Can't open data file %s", logpath.c_str());
    }

}


void ClassLogFile::WriteToDedicatedFile(std::string _fn, esp_log_level_t level, std::string info, bool _time)
{
    FILE* pFile;
    std::string zwtime;
    std::string logline = "";

    if (!doLogFile){
        return;
    }

//    pFile = OpenFileAndWait(_fn.c_str(), "a"); 
    pFile = fopen(_fn.c_str(), "a+");
//    ESP_LOGD(TAG, "Logfile opened: %s", _fn.c_str());

    if (pFile!=NULL) {
        if (_time)
        {
            time_t rawtime;
            struct tm* timeinfo;
            char buffer[80];

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer, 80, "%Y-%m-%dT%H:%M:%S", timeinfo);

            zwtime = std::string(buffer);
            logline = zwtime;
        }

        std::string loglevelString; 
        switch(level) {
            case  ESP_LOG_NONE:
                loglevelString = "NONE";
                break;
            case  ESP_LOG_ERROR:
                loglevelString = "ERR";
                break;
            case  ESP_LOG_WARN:
                loglevelString = "WRN";
                break;
            case  ESP_LOG_INFO:
                loglevelString = "INF";
                break;
            case  ESP_LOG_DEBUG:
                loglevelString = "DBG";
                break;
            case  ESP_LOG_VERBOSE:
                loglevelString = "VER";
                break;
            default:
                loglevelString = "NONE";
                break;
        }
        
        logline = logline + "\t<" + loglevelString + ">\t" + info.c_str() + "\n";
        fputs(logline.c_str(), pFile);
        fclose(pFile);    
    } else {
        ESP_LOGE(TAG, "Can't open log file %s", _fn.c_str());
    }
}

void ClassLogFile::SwitchOnOff(bool _doLogFile){
    doLogFile = _doLogFile;
};

void ClassLogFile::SetRetention(unsigned short _retentionInDays){
    retentionInDays = _retentionInDays;
};

void ClassLogFile::WriteToFile(esp_log_level_t level, std::string info, bool _time)
{
/*
    struct stat path_stat;
    if (stat(logroot.c_str(), &path_stat) != 0) {
        ESP_LOGI(TAG, "Create log folder: %s", logroot.c_str());
        if (mkdir_r(logroot.c_str(), S_IRWXU) == -1)  {
            ESP_LOGE(TAG, "Can't create log folder");
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
    
    WriteToDedicatedFile(logpath, level, info, _time);
    ESP_LOG_LEVEL(level, TAG, "%s", info.c_str());
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
    

//////////////////////  message /////////////////////////////////////////
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
            if ((strlen(entry->d_name) == strlen(cmpfilename)) && (strcmp(entry->d_name, cmpfilename) == 0)) {
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
    LogFile.WriteToFile(ESP_LOG_INFO, "logfiles deleted: " + std::to_string(deleted) + " files not deleted (incl. leer.txt): " + std::to_string(notDeleted));	
    closedir(dir);


//////////////////////  data /////////////////////////////////////////
    strftime(cmpfilename, 30, datafile.c_str(), timeinfo);
    //ESP_LOGE(TAG, "log file name to compare: %s", cmpfilename);

    dir = opendir(dataroot.c_str());
    if (!dir) {
        ESP_LOGI(TAG, "Failed to stat dir : %s", dataroot.c_str());
        return;
    }

    deleted = 0;
    notDeleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            //ESP_LOGI(TAG, "list log file : %s %s", entry->d_name, cmpfilename);
            if ((strlen(entry->d_name) == strlen(cmpfilename)) && (strcmp(entry->d_name, cmpfilename) < 0)) {
                ESP_LOGI(TAG, "delete data file : %s", entry->d_name);
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

ClassLogFile::ClassLogFile(std::string _logroot, std::string _logfile, std::string _logdatapath, std::string _datafile)
{
    logroot = _logroot;
    logfile =  _logfile;
    datafile = _datafile;
    dataroot = _logdatapath;
    doLogFile = true;
    retentionInDays = 10;
    loglevel = ESP_LOG_INFO;
    MakeDir("/sdcard/log/data");
    MakeDir("/sdcard/test");
    MakeDir("/test");
}
