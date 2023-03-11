#pragma once

#ifndef CLASSLOGFILE_H
#define CLASSLOGFILE_H


#include <string>
#include "esp_log.h"


class ClassLogFile
{
private:
    std::string logroot;
    std::string logfile;
    std::string dataroot;
    std::string datafile;
    unsigned short logFileRetentionInDays;
    unsigned short dataLogRetentionInDays;
    bool doDataLogToSD;
    esp_log_level_t loglevel;
public:
    ClassLogFile(std::string _logpath, std::string _logfile, std::string _logdatapath, std::string _datafile);

    void WriteHeapInfo(std::string _id);

    void setLogLevel(esp_log_level_t _logLevel);
    void SetLogFileRetention(unsigned short _LogFileRetentionInDays);
    void SetDataLogRetention(unsigned short _DataLogRetentionInDays);
    void SetDataLogToSD(bool _doDataLogToSD);
    bool GetDataLogToSD();

    void WriteToFile(esp_log_level_t level, std::string tag, std::string message, bool _time);
    void WriteToFile(esp_log_level_t level, std::string tag, std::string message);

    void CloseLogFileAppendHandle();

    bool CreateLogDirectories();
    void RemoveOldLogFile();
    void RemoveOldDataLog();

//    void WriteToData(std::string _ReturnRawValue, std::string _ReturnValue, std::string _ReturnPreValue, std::string _ErrorMessageText, std::string _digital, std::string _analog);
    void WriteToData(std::string _timestamp, std::string _name, std::string  _ReturnRawValue, std::string  _ReturnValue, std::string  _ReturnPreValue, std::string  _ReturnRateValue, std::string  _ReturnChangeAbsolute, std::string  _ErrorMessageText, std::string  _digital, std::string  _analog);


    std::string GetCurrentFileName();
    std::string GetCurrentFileNameData();
};

extern ClassLogFile LogFile;

#endif //CLASSLOGFILE_H