#include "ClassFlow.h"
#include <fstream>
#include <string>
#include <iostream>
#include <string.h>
#include "esp_log.h"
#include "../../include/defines.h"

static const char *TAG = "CLASS";


void ClassFlow::SetInitialParameter(void)
{
	ListFlowControll = NULL;
	previousElement = NULL;	
	disabled = false;
}

bool ClassFlow::isNewParagraph(string input)
{
	if ((input[0] == '[') || ((input[0] == ';') && (input[1] == '[')))
	{
		return true;
	}
	return false;
}

bool ClassFlow::GetNextParagraph(FILE* pfile, string& aktparamgraph)
{
	while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph));

	if (isNewParagraph(aktparamgraph))
		return true;
	return false;
}


ClassFlow::ClassFlow(void)
{
	SetInitialParameter();
}

ClassFlow::ClassFlow(std::vector<ClassFlow*> * lfc)
{
	SetInitialParameter();	
	ListFlowControll = lfc;
}

ClassFlow::ClassFlow(std::vector<ClassFlow*> * lfc, ClassFlow *_prev)
{
	SetInitialParameter();	
	ListFlowControll = lfc;
	previousElement = _prev;
}	

bool ClassFlow::ReadParameter(FILE* pfile, string &aktparamgraph)
{
	return false;
}

bool ClassFlow::doFlow(string time)
{
	return false;
}

string ClassFlow::getHTMLSingleStep(string host){
	return "";
}

string ClassFlow::getReadout()
{
	return string();
}

std::string ClassFlow::GetParameterName(std::string _input)
{
    string _param;
    int _pospunkt = _input.find_first_of(".");
    if (_pospunkt > -1)
    {
        _param = _input.substr(_pospunkt+1, _input.length() - _pospunkt - 1);
    }
    else
    {
        _param = _input;
    }
//    ESP_LOGD(TAG, "Parameter: %s, Pospunkt: %d", _param.c_str(), _pospunkt);
	return _param;
}


bool ClassFlow::getNextLine(FILE* pfile, string *rt)
{
	char zw[1024];
	if (pfile == NULL)
	{
		*rt = "";
		return false;
	}
	if (!fgets(zw, 1024, pfile))
	{
		*rt = "";
		ESP_LOGD(TAG, "END OF FILE");
		return false;
	}
	ESP_LOGD(TAG, "%s", zw);
	*rt = zw;
	*rt = trim(*rt);
	while ((zw[0] == ';' || zw[0] == '#' || (rt->size() == 0)) && !(zw[1] == '['))
	{
		*rt = "";
		if (!fgets(zw, 1024, pfile))
			return false;
		ESP_LOGD(TAG, "%s", zw);
		*rt = zw;
		*rt = trim(*rt);
	}
	return true;
}
