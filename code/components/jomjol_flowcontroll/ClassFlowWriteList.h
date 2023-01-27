#pragma once

#ifndef CLASSFFLOWPWRITELIST_H
#define CLASSFFLOWPWRITELIST_H

#include "ClassFlow.h"
#include "ClassFlowPostProcessing.h"

#include <string>

class ClassFlowWriteList :
    public ClassFlow
{
protected:
	ClassFlowPostProcessing* flowpostprocessing;  
	void SetInitialParameter(void);        

public:
    ClassFlowWriteList();
    ClassFlowWriteList(std::vector<ClassFlow*>* lfc);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowWriteList";};
};

#endif //CLASSFFLOWPWRITELIST_H