#include <string>
#include <vector>

class ConfigFile {
public:
    ConfigFile(std::string filePath);
    ~ConfigFile();

    bool isNewParagraph(std::string input);
    bool GetNextParagraph(std::string& aktparamgraph, bool &disabled, bool &eof);
	bool getNextLine(std::string* rt, bool &disabled, bool &eof);
    
private:
    FILE* pFile;
};