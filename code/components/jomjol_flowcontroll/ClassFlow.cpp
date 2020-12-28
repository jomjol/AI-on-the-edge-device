#include "ClassFlow.h"
#include <fstream>
#include <string>
#include <iostream>
#include <string.h>



void ClassFlow::SetInitialParameter(void)
{
	ListFlowControll = NULL;
	previousElement = NULL;	
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
	if (input[0] == '[')
		return true;
	return false;
}

bool ClassFlow::GetNextParagraph(FILE* pfile, string& aktparamgraph)
{
	while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph));

	if (this->isNewParagraph(aktparamgraph))
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

bool ClassFlow::getNextLine(FILE* pfile, string *rt)
{
	char zw[1024];
	if (pfile == NULL)
	{
		*rt = "";
		return false;
	}
	fgets(zw, 1024, pfile);
	printf("%s", zw);
	if ((strlen(zw) == 0) && feof(pfile))
	{
		*rt = "";
		return false;
	}
	*rt = zw;
	*rt = trim(*rt);
	while (zw[0] == ';' || zw[0] == '#' || (rt->size() == 0))			// Kommentarzeilen (; oder #) und Leerzeilen überspringen
	{
		fgets(zw, 1024, pfile);
		printf("%s", zw);		
		if (feof(pfile))
		{
			*rt = "";
			return false;
		}
		*rt = zw;
		*rt = trim(*rt);
	}
	return true;
}
