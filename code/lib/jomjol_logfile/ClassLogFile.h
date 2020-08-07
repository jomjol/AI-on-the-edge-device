
#include <string>


class ClassLogFile
{
private:
    /* data */
    std::string logfile;
public:
    ClassLogFile(std::string _logfile);
    ~ClassLogFile();

    void WriteToFile(std::string info, bool _time = true);
};

extern ClassLogFile LogFile;