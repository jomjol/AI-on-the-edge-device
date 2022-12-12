#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "sdmmc_cmd.h"

using namespace std;

std::string FormatFileName(std::string input);
std::size_t file_size(const std::string& file_name);
void FindReplace(std::string& line, std::string& oldString, std::string& newString);

bool CopyFile(string input, string output);
bool DeleteFile(string fn);
bool RenameFile(string from, string to);
bool MakeDir(std::string _what);


string RundeOutput(double _in, int _anzNachkomma);


FILE* OpenFileAndWait(const char* nm, const char* _mode, int _waitsec = 1, bool silent = true);

size_t findDelimiterPos(string input, string delimiter);
//string trim(string istring);
string trim(string istring, string adddelimiter = "");
bool ctype_space(const char c, string adddelimiter);

string getFileType(string filename);
string getFileFullFileName(string filename);
string getDirectory(string filename);

int mkdir_r(const char *dir, const mode_t mode);
int removeFolder(const char* folderPath, const char* logTag);

string toLower(string in);
string toUpper(string in);

float temperatureRead();

time_t addDays(time_t startTime, int days);

void memCopyGen(uint8_t* _source, uint8_t* _target, int _size);

std::vector<string> HelperZerlegeZeile(std::string input, std::string _delimiter);
std::vector<std::string> ZerlegeZeile(std::string input, std::string delimiter = " =, \t");

///////////////////////////
size_t getInternalESPHeapSize();
size_t getESPHeapSize();
string getESPHeapInfo();

/////////////////////////////
string getSDCardPartitionSize();
string getSDCardFreePartitionSpace();
string getSDCardPartitionAllocationSize();

void SaveSDCardInfo(sdmmc_card_t* card);
string SDCardParseManufacturerIDs(int);
string getSDCardManufacturer();
string getSDCardName();
string getSDCardCapacity();
string getSDCardSectorSize();

string getMac(void);
string getResetReason(void);
std::string getFormatedUptime(bool compact);

const char* get404(void);
