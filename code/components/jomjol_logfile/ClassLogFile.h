
#include <string>


class ClassLogFile
{
private:
    std::string logroot;
    std::string logfile;
    bool doLogFile;
    unsigned short retentionInDays;
    int loglevel;
public:
    ClassLogFile(std::string _logpath, std::string _logfile);

    std::string getESPHeapInfo();

    void setLogLevel(int i){loglevel = i;};

    void WriteHeapInfo(std::string _id);

    void SwitchOnOff(bool _doLogFile);
    void SetRetention(unsigned short _retentionInDays);

    void WriteToFile(std::string info, bool _time = true);
    void WriteToDedicatedFile(std::string _fn, std::string info, bool _time = true);
    void RemoveOld();

    std::string GetCurrentFileName();
};

extern ClassLogFile LogFile;