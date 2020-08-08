#include "ClassLogFile.h"
#include "time_sntp.h"

ClassLogFile LogFile("/sdcard/log.txt");

void ClassLogFile::WriteToDedicatedFile(std::string _fn, std::string info, bool _time)
{
    FILE* pFile;
    std::string zwtime;

    pFile = fopen(_fn.c_str(), "a+");

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
}

void ClassLogFile::WriteToFile(std::string info, bool _time)
{
    WriteToDedicatedFile(logfile, info, _time);
}

ClassLogFile::ClassLogFile(std::string _logfile)
{
    logfile = _logfile;
}