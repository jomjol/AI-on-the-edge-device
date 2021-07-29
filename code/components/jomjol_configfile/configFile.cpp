#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Helper.h"
#include "configFile.h"

//static const char *TAGCONFIGFILE = "configFile";

ConfigFile::ConfigFile(std::string filePath)
{
    std::string config = FormatFileName(filePath);
    pFile = OpenFileAndWait(config.c_str(), "r");
}

ConfigFile::~ConfigFile()
{
    fclose(pFile);
}

bool ConfigFile::isNewParagraph(std::string input)
{
	if ((input[0] == '[') || ((input[0] == ';') && (input[1] == '[')))
	{
		return true;
	}
	return false;
}

bool ConfigFile::GetNextParagraph(std::string& aktparamgraph, bool &disabled, bool &eof)
{
	while (getNextLine(&aktparamgraph, disabled, eof) && !isNewParagraph(aktparamgraph));

	if (isNewParagraph(aktparamgraph))
		return true;
	return false;
}

bool ConfigFile::getNextLine(std::string *rt, bool &disabled, bool &eof)
{
    eof = false;
	char zw[1024] = "";
	if (pFile == NULL)
	{
		*rt = "";
		return false;
	}

	if (fgets(zw, 1024, pFile))
	{
		printf("%s", zw);
		if ((strlen(zw) == 0) && feof(pFile))
		{
			*rt = "";
			eof = true;
			return false;
		}
	}
	else
	{
		*rt = "";
		eof = true;
		return false;
	}
	*rt = zw;
	*rt = trim(*rt);
	while ((zw[0] == ';' || zw[0] == '#' || (rt->size() == 0)) && !(zw[1] == '['))			// Kommentarzeilen (; oder #) und Leerzeilen überspringen, es sei denn es ist ein neuer auskommentierter Paragraph
	{
		fgets(zw, 1024, pFile);
		printf("%s", zw);		
		if (feof(pFile))
		{
			*rt = "";
            eof = true;
			return false;
		}
		*rt = zw;
		*rt = trim(*rt);
	}

    disabled = ((*rt)[0] == ';');
	return true;
}

std::vector<string> ConfigFile::ZerlegeZeile(std::string input, std::string delimiter)
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
