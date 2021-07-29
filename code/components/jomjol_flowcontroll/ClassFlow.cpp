#include "ClassFlow.h"
#include <fstream>
#include <string>
#include <iostream>
#include <string.h>



void ClassFlow::SetInitialParameter(void)
{
	ListFlowControll = NULL;
	previousElement = NULL;	
	disabled = false;
}




std::vector<string> ClassFlow::ZerlegeZeile(std::string input, std::string delimiter)
{
	std::vector<string> Output;
//	std::string delimiter = " =,";

	input = trim(input, delimiter);
	size_t pos = findDelimiterPos(input, delimiter);
	std::string token;
	while (pos != std::string::npos) {
		token = input.substr(0, pos);
		token = trim(token, delimiter);
		Output.push_back(token);
		input.erase(0, pos + 1);
		input = trim(input, delimiter);
		pos = findDelimiterPos(input, delimiter);
	}
	Output.push_back(input);

	return Output;

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
//    printf("Parameter: %s, Pospunkt: %d\n", _param.c_str(), _pospunkt);
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
		printf("END OF FILE\n");
		return false;
	}
	printf("%s", zw);
	*rt = zw;
	*rt = trim(*rt);
	while ((zw[0] == ';' || zw[0] == '#' || (rt->size() == 0)) && !(zw[1] == '['))			// Kommentarzeilen (; oder #) und Leerzeilen überspringen, es sei denn es ist ein neuer auskommentierter Paragraph
	{
		*rt = "";
		if (!fgets(zw, 1024, pfile))
			return false;
		printf("%s", zw);		
		*rt = zw;
		*rt = trim(*rt);
	}
	return true;
}
