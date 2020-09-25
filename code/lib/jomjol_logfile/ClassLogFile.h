
#include <string>


class ClassLogFile
{
private:
    std::string logfile;
    bool doLogFile;
public:
    ClassLogFile(std::string _logfile);

    void SwitchOnOff(bool _doLogFile);

    void WriteToFile(std::string info, bool _time = true);
    void WriteToDedicatedFile(std::string _fn, std::string info, bool _time = true);
};

extern ClassLogFile LogFile;