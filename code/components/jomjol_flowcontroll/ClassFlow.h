#pragma once

#ifndef CLASSFLOW_H
#define CLASSFLOW_H

#include <fstream>
#include <string>
#include <vector>

#include "Helper.h"
#include "CImageBasis.h"

using namespace std;

struct HTMLInfo
{
	float val;
	CImageBasis *image = NULL;
	CImageBasis *image_org = NULL;
	std::string filename;
	std::string filename_org;
};

class ClassFlow
{
protected:
	bool isNewParagraph(std::string input);
	bool GetNextParagraph(FILE *pfile, std::string &aktparamgraph);
	bool getNextLine(FILE *pfile, std::string *rt);

	std::vector<ClassFlow *> *ListFlowControll;
	ClassFlow *previousElement;

	virtual void SetInitialParameter(void);

	std::string GetParameterName(std::string _input);

	bool disabled;

public:
	ClassFlow(void);
	ClassFlow(std::vector<ClassFlow *> *lfc);
	ClassFlow(std::vector<ClassFlow *> *lfc, ClassFlow *_prev);

	virtual bool ReadParameter(FILE *pfile, std::string &aktparamgraph);
	virtual bool doFlow(std::string time);
	virtual std::string getHTMLSingleStep(std::string host);
	virtual std::string name(void) { return "ClassFlow"; };
};

#endif // CLASSFLOW_H