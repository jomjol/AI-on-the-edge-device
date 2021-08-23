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

struct digit {
    string name;
    std::vector<roi*> ROI;
};

class ClassFlowDigit :
    public ClassFlowImage
{
protected:
//    std::vector<roi*> ROI;
    std::vector<digit*> DIGIT;
    string cnnmodelfile;
    int modelxsize, modelysize;
    bool SaveAllFiles;
    string NameDigit;
    int DecimalShift;
    bool DecimalShiftEnabled;

    bool isLogImageSelect;
    string LogImageSelect;


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
    string getReadout(int _digit);
   	std::vector<HTMLInfo*> GetHTMLInfo();

    int getAnzahlDIGIT();
    digit* GetDIGIT(int _digit);
    digit* GetDIGIT(string _name, bool _create);
    digit* FindDIGIT(string _name_number);

    string getNameDIGIT(int _digit);

    void UpdateNameNumbers(std::vector<std::string> *_name_numbers);

    void DrawROI(CImageBasis *_zw);        

    string name(){return "ClassFlowDigit";};
};

