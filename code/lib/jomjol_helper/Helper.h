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

string toUpper(string in);

float temperatureRead();
