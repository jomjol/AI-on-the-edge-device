
#include <string>
#include "esp_log.h"


class ClassLogFile
{
private:
    std::string logroot;
    std::string logfile;
    std::string dataroot;
    std::string datafile;
    bool doLogFile;
    unsigned short retentionInDays;
    esp_log_level_t loglevel;
public:
    ClassLogFile(std::string _logpath, std::string _logfile, std::string _logdatapath, std::string _datafile);

    std::string getESPHeapInfo();

    void setLogLevel(esp_log_level_t i){loglevel = i;};

    void WriteHeapInfo(std::string _id);

    void SwitchOnOff(bool _doLogFile);
    void SetRetention(unsigned short _retentionInDays);

    void WriteToFile(esp_log_level_t level, std::string info, bool _time = true);
    void WriteToDedicatedFile(std::string _fn, esp_log_level_t level, std::string info, bool _time = true);
    void RemoveOld();

    void WriteToData(std::string _ReturnRawValue, std::string _ReturnValue, std::string _ReturnPreValue, std::string _ErrorMessageText, std::string _digital, std::string _analog);


    std::string GetCurrentFileName();
};

extern ClassLogFile LogFile;