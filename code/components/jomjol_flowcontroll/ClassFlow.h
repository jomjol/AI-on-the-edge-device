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
	bool isNewParagraph(string input);
	bool GetNextParagraph(FILE* pfile, string& aktparamgraph);
	bool getNextLine(FILE* pfile, string* rt);

	std::vector<ClassFlow*>* ListFlowControll;
	ClassFlow *previousElement;

	virtual void SetInitialParameter(void);

	std::string GetParameterName(std::string _input);

	bool disabled;

public:
	ClassFlow(void);
	ClassFlow(std::vector<ClassFlow*> * lfc);
	ClassFlow(std::vector<ClassFlow*> * lfc, ClassFlow *_prev);	
	
	virtual bool ReadParameter(FILE* pfile, string &aktparamgraph);
	virtual bool doFlow(string time);
	virtual string getHTMLSingleStep(string host);
	virtual string getReadout();
	virtual string name(){return "ClassFlow";};

};

#endif //CLASSFLOW_H