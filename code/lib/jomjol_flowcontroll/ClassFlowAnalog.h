#pragma once
#include "ClassFlow.h"
// #include "CTfLiteClass.h"

struct roianalog {
    int posx, posy, deltax, deltay;
    float result;
    string name;
};


class ClassFlowAnalog :
    public ClassFlow
{
protected:
    string LogImageLocation;
    bool isLogImage;
    std::vector<roianalog*> ROI;
    string cnnmodelfile;
    int modelxsize, modelysize;
    int ZeigerEval(float zahl, int ziffer_vorgaenger);

public:
    ClassFlowAnalog();
    ClassFlowAnalog(std::vector<ClassFlow*>* lfc);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    string getReadout();    

    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time);
   	std::vector<HTMLInfo*> GetHTMLInfo();   
    int AnzahlROIs(){return ROI.size();}; 

    string name(){return "ClassFlowAnalog";};
};

