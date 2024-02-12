#pragma once

#ifndef CLASSFLOWALIGNMENT_H
#define CLASSFLOWALIGNMENT_H

#include "ClassFlow.h"
#include "Helper.h"
#include "CAlignAndCutImage.h"
#include "CFindTemplate.h"

#include <string>

using namespace std;

class ClassFlowAlignment :
    public ClassFlow
{
protected:
    float initialrotate;
    bool initialmirror;
    bool initialflip;
    bool use_antialiasing;
    RefInfo References[2];
    int anz_ref;
    string namerawimage;
    bool SaveAllFiles;
    CAlignAndCutImage *AlignAndCutImage;
    std::string FileStoreRefAlignment;
    float SAD_criteria;

    void SetInitialParameter(void);
    bool LoadReferenceAlignmentValues(void);
    void SaveReferenceAlignmentValues();

public:
    CImageBasis *ImageBasis, *ImageTMP;
    #ifdef ALGROI_LOAD_FROM_MEM_AS_JPG 
    ImageData *AlgROI;
    #endif
    
    ClassFlowAlignment(std::vector<ClassFlow*>* lfc);

    CAlignAndCutImage* GetAlignAndCutImage(){return AlignAndCutImage;};

    void DrawRef(CImageBasis *_zw);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    string name(){return "ClassFlowAlignment";};
};


#endif //CLASSFLOWALIGNMENT_H
