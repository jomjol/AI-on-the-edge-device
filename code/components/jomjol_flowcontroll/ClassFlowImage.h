#pragma once

#ifndef CLASSFLOWIMAGE_H
#define CLASSFLOWIMAGE_H

#include "ClassFlow.h"

using namespace std;

class ClassFlowImage : public ClassFlow
{
protected:
	std::string imagesLocation;
	bool isLogImage;
	unsigned short imagesRetention;
	const char *logTag;

	std::string CreateLogFolder(std::string time);
	void LogImage(std::string logPath, std::string name, float *resultFloat, int *resultInt, std::string time, CImageBasis *_img);

public:
	ClassFlowImage(const char *logTag);
	ClassFlowImage(std::vector<ClassFlow *> *lfc, const char *logTag);
	ClassFlowImage(std::vector<ClassFlow *> *lfc, ClassFlow *_prev, const char *logTag);

	void RemoveOldLogs();
};

#endif // CLASSFLOWIMAGE_H