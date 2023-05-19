#pragma once

#ifndef CLASSFLOWCONTROLL_H
#define CLASSFLOWCONTROLL_H

#include <string>

#include "ClassFlow.h"
#include "ClassFlowTakeImage.h"
#include "ClassFlowAlignment.h"
#include "ClassFlowCNNGeneral.h"
#include "ClassFlowPostProcessing.h"
#ifdef ENABLE_MQTT
	#include "ClassFlowMQTT.h"
#endif //ENABLE_MQTT
#ifdef ENABLE_INFLUXDB
	#include "ClassFlowInfluxDB.h"
	#include "ClassFlowInfluxDBv2.h"
#endif //ENABLE_INFLUXDB
#include "ClassFlowCNNGeneral.h"

class ClassFlowControll :
    public ClassFlow
{
protected:
	std::vector<ClassFlow*> FlowControll;
	ClassFlowPostProcessing* flowpostprocessing;
	ClassFlowAlignment* flowalignment;	
	ClassFlowCNNGeneral* flowanalog;
	ClassFlowCNNGeneral* flowdigit;
//	ClassFlowDigit* flowdigit;
	ClassFlowTakeImage* flowtakeimage;
	ClassFlow* CreateClassFlow(std::string _type);

	bool AutoStart;
	float AutoInterval;
	void SetInitialParameter(void);	
	std::string aktstatusWithTime;
	std::string aktstatus;
	int aktRunNr;

public:
	bool SetupModeActive;

	void InitFlow(std::string config);
	bool doFlow(string time);
	void doFlowTakeImageOnly(string time);
	bool getStatusSetupModus(){return SetupModeActive;};
	string getReadout(bool _rawvalue, bool _noerror, int _number);
	string getReadoutAll(int _type);	
	bool UpdatePrevalue(std::string _newvalue, std::string _numbers, bool _extern);
	string GetPrevalue(std::string _number = "");	
	bool ReadParameter(FILE* pfile, string& aktparamgraph);	
	string getJSON();
	string getNumbersName();

	string TranslateAktstatus(std::string _input);

	#ifdef ALGROI_LOAD_FROM_MEM_AS_JPG
	void DigitalDrawROI(CImageBasis *_zw);
	void AnalogDrawROI(CImageBasis *_zw);
	#endif

	esp_err_t GetJPGStream(std::string _fn, httpd_req_t *req);
	esp_err_t SendRawJPG(httpd_req_t *req);

	std::string doSingleStep(std::string _stepname, std::string _host);

	bool getIsAutoStart();
	void setAutoStartInterval(long &_interval);

	std::string* getActStatusWithTime();
	std::string* getActStatus();
	void setActStatus(std::string _aktstatus);

	std::vector<HTMLInfo*> GetAllDigital();
	std::vector<HTMLInfo*> GetAllAnalog();	

	t_CNNType GetTypeDigital();
	t_CNNType GetTypeAnalog();
	
	#ifdef ENABLE_MQTT
	bool StartMQTTService();
	#endif //ENABLE_MQTT

	int CleanTempFolder();

	string name(){return "ClassFlowControll";};
};

#endif



