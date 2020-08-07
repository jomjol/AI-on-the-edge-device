#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "Helper.h"
#include "CFindTemplate.h"

using namespace std;

struct HTMLInfo
{
	float val;
	std::string filename;
};


class ClassFlow
{
protected:
	std::vector<string> ZerlegeZeile(string input);
	bool isNewParagraph(string input);
	bool GetNextParagraph(FILE* pfile, string& aktparamgraph);
	bool getNextLine(FILE* pfile, string* rt);

	std::vector<ClassFlow*>* ListFlowControll;

	virtual void SetInitialParameter(void);

public:
	ClassFlow(void);
	ClassFlow(std::vector<ClassFlow*> * lfc);
	virtual bool ReadParameter(FILE* pfile, string &aktparamgraph);
	virtual bool doFlow(string time);
	virtual string getReadout();
	virtual string name(){return "ClassFlow";};

};

