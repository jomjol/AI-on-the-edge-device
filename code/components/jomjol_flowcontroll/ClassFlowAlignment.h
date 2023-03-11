#pragma once

#ifndef CLASSFLOWALIGNMENT_H
#define CLASSFLOWALIGNMENT_H

#include "ClassFlow.h"
#include "Helper.h"
#include "CAlignAndCutImage.h"
#include "CFindTemplate.h"

#include <string>

using namespace std;

class ClassFlowAlignment : public ClassFlow
{
protected:
    float initalrotate;
    bool initialmirror;
    bool initialflip;
    bool use_antialiasing;
    bool SaveAllFiles;
    int anz_ref;
    RefInfo References[2];
    std::string namerawimage;
    std::string FileStoreRefAlignment;
    CAlignAndCutImage *AlignAndCutImage;
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
    virtual ~ClassFlowAlignment();

    CAlignAndCutImage* GetAlignAndCutImage() {return AlignAndCutImage;};

    void DrawRef(CImageBasis *_zw);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    string name() {return "ClassFlowAlignment";};
};


#endif //CLASSFLOWALIGNMENT_H
