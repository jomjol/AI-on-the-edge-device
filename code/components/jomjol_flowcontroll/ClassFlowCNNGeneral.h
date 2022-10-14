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
    float DigitalUnschaerfe = 0.2;
    int DigitalBand = 3;
    float DigitalAnalogerVorgaengerUebergangsbereich = 2;
    float DigitalUebergangsbereichVorgaengerAnalogToDigit = 1; // war vorher 2
    float DigitalUebergangsbereichVorgaenger = 0.7; // 9.3 - 0.7
    float DigitalUebergangsbereichVorlauf = 9.7; // Vorlauf-Nulldurchgang passiert erst ab ca. 9.7

    string cnnmodelfile;
    int modelxsize, modelysize, modelchannel;
    bool isLogImageSelect;
    string LogImageSelect;
    ClassFlowAlignment* flowpostalignment;

    bool SaveAllFiles;   

    int ZeigerEvalAnalogNeu(float zahl, int ziffer_vorgaenger);
    int ZeigerEvalAnalogToDigitNeu(float zahl, float ziffer_vorgaenger,  int eval_vorgaenger);
    int ZeigerEvalHybridNeu(float zahl, float zahl_vorgaenger, int eval_vorgaenger, bool AnalogerVorgaenger = false);



    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time);

    bool getNetworkParameter();

public:
    ClassFlowCNNGeneral(ClassFlowAlignment *_flowalign, t_CNNType _cnntype = AutoDetect);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);

    string getHTMLSingleStep(string host);
    string getReadout(int _analog, bool _extendedResolution = false, int prev = -1, float _vorgaengerAnalog = -1);   

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

