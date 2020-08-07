#pragma once

#include "ClassFlow.h"
#include "Helper.h"

#include <string>

using namespace std;

class ClassFlowAlignment :
    public ClassFlow
{
protected:
    float initalrotate;
    string reffilename[2];
    int ref_x[2], ref_y[2];
    int anz_ref;
    int suchex, suchey;
    string namerawimage;

public:
    ClassFlowAlignment();
    ClassFlowAlignment(std::vector<ClassFlow*>* lfc);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string name(){return "ClassFlowAlignment";};
};

