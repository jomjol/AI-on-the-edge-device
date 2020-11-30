#pragma once
#include <string>
#include <fstream>


using namespace std;

std::string FormatFileName(std::string input);
void FindReplace(std::string& line, std::string& oldString, std::string& newString);

void CopyFile(string input, string output);

size_t findDelimiterPos(string input, string delimiter);
//string trim(string istring);
string trim(string istring, string adddelimiter = "");
bool ctype_space(const char c, string adddelimiter);

string getFileType(string filename);

int mkdir_r(const char *dir, const mode_t mode);
int removeFolder(const char* folderPath, const char* logTag);

string toUpper(string in);

float temperatureRead();

time_t addDays(time_t startTime, int days);
