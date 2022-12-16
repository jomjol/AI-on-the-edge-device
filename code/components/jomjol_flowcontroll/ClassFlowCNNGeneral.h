#ifndef __CLASSCNNGENERAL__
#define __CLASSCNNGENERAL__

#include"ClassFlowDefineTypes.h"
#include "ClassFlowAlignment.h"


enum t_CNNType {
    AutoDetect,
    Analogue,
    Analogue100,
    Digital,
    DigitalHyprid10,
    DoubleHyprid10,
    Digital100,
    None
 };

class ClassFlowCNNGeneral :
    public ClassFlowImage
{
protected:
    t_CNNType CNNType;
    std::vector<general*> GENERAL;
    float CNNGoodThreshold;
    float AnalogFehler = 3.0;
    float AnalogToDigtalFehler = 0.8;
    float Digital_Uncertainty = 0.2;
    int DigitalBand = 3;
    float DigitalAnalog_PredecessorsUebergangsbereich = 2;
    float DigitalUebergangsbereichVorgaenger = 0.7; // 9.3 - 0.7
    float DigitalUebergangsbereichVorlauf = 9.7; // Pre-run zero crossing only happens from approx. 9.7 onwards

    string cnnmodelfile;
    int modelxsize, modelysize, modelchannel;
    bool isLogImageSelect;
    string LogImageSelect;
    ClassFlowAlignment* flowpostalignment;

    bool SaveAllFiles;   

    int PointerEvalAnalogNew(float zahl, int ziffer_vorgaenger);
    int ZeigerEvalAnalogToDigitNeu(float zahl, float ziffer_vorgaenger,  int eval_predecessors, float analogDigitalTransitionStart);
    int PointerEvalHybridNew(float zahl, float number_of_predecessors, int eval_predecessors, bool Analog_Predecessors = false, float analogDigitalTransitionStart=9.2);



    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time);

    bool getNetworkParameter();

public:
    ClassFlowCNNGeneral(ClassFlowAlignment *_flowalign, t_CNNType _cnntype = AutoDetect);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);

    string getHTMLSingleStep(string host);
    string getReadout(int _analog, bool _extendedResolution = false, int prev = -1, float _before_narrow_Analog = -1, float analogDigitalTransitionStart=9.2); 

    string getReadoutRawString(int _analog);  

    void DrawROI(CImageBasis *_zw); 

   	std::vector<HTMLInfo*> GetHTMLInfo();   

    int getAnzahlGENERAL();
    general* GetGENERAL(int _analog);
    general* GetGENERAL(string _name, bool _create);
    general* FindGENERAL(string _name_number);    
    string getNameGENERAL(int _analog);    

    bool isExtendedResolution(int _number = 0);

    void UpdateNameNumbers(std::vector<std::string> *_name_numbers);

    t_CNNType getCNNType(){return CNNType;};

    string name(){return "ClassFlowCNNGeneral";}; 
};

#endif

