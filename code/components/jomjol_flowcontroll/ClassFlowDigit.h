#pragma once
#include "ClassFlowImage.h"
#include "ClassFlowAlignment.h"
#include "Helper.h"

#include <string>

struct roi {
    int posx, posy, deltax, deltay;
    int resultklasse;
    string name;
    CImageBasis *image, *image_org;
    roi* next;
};

class ClassFlowDigit :
    public ClassFlowImage
{
protected:
    std::vector<roi*> ROI;
    string cnnmodelfile;
    int modelxsize, modelysize;
    bool SaveAllFiles;

    ClassFlowAlignment* flowpostalignment;

    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time); 

	void SetInitialParameter(void);    

public:
    ClassFlowDigit();
    ClassFlowDigit(std::vector<ClassFlow*>* lfc);
    ClassFlowDigit(std::vector<ClassFlow*>* lfc, ClassFlow *_prev);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host); 
    string getReadout();
   	std::vector<HTMLInfo*> GetHTMLInfo();

    void DrawROI(CImageBasis *_zw);        

    string name(){return "ClassFlowDigit";};
};

