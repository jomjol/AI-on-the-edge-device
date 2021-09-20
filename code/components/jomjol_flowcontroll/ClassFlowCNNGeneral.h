#ifndef __CLASSCNNGENERAL__
#define __CLASSCNNGENERAL__

#include"ClassFlowDefineTypes.h"
#include "ClassFlowAlignment.h"
// #include "ClassFlowPostProcessing.h"


enum t_CNNType {
    AutoDetect,
    Analogue,
    Digital,
    DigitalHyprid,
    None
 };

class ClassFlowCNNGeneral :
    public ClassFlowImage
{
protected:
    t_CNNType CNNType;
    std::vector<general*> GENERAL;

    string cnnmodelfile;
    int modelxsize, modelysize;
    bool isLogImageSelect;
    string LogImageSelect;
    ClassFlowAlignment* flowpostalignment;
//    ClassFlowPostProcessing *flowpostprocessing = NULL;
    bool SaveAllFiles;   
//    bool extendedResolution;

    int ZeigerEval(float zahl, int ziffer_vorgaenger);
    int ZeigerEvalHybrid(float zahl, float zahl_vorgaenger, int eval_vorgaenger);


    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time);

public:
    ClassFlowCNNGeneral(ClassFlowAlignment *_flowalign, t_CNNType _cnntype = AutoDetect);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);

    string getHTMLSingleStep(string host);
    string getReadout(int _analog, bool _extendedResolution);   

    void DrawROI(CImageBasis *_zw); 

   	std::vector<HTMLInfo*> GetHTMLInfo();   

//    int AnzahlROIs(int _analog);
    int getAnzahlGENERAL();
    general* GetGENERAL(int _analog);
    general* GetGENERAL(string _name, bool _create);
    general* FindGENERAL(string _name_number);    
    string getNameGENERAL(int _analog);    

    bool isExtendedResolution(int _number = 0);

//    void setPostprocessing(ClassFlowPostProcessing *_fpp){flowpostprocessing = _fpp;}; 

    void UpdateNameNumbers(std::vector<std::string> *_name_numbers);

    t_CNNType getCNNType(){return CNNType;};

    string name(){return "ClassFlowCNNGeneral";}; 
};

#endif

