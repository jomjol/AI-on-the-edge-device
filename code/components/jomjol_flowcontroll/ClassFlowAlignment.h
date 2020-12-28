#pragma once

#include "ClassFlow.h"
#include "Helper.h"
#include "CAlignAndCutImage.h"

#include <string>

using namespace std;

class ClassFlowAlignment :
    public ClassFlow
{
protected:
    float initalrotate;
    bool initialmirror;
    string reffilename[2];
    int ref_x[2], ref_y[2];
    int ref_dx[2], ref_dy[2];
    int anz_ref;
    int suchex, suchey;
    string namerawimage;
    bool SaveAllFiles;
    CAlignAndCutImage *AlignAndCutImage;

    void SetInitialParameter(void);

public:
    CImageBasis *ImageBasis, *ImageTMP;
    
    ClassFlowAlignment(std::vector<ClassFlow*>* lfc);

    CAlignAndCutImage* GetAlignAndCutImage(){return AlignAndCutImage;};

    void DrawRef(CImageBasis *_zw);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    string name(){return "ClassFlowAlignment";};
};

