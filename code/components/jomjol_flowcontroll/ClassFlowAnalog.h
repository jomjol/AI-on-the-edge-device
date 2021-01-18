#pragma once
#include "ClassFlowImage.h"
#include "ClassFlowAlignment.h"
// #include "CTfLiteClass.h"

struct roianalog {
    int posx, posy, deltax, deltay;
    float result;
    CImageBasis *image, *image_org;
    string name;
};


class ClassFlowAnalog :
    public ClassFlowImage
{
protected:
    std::vector<roianalog*> ROI;
    string cnnmodelfile;
    int modelxsize, modelysize;
    int ZeigerEval(float zahl, int ziffer_vorgaenger);
    bool SaveAllFiles;    


    ClassFlowAlignment* flowpostalignment;

	void SetInitialParameter(void);        

public:
    bool extendedResolution;

    ClassFlowAnalog(std::vector<ClassFlow*>* lfc);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    string getReadout();   

    void DrawROI(CImageBasis *_zw); 

    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time);
   	std::vector<HTMLInfo*> GetHTMLInfo();   
    int AnzahlROIs(); 

    string name(){return "ClassFlowAnalog";}; 
};

