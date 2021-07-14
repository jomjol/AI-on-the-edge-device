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

struct analog {
    string name;
    std::vector<roianalog*> ROI;
};


class ClassFlowAnalog :
    public ClassFlowImage
{
protected:
//    std::vector<roianalog*> ROI;
    std::vector<analog*> ANALOG;

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
    string getReadout(int _analog);   

    void DrawROI(CImageBasis *_zw); 

    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time);
   	std::vector<HTMLInfo*> GetHTMLInfo();   
    int AnzahlROIs(int _analog);

    int getAnzahlANALOG();
    analog* GetANALOG(int _analog);
    analog* GetANALOG(string _name, bool _create);
    analog* FindANALOG(string _name_number);    
    string getNameANALOG(int _analog);     

    void UpdateNameNumbers(std::vector<std::string> *_name_numbers);


    string name(){return "ClassFlowAnalog";}; 
};

