#include "ClassFlowImage.h"
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include "time_sntp.h"
#include "ClassLogFile.h"
#include "CImageBasis.h"

ClassFlowImage::ClassFlowImage(const char* logTag)
{
	this->logTag = logTag;
	isLogImage = false;
    disabled = false;

}

ClassFlowImage::ClassFlowImage(std::vector<ClassFlow*> * lfc, const char* logTag) : ClassFlow(lfc)
{
	this->logTag = logTag;
	isLogImage = false;
    disabled = false;
}

ClassFlowImage::ClassFlowImage(std::vector<ClassFlow*> * lfc, ClassFlow *_prev, const char* logTag) :  ClassFlow(lfc, _prev)
{
	this->logTag = logTag;
	isLogImage = false;
    disabled = false;
}


string ClassFlowImage::CreateLogFolder(string time) {
	if (!isLogImage)
		return "";

	string logPath = LogImageLocation + "/" + time.LOGFILE_TIME_FORMAT_DATE_EXTR + "/" + time.LOGFILE_TIME_FORMAT_HOUR_EXTR;
    isLogImage = mkdir_r(logPath.c_str(), S_IRWXU) == 0;
    if (!isLogImage) {
        ESP_LOGW(logTag, "Can't create log foolder for analog images. Path %s", logPath.c_str());
        LogFile.WriteToFile("Can't create log foolder for analog images. Path " + logPath);
    }

	return logPath;
}

void ClassFlowImage::LogImage(string logPath, string name, float *resultFloat, int *resultInt, string time, CImageBasis *_img) {
	if (!isLogImage)
		return;
	
	char buf[10];
	if (resultFloat != NULL) {
		sprintf(buf, "%.1f_", *resultFloat);
	} else if (resultInt != NULL) {
		sprintf(buf, "%d_", *resultInt);
	} else {
		buf[0] = '\0';
	}

	string nm = logPath + "/" + buf + name + "_" + time + ".jpg";
	nm = FormatFileName(nm);
	string output = "/sdcard/img_tmp/" + name + ".jpg";
	output = FormatFileName(output);
	printf("save to file: %s\n", nm.c_str());
	_img->SaveToFile(nm);
//	CopyFile(output, nm);
}

void ClassFlowImage::RemoveOldLogs()
{
	if (!isLogImage)
		return;
	
	ESP_LOGI(logTag, "remove old log images");
    if (logfileRetentionInDays == 0) {
        return;
    }

    time_t rawtime;
    struct tm* timeinfo;
    char cmpfilename[30];

    time(&rawtime);
    rawtime = addDays(rawtime, -logfileRetentionInDays);
    timeinfo = localtime(&rawtime);
    
    strftime(cmpfilename, 30, LOGFILE_TIME_FORMAT, timeinfo);
    //ESP_LOGE(TAG, "log file name to compare: %s", cmpfilename);
	string folderName = string(cmpfilename).LOGFILE_TIME_FORMAT_DATE_EXTR;

    DIR *dir = opendir(LogImageLocation.c_str());
    if (!dir) {
        ESP_LOGI(logTag, "Failed to stat dir : %s", LogImageLocation.c_str());
        return;
    }

    struct dirent *entry;
    int deleted = 0;
    int notDeleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        string folderPath = LogImageLocation + "/" + entry->d_name;
		if (entry->d_type == DT_DIR) {
			//ESP_LOGI(logTag, "Compare %s %s", entry->d_name, folderName.c_str());	
			if ((strlen(entry->d_name) == folderName.length()) && (strcmp(entry->d_name, folderName.c_str()) < 0)) {
                deleted += removeFolder(folderPath.c_str(), logTag);
			} else {
                notDeleted ++;
            }
		}
    }
    ESP_LOGI(logTag, "%d older log files deleted. %d current log files not deleted.", deleted, notDeleted);
    closedir(dir);
}

