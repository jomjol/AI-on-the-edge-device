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
	std::string aktstatus;


public:
	void InitFlow(std::string config);
	bool doFlow(string time);
	string getReadout(bool _rawvalue);
	string UpdatePrevalue(std::string _newvalue);
	string GetPrevalue();	
	bool ReadParameter(FILE* pfile, string& aktparamgraph);	

	std::string doSingleStep(std::string _stepname, std::string _host);

	bool isAutoStart(long &_intervall);

	std::string getActStatus();

	std::vector<HTMLInfo*> GetAllDigital();
	std::vector<HTMLInfo*> GetAllAnalog();	

	string name(){return "ClassFlowControll";};
};

