#pragma once
#include "ClassFlow.h"

using namespace std;

class ClassFlowImage : public ClassFlow
{
protected:
	string LogImageLocation;
    bool isLogImage;
    unsigned short logfileRetentionInDays;
	const char* logTag;

	string CreateLogFolder(string time);
	void LogImage(string logPath, string name, float *resultFloat, int *resultInt, string time, CImageBasis *_img);


public:
	ClassFlowImage(const char* logTag);
	ClassFlowImage(std::vector<ClassFlow*> * lfc, const char* logTag);
	ClassFlowImage(std::vector<ClassFlow*> * lfc, ClassFlow *_prev, const char* logTag);
	
	void RemoveOldLogs();
};
