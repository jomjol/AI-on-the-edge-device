
#include <string>


class ClassLogFile
{
private:
    std::string logroot;
    std::string logfile;
    bool doLogFile;
    unsigned short retentionInDays;
public:
    ClassLogFile(std::string _logpath, std::string _logfile);

    void SwitchOnOff(bool _doLogFile);
    void SetRetention(unsigned short _retentionInDays);

    void WriteToFile(std::string info, bool _time = true);
    void WriteToDedicatedFile(std::string _fn, std::string info, bool _time = true);
    void RemoveOld();

    std::string GetCurrentFileName();
};

extern ClassLogFile LogFile;