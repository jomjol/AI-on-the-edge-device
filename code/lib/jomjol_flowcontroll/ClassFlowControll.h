#pragma once

#include <string>

#include "ClassFlow.h"
#include "ClassFlowMakeImage.h"
#include "ClassFlowAlignment.h"
#include "ClassFlowDigit.h"
#include "ClassFlowAnalog.h"
#include "ClassFlowPostProcessing.h"


class ClassFlowControll :
    public ClassFlow
{
protected:
	std::vector<ClassFlow*> FlowControll;
	ClassFlowPostProcessing* flowpostprocessing;
	ClassFlow* CreateClassFlow(std::string _type);

	bool AutoStart;
	float AutoIntervall;
	void SetInitialParameter(void);	


public:
	void InitFlow(std::string config);
	bool doFlow(string time);
	string getReadout(bool _rawvalue);
	string UpdatePrevalue(std::string _newvalue);
	string GetPrevalue();	
	bool ReadParameter(FILE* pfile, string& aktparamgraph);	

	bool isAutoStart(long &_intervall);

	std::vector<HTMLInfo*> GetAllDigital();
	std::vector<HTMLInfo*> GetAllAnalog();	

	string name(){return "ClassFlowControll";};
};

