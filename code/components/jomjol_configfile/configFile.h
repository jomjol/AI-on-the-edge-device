#pragma once

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <string>
#include <vector>

class ConfigFile {
public:
    ConfigFile(std::string filePath);
    ~ConfigFile();

    bool isNewParagraph(std::string input);
    bool GetNextParagraph(std::string& aktparamgraph, bool &disabled, bool &eof);
	bool getNextLine(std::string* rt, bool &disabled, bool &eof);
    bool ConfigFileExists(){return pFile;};
    
private:
    FILE* pFile;
};

#endif //CONFIGFILE_H