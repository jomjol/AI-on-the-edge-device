
#include <string>
#include "esp_log.h"


class ClassLogFile
{
private:
    std::string logroot;
    std::string logfile;
    std::string dataroot;
    std::string datafile;
    unsigned short retentionInDays;
    esp_log_level_t loglevel;
public:
    ClassLogFile(std::string _logpath, std::string _logfile, std::string _logdatapath, std::string _datafile);

    std::string getESPHeapInfo();

    void WriteHeapInfo(std::string _id);

    void setLogLevel(esp_log_level_t _logLevel);
    void SetRetention(unsigned short _retentionInDays);

    void WriteToFile(esp_log_level_t level, std::string info, bool _time = true);
    void WriteToDedicatedFile(std::string _fn, esp_log_level_t level, std::string info, bool _time = true);

    void CreateLogDirectories();
    void RemoveOld();

//    void WriteToData(std::string _ReturnRawValue, std::string _ReturnValue, std::string _ReturnPreValue, std::string _ErrorMessageText, std::string _digital, std::string _analog);
    void WriteToData(std::string _timestamp, std::string _name, std::string  _ReturnRawValue, std::string  _ReturnValue, std::string  _ReturnPreValue, std::string  _ReturnRateValue, std::string  _ReturnChangeAbsolute, std::string  _ErrorMessageText, std::string  _digital, std::string  _analog);


    std::string GetCurrentFileName();
};

extern ClassLogFile LogFile;