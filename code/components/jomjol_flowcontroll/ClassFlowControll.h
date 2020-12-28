#pragma once

#include <string>

#include "ClassFlow.h"
#include "ClassFlowMakeImage.h"
#include "ClassFlowAlignment.h"
#include "ClassFlowDigit.h"
#include "ClassFlowAnalog.h"
#include "ClassFlowPostProcessing.h"
#include "ClassFlowMQTT.h"


class ClassFlowControll :
    public ClassFlow
{
protected:
	std::vector<ClassFlow*> FlowControll;
	ClassFlowPostProcessing* flowpostprocessing;
	ClassFlowAlignment* flowalignment;	
	ClassFlowAnalog* flowanalog;
	ClassFlowDigit* flowdigit;
	ClassFlowMakeImage* flowmakeimage;
	ClassFlow* CreateClassFlow(std::string _type);

	bool AutoStart;
	float AutoIntervall;
	bool SetupModeActive;
	void SetInitialParameter(void);	
	std::string aktstatus;

public:
	void InitFlow(std::string config);
	bool doFlow(string time);
	void doFlowMakeImageOnly(string time);
	bool getStatusSetupModus(){return SetupModeActive;};
	string getReadout(bool _rawvalue, bool _noerror);
	string UpdatePrevalue(std::string _newvalue);
	string GetPrevalue();	
	bool ReadParameter(FILE* pfile, string& aktparamgraph);	

	esp_err_t GetJPGStream(std::string _fn, httpd_req_t *req);
	esp_err_t SendRawJPG(httpd_req_t *req);

	std::string doSingleStep(std::string _stepname, std::string _host);

	bool isAutoStart(long &_intervall);

	std::string getActStatus();

	std::vector<HTMLInfo*> GetAllDigital();
	std::vector<HTMLInfo*> GetAllAnalog();	

	int CleanTempFolder();

	string name(){return "ClassFlowControll";};
};

extern bool debug_detail_heap;

