#include <sstream>
#include "ClassFlowWriteList.h"
#include "Helper.h"

#include "time_sntp.h"
#include "../../include/defines.h"

#include <time.h>

void ClassFlowWriteList::SetInitialParameter(void)
{
    flowpostprocessing = NULL;  
    previousElement = NULL;
    ListFlowControll = NULL; 
    disabled = false;
}       

ClassFlowWriteList::ClassFlowWriteList()
{
    SetInitialParameter();
}

ClassFlowWriteList::ClassFlowWriteList(std::vector<ClassFlow*>* lfc)
{
    SetInitialParameter();

    ListFlowControll = lfc;
    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }
    }
}


bool ClassFlowWriteList::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[MQTT]") != 0)  
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);
    }
   
    return true;
}



bool ClassFlowWriteList::doFlow(string zwtime)
{
    std::string line = "";

    std::string result;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    string zw = "";
    string namenumber = "";

    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;

            line = line + resulttimestamp + "\t" + resultraw + "\t" + result + "\t" + resultraw + "\t" + resultrate + "\t" + resulttimestamp + "\t"; 

        }
    }
    
    return true;
}
