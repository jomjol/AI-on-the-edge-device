#include "ClassFlowCNNGeneral.h"

#include <math.h>
#include <iomanip> 
#include <sys/types.h>
#include <sstream>      // std::stringstream

#include "CTfLiteClass.h"
#include "ClassLogFile.h"
#include "esp_log.h"
#include "../../include/defines.h"

static const char* TAG = "CNN";

//#ifdef CONFIG_HEAP_TRACING_STANDALONE
#ifdef HEAP_TRACING_CLASS_FLOW_CNN_GENERAL_DO_ALING_AND_CUT
    #include <esp_heap_trace.h>
    #define NUM_RECORDS 300
    static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be in internal RAM
#endif


ClassFlowCNNGeneral::ClassFlowCNNGeneral(ClassFlowAlignment *_flowalign, t_CNNType _cnntype) : ClassFlowImage(NULL, TAG)
{
    string cnnmodelfile = "";
    modelxsize = 1;
    modelysize = 1;
    CNNGoodThreshold = 0.0;
    ListFlowControll = NULL;
    previousElement = NULL;   
    SaveAllFiles = false; 
    disabled = false;
    isLogImageSelect = false;
    CNNType = AutoDetect;
    CNNType = _cnntype;
    flowpostalignment = _flowalign;
    imagesRetention = 5;
}


string ClassFlowCNNGeneral::getReadout(int _analog = 0, bool _extendedResolution, int prev, float _before_narrow_Analog, float analogDigitalTransitionStart)
{
    string result = "";    

    if (GENERAL[_analog]->ROI.size() == 0)
        return result;
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "getReadout _analog=" + std::to_string(_analog) + ", _extendedResolution=" + std::to_string(_extendedResolution) + ", prev=" + std::to_string(prev));
 
    if (CNNType == Analogue || CNNType == Analogue100)
    {
        float number = GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float;
        int result_after_decimal_point = ((int) floor(number * 10) + 10) % 10;
        
        prev = PointerEvalAnalogNew(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, prev);
//        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "getReadout(analog) number=" + std::to_string(number) + ", result_after_decimal_point=" + std::to_string(result_after_decimal_point) + ", prev=" + std::to_string(prev));
        result = std::to_string(prev);

        if (_extendedResolution)
            result = result + std::to_string(result_after_decimal_point);

        for (int i = GENERAL[_analog]->ROI.size() - 2; i >= 0; --i)
        {
            prev = PointerEvalAnalogNew(GENERAL[_analog]->ROI[i]->result_float, prev);
            result = std::to_string(prev) + result;
        }
        return result;
    }

    if (CNNType == Digital)
    {
        for (int i = 0; i < GENERAL[_analog]->ROI.size(); ++i)
        {
            if (GENERAL[_analog]->ROI[i]->result_klasse >= 10)
                result = result + "N";
            else
                result = result + std::to_string(GENERAL[_analog]->ROI[i]->result_klasse);
        }
        return result;
    }

    if ((CNNType == DoubleHyprid10) || (CNNType == Digital100))
    {

        float number = GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float;
        if (number >= 0)       // NaN?
        {
            if (_extendedResolution)            // is only set if it is the first digit (no analogue before!)
            {
                int result_after_decimal_point = ((int) floor(number * 10)) % 10;
                int result_before_decimal_point = ((int) floor(number)) % 10;

                result = std::to_string(result_before_decimal_point) + std::to_string(result_after_decimal_point);
                prev = result_before_decimal_point;
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "getReadout(dig100-ext) result_before_decimal_point=" + std::to_string(result_before_decimal_point) + ", result_after_decimal_point=" + std::to_string(result_after_decimal_point) + ", prev=" + std::to_string(prev));
            }
            else
            {
                if (_before_narrow_Analog >= 0)
                    prev = PointerEvalHybridNew(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, _before_narrow_Analog, prev, true, analogDigitalTransitionStart);
                else
                    prev = PointerEvalHybridNew(GENERAL[_analog]->ROI[GENERAL[_analog]->ROI.size() - 1]->result_float, prev, prev);
                result = std::to_string(prev);
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "getReadout(dig100)  prev=" + std::to_string(prev));
        
            }
        }
        else
        {
            result = "N";
            if (_extendedResolution && (CNNType != Digital))
                result = "NN";
        }

        for (int i = GENERAL[_analog]->ROI.size() - 2; i >= 0; --i)
        {
            if (GENERAL[_analog]->ROI[i]->result_float >= 0)
            {
                prev = PointerEvalHybridNew(GENERAL[_analog]->ROI[i]->result_float, GENERAL[_analog]->ROI[i+1]->result_float, prev);
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "getReadout#PointerEvalHybridNew()= " + std::to_string(prev));
                result = std::to_string(prev) + result;
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "getReadout#result= " + result);
                
            }
            else
            {
                prev = -1;
                result = "N" + result;
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "getReadout(result_float<0 /'N')  result_float=" + std::to_string(GENERAL[_analog]->ROI[i]->result_float));
        
            }
        }
        return result;
    }
    return result;
}


int ClassFlowCNNGeneral::PointerEvalHybridNew(float number, float number_of_predecessors, int eval_predecessors, bool Analog_Predecessors, float digitalAnalogTransitionStart)
{
    int result;
    int result_after_decimal_point = ((int) floor(number * 10)) % 10;
    int result_before_decimal_point = ((int) floor(number) + 10) % 10;

    if (eval_predecessors < 0)
    {   
        // on first digit is no spezial logic for transition needed
        // we use the recognition as given. The result is the int value of the recognition
        // add precisition of 2 digits and round before trunc
        result = (int) ((int) trunc(round((number+10 % 10)*100)) )  / 100;

        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalHybridNew - No predecessor - Result = " + std::to_string(result) +
                                                    " number: " + std::to_string(number) + " number_of_predecessors = " + std::to_string(number_of_predecessors)+ " eval_predecessors = " + std::to_string(eval_predecessors) + " Digital_Uncertainty = " +  std::to_string(Digital_Uncertainty));
        return result;
    }

    if (Analog_Predecessors)
    {
        result = PointerEvalAnalogToDigitNew(number, number_of_predecessors, eval_predecessors, digitalAnalogTransitionStart);
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalHybridNew - Analog predecessor, evaluation over PointerEvalAnalogNew = " + std::to_string(result) +
                                                    " number: " + std::to_string(number) + " number_of_predecessors = " + std::to_string(number_of_predecessors)+ " eval_predecessors = " + std::to_string(eval_predecessors) + " Digital_Uncertainty = " +  std::to_string(Digital_Uncertainty));
        return result;
    }

    if ((number_of_predecessors >= Digital_Transition_Area_Predecessor ) && (number_of_predecessors <= (10.0 - Digital_Transition_Area_Predecessor)))
    {
        // no digit change, because predecessor is far enough away (0+/-DigitalTransitionRangePredecessor) --> number is rounded
        if ((result_after_decimal_point <= DigitalBand) || (result_after_decimal_point >= (10-DigitalBand)))     // Band around the digit --> Round off, as digit reaches inaccuracy in the frame
            result = ((int) round(number) + 10) % 10;
        else
            result = ((int) trunc(number) + 10) % 10;

        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalHybridNew - NO analogue predecessor, no change of digits, as pre-decimal point far enough away = " + std::to_string(result) +
                                                    " number: " + std::to_string(number) + " number_of_predecessors = " + std::to_string(number_of_predecessors)+ " eval_predecessors = " + std::to_string(eval_predecessors) + " Digital_Uncertainty = " +  std::to_string(Digital_Uncertainty));
        return result;
    }  

    if (eval_predecessors <= 1)  // Zero crossing at the predecessor has taken place (! evaluation via Prev_value and not number!) --> round up here (2.8 --> 3, but also 3.1 --> 3)
    {
        // We simply assume that the current digit after the zero crossing of the predecessor
        // has passed through at least half (x.5)
        if (result_after_decimal_point > 5)
            // The current digit does not yet have a zero crossing, but the predecessor does..
            result =  (result_before_decimal_point + 1) % 10;
        else
            // Act. digit and predecessor have zero crossing
            result =  result_before_decimal_point % 10;
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalHybridNew - NO analogue predecessor, zero crossing has taken placen = " + std::to_string(result) +
                                                    " number: " + std::to_string(number) + " number_of_predecessors = " + std::to_string(number_of_predecessors)+ " eval_predecessors = " + std::to_string(eval_predecessors) + " Digital_Uncertainty = " +  std::to_string(Digital_Uncertainty));
        return result;
    }

    
    // remains only >= 9.x --> no zero crossing yet --> 2.8 --> 2, 
    // and from 9.7(DigitalTransitionRangeLead) 3.1 --> 2
    // everything >=x.4 can be considered as current number in transition. With 9.x predecessor the current
    // number can still be x.6 - x.7. 
    // Preceding (else - branch) does not already happen from 9.
    if (Digital_Transition_Area_Forward>=number_of_predecessors || result_after_decimal_point >= 4)
        // The current digit, like the previous digit, does not yet have a zero crossing. 
        result =  result_before_decimal_point % 10;
    else
        // current digit precedes the smaller digit (9.x). So already >=x.0 while the previous digit has not yet
        // has no zero crossing. Therefore, it is reduced by 1.
        result =  (result_before_decimal_point - 1 + 10) % 10;

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalHybridNew - O analogue predecessor, >= 9.5 --> no zero crossing yet = " + std::to_string(result) +
                                                " number: " + std::to_string(number) + " number_of_predecessors = " + std::to_string(number_of_predecessors)+ " eval_predecessors = " + std::to_string(eval_predecessors) + " Digital_Uncertainty = " +  std::to_string(Digital_Uncertainty) + " result_after_decimal_point = " + std::to_string(result_after_decimal_point));
    return result;
}


int ClassFlowCNNGeneral::PointerEvalAnalogToDigitNew(float number, float numeral_preceder,  int eval_predecessors, float analogDigitalTransitionStart)
{
    int result;
    int result_after_decimal_point = ((int) floor(number * 10)) % 10;
    int result_before_decimal_point = ((int) floor(number) + 10) % 10;
    bool roundedUp = false;

    // Within the digital inequalities 
    if ((result_after_decimal_point >= (10-Digital_Uncertainty * 10))     // Band around the digit --> Round off, as digit reaches inaccuracy in the frame
        || (eval_predecessors <= 4 && result_after_decimal_point>=6))  {   // or digit runs after (analogue =0..4, digit >=6)
        result = (int) (round(number) + 10) % 10;
        roundedUp = true;
        // before/ after decimal point, because we adjust the number based on the uncertainty.
        result_after_decimal_point = ((int) floor(result * 10)) % 10;
        result_before_decimal_point = ((int) floor(result) + 10) % 10;
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalAnalogToDigitNew - Digital Uncertainty - Result = " + std::to_string(result) +
                                                    " number: " + std::to_string(number) + " numeral_preceder: " + std::to_string(numeral_preceder) +
                                                    " erg before comma: " + std::to_string(result_before_decimal_point) + 
                                                    " erg after comma: " + std::to_string(result_after_decimal_point));
    } else {
        result = (int) ((int) trunc(number) + 10) % 10;
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalAnalogToDigitNew - NO digital Uncertainty - Result = " + std::to_string(result) +
                                                    " number: " + std::to_string(number) + " numeral_preceder = " + std::to_string(numeral_preceder));
    }

    // No zero crossing has taken place.
    // Only eval_predecessors used because numeral_preceder could be wrong here.
    // numeral_preceder<=0.1 & eval_predecessors=9 corresponds to analogue was reset because of previous analogue that are not yet at 0.
    if ((eval_predecessors>=6 && (numeral_preceder>analogDigitalTransitionStart || numeral_preceder<=0.2) && roundedUp))
    {
        result =  ((result_before_decimal_point+10) - 1) % 10;
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalAnalogToDigitNew - Nulldurchgang noch nicht stattgefunden = " + std::to_string(result) +
                                    " number: " + std::to_string(number) + 
                                    " numeral_preceder = " + std::to_string(numeral_preceder) + 
                                    " eerg after comma = " +  std::to_string(result_after_decimal_point));

    }

    return result;

}


int ClassFlowCNNGeneral::PointerEvalAnalogNew(float number, int numeral_preceder)
{
    float number_min, number_max;
    int result;

    if (numeral_preceder == -1)
    {
        result = (int) floor(number);
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalAnalogNew - No predecessor - Result = " + std::to_string(result) +
                                                    " number: " + std::to_string(number) + " numeral_preceder = " + std::to_string(numeral_preceder) + " Analog_error = " +  std::to_string(Analog_error));
        return result;
    }

    number_min = number - Analog_error / 10.0;
    number_max = number + Analog_error / 10.0;

    if ((int) floor(number_max) - (int) floor(number_min) != 0)
    {
        if (numeral_preceder <= Analog_error)
        {
            result = ((int) floor(number_max) + 10) % 10;
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalAnalogNew - number ambiguous, correction upwards - result = " + std::to_string(result) +
                                                        " number: " + std::to_string(number) + " numeral_preceder = " + std::to_string(numeral_preceder) + " Analog_error = " +  std::to_string(Analog_error));
            return result;
        }
        if (numeral_preceder >= 10 - Analog_error)
        {
            result = ((int) floor(number_min) + 10) % 10;
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalAnalogNew - number ambiguous, downward correction - result = " + std::to_string(result) +
                                                        " number: " + std::to_string(number) + " numeral_preceder = " + std::to_string(numeral_preceder) + " Analog_error = " +  std::to_string(Analog_error));
            return result;
        }
    }
    

    result = ((int) floor(number) + 10) % 10;
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "PointerEvalAnalogNew - number unambiguous, no correction necessary - result = " + std::to_string(result) +
                                                " number: " + std::to_string(number) + " numeral_preceder = " + std::to_string(numeral_preceder) + " Analog_error = " +  std::to_string(Analog_error));

    return result;
}


bool ClassFlowCNNGeneral::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if ((toUpper(aktparamgraph) != "[ANALOG]") && (toUpper(aktparamgraph) != ";[ANALOG]") 
        && (toUpper(aktparamgraph) != "[DIGIT]") && (toUpper(aktparamgraph) != ";[DIGIT]")
        && (toUpper(aktparamgraph) != "[DIGITS]") && (toUpper(aktparamgraph) != ";[DIGITS]")
        )       // Paragraph passt nicht
        return false;

    if (aktparamgraph[0] == ';')
    {
        disabled = true;
        while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph));
        ESP_LOGD(TAG, "[Analog/Digit] is disabled!");
        return true;
    }


    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);
        if ((toUpper(splitted[0]) == "ROIIMAGESLOCATION") && (splitted.size() > 1))
        {
            this->imagesLocation = "/sdcard" + splitted[1];
            this->isLogImage = true;
        }
        if ((toUpper(splitted[0]) == "LOGIMAGESELECT") && (splitted.size() > 1))
        {
            LogImageSelect = splitted[1];
            isLogImageSelect = true;            
        }

        if ((toUpper(splitted[0]) == "ROIIMAGESRETENTION") && (splitted.size() > 1))
        {
            this->imagesRetention = std::stoi(splitted[1]);
        }

        if ((toUpper(splitted[0]) == "MODEL") && (splitted.size() > 1))
        {
            this->cnnmodelfile = splitted[1];
        }
        
        if ((toUpper(splitted[0]) == "CNNGOODTHRESHOLD") && (splitted.size() > 1))
        {
            CNNGoodThreshold = std::stof(splitted[1]);
        }
        if (splitted.size() >= 5)
        {
            general* _analog = GetGENERAL(splitted[0], true);
            roi* neuroi = _analog->ROI[_analog->ROI.size()-1];
            neuroi->posx = std::stoi(splitted[1]);
            neuroi->posy = std::stoi(splitted[2]);
            neuroi->deltax = std::stoi(splitted[3]);
            neuroi->deltay = std::stoi(splitted[4]);
            neuroi->CCW = false;
            if (splitted.size() >= 6)
            {
                neuroi->CCW = toUpper(splitted[5]) == "TRUE";
            }
            neuroi->result_float = -1;
            neuroi->image = NULL;
            neuroi->image_org = NULL;
        }

        if ((toUpper(splitted[0]) == "SAVEALLFILES") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                SaveAllFiles = true;
        }
    }

    if (!getNetworkParameter()) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "An error occured on setting up the Network -> Disabling it!");
        disabled = true; // An error occured, disable this CNN!
        return false;
    }


    for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
            GENERAL[_ana]->ROI[i]->image = new CImageBasis("ROI " + GENERAL[_ana]->ROI[i]->name, 
                    modelxsize, modelysize, modelchannel);
            GENERAL[_ana]->ROI[i]->image_org = new CImageBasis("ROI " + GENERAL[_ana]->ROI[i]->name + " original",
                    GENERAL[_ana]->ROI[i]->deltax, GENERAL[_ana]->ROI[i]->deltay, 3);
        }

    return true;
}


general* ClassFlowCNNGeneral::FindGENERAL(string _name_number)
{
    for (int i = 0; i < GENERAL.size(); ++i)
        if (GENERAL[i]->name == _name_number)
            return GENERAL[i];
    return NULL;
}


general* ClassFlowCNNGeneral::GetGENERAL(string _name, bool _create = true)
{
    string _analog, _roi;
    int _pospunkt = _name.find_first_of(".");

    if (_pospunkt > -1)
    {
        _analog = _name.substr(0, _pospunkt);
        _roi = _name.substr(_pospunkt+1, _name.length() - _pospunkt - 1);
    }
    else
    {
        _analog = "default";
        _roi = _name;
    }

    general *_ret = NULL;

    for (int i = 0; i < GENERAL.size(); ++i)
        if (GENERAL[i]->name == _analog)
            _ret = GENERAL[i];

    if (!_create)         // not found and should not be created
        return _ret;

    if (_ret == NULL)
    {
        _ret = new general;
        _ret->name = _analog;
        GENERAL.push_back(_ret);
    }

    roi* neuroi = new roi;
    neuroi->name = _roi;

    _ret->ROI.push_back(neuroi);

    ESP_LOGD(TAG, "GetGENERAL - GENERAL %s - roi %s - CCW: %d", _analog.c_str(), _roi.c_str(), neuroi->CCW);

    return _ret;
}


string ClassFlowCNNGeneral::getHTMLSingleStep(string host)
{
    string result, zw;
    std::vector<HTMLInfo*> htmlinfo;

    result = "<p>Found ROIs: </p> <p><img src=\"" + host + "/img_tmp/alg_roi.jpg\"></p>\n";
    result = result + "Analog Pointers: <p> ";

    htmlinfo = GetHTMLInfo();
    for (int i = 0; i < htmlinfo.size(); ++i)
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << htmlinfo[i]->val;
        zw = stream.str();

        result = result + "<img src=\"" + host + "/img_tmp/" +  htmlinfo[i]->filename + "\"> " + zw;
        delete htmlinfo[i];
    }
    htmlinfo.clear();         

    return result;
}


bool ClassFlowCNNGeneral::doFlow(string time)
{

#ifdef HEAP_TRACING_CLASS_FLOW_CNN_GENERAL_DO_ALING_AND_CUT
    //register a buffer to record the memory trace
    ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );
    // start tracing
    ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
#endif

    if (disabled)
      return true;

    if (!doAlignAndCut(time)){
        return false;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "doFlow after alignment");

    doNeuralNetwork(time);

    RemoveOldLogs();

#ifdef HEAP_TRACING_CLASS_FLOW_CNN_GENERAL_DO_ALING_AND_CUT
    ESP_ERROR_CHECK( heap_trace_stop() );
    heap_trace_dump(); 
#endif   

    return true;
}


bool ClassFlowCNNGeneral::doAlignAndCut(string time)
{
    if (disabled)
        return true;

    CAlignAndCutImage *caic = flowpostalignment->GetAlignAndCutImage();    

    for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
            ESP_LOGD(TAG, "General %d - Align&Cut", i);
            
            caic->CutAndSave(GENERAL[_ana]->ROI[i]->posx, GENERAL[_ana]->ROI[i]->posy, GENERAL[_ana]->ROI[i]->deltax, GENERAL[_ana]->ROI[i]->deltay, GENERAL[_ana]->ROI[i]->image_org);
            if (SaveAllFiles)
            {
                if (GENERAL[_ana]->name == "default")
                    GENERAL[_ana]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
                else
                    GENERAL[_ana]->ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
            } 

            GENERAL[_ana]->ROI[i]->image_org->Resize(modelxsize, modelysize, GENERAL[_ana]->ROI[i]->image);
            if (SaveAllFiles)
            {
                if (GENERAL[_ana]->name == "default")
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
                else
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
            } 
        }

    return true;
} 


void ClassFlowCNNGeneral::DrawROI(CImageBasis *_zw)
{
    if (_zw->ImageOkay()) 
    { 
        if (CNNType == Analogue || CNNType == Analogue100)
        {
            int r = 0;
            int g = 255;
            int b = 0;

            for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
                for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
                {
                    _zw->drawRect(GENERAL[_ana]->ROI[i]->posx, GENERAL[_ana]->ROI[i]->posy, GENERAL[_ana]->ROI[i]->deltax, GENERAL[_ana]->ROI[i]->deltay, r, g, b, 1);
                    _zw->drawEllipse( (int) (GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax/2), (int)  (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay/2), (int) (GENERAL[_ana]->ROI[i]->deltax/2), (int) (GENERAL[_ana]->ROI[i]->deltay/2), r, g, b, 2);
                    _zw->drawLine((int) (GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax/2), (int) GENERAL[_ana]->ROI[i]->posy, (int) (GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax/2), (int) (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay), r, g, b, 2);
                    _zw->drawLine((int) GENERAL[_ana]->ROI[i]->posx, (int) (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay/2), (int) GENERAL[_ana]->ROI[i]->posx + GENERAL[_ana]->ROI[i]->deltax, (int) (GENERAL[_ana]->ROI[i]->posy + GENERAL[_ana]->ROI[i]->deltay/2), r, g, b, 2);
                }
        }
        else
        {
            for (int _dig = 0; _dig < GENERAL.size(); ++_dig)
                for (int i = 0; i < GENERAL[_dig]->ROI.size(); ++i)
                    _zw->drawRect(GENERAL[_dig]->ROI[i]->posx, GENERAL[_dig]->ROI[i]->posy, GENERAL[_dig]->ROI[i]->deltax, GENERAL[_dig]->ROI[i]->deltay, 0, 0, (255 - _dig*100), 2);
        }
    }
} 


bool ClassFlowCNNGeneral::getNetworkParameter()
{
    if (disabled)
        return true;

    CTfLiteClass *tflite = new CTfLiteClass;  
    string zwcnn = "/sdcard" + cnnmodelfile;
    zwcnn = FormatFileName(zwcnn);
    ESP_LOGD(TAG, "%s", zwcnn.c_str());
    if (!tflite->LoadModel(zwcnn)) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't load tflite model " + cnnmodelfile + " -> Init aborted!");
        LogFile.WriteHeapInfo("getNetworkParameter-LoadModel");
        delete tflite;
        return false;
    } 

    if (!tflite->MakeAllocate()) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't allocate tflite model -> Init aborted!");
        LogFile.WriteHeapInfo("getNetworkParameter-MakeAllocate");
        delete tflite;
        return false;
    }

    if (CNNType == AutoDetect)
    {
        tflite->GetInputDimension(false);
        modelxsize = tflite->ReadInputDimenstion(0);
        modelysize = tflite->ReadInputDimenstion(1);
        modelchannel = tflite->ReadInputDimenstion(2);

        int _anzoutputdimensions = tflite->GetAnzOutPut();
        switch (_anzoutputdimensions) 
        {
            case 2:
                CNNType = Analogue;
                ESP_LOGD(TAG, "TFlite-Type set to Analogue");
                break;
            case 10:
                CNNType = DoubleHyprid10;
                ESP_LOGD(TAG, "TFlite-Type set to DoubleHyprid10");
                break;
            case 11:
                CNNType = Digital;
                ESP_LOGD(TAG, "TFlite-Type set to Digital");
                break;
/*            case 20:
                CNNType = DigitalHyprid10;
                ESP_LOGD(TAG, "TFlite-Type set to DigitalHyprid10");
                break;
*/
//            case 22:
//                CNNType = DigitalHyprid;
//                ESP_LOGD(TAG, "TFlite-Type set to DigitalHyprid");
//                break;
             case 100:
                if (modelxsize==32 && modelysize == 32) {
                    CNNType = Analogue100;
                    ESP_LOGD(TAG, "TFlite-Type set to Analogue100");
                } else {
                    CNNType = Digital100;
                    ESP_LOGD(TAG, "TFlite-Type set to Digital");
                }
                break;
            default:
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "tflite does not fit the firmware (outout_dimension=" + std::to_string(_anzoutputdimensions) + ")");
        }
    }

    delete tflite;
    return true;
}


bool ClassFlowCNNGeneral::doNeuralNetwork(string time)
{
    if (disabled)
        return true;

    string logPath = CreateLogFolder(time);

    CTfLiteClass *tflite = new CTfLiteClass;  
    string zwcnn = "/sdcard" + cnnmodelfile;
    zwcnn = FormatFileName(zwcnn);
    ESP_LOGD(TAG, "%s", zwcnn.c_str());

    if (!tflite->LoadModel(zwcnn)) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't load tflite model " + cnnmodelfile + " -> Exec aborted this round!");
        LogFile.WriteHeapInfo("doNeuralNetwork-LoadModel");
        delete tflite;
        return false;
    }

    if (!tflite->MakeAllocate()) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't allocate tfilte model -> Exec aborted this round!");
        LogFile.WriteHeapInfo("doNeuralNetwork-MakeAllocate");
        delete tflite;
        return false;
    }

    for (int n = 0; n < GENERAL.size(); ++n) // For each NUMBER
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Processing Number '" + GENERAL[n]->name + "'");
        for (int roi = 0; roi < GENERAL[n]->ROI.size(); ++roi) // For each ROI
        {
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "ROI #" + std::to_string(roi) + " - TfLite");
            //ESP_LOGD(TAG, "General %d - TfLite", i);

            switch (CNNType) {
                case Analogue:
                    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CNN Type: Analogue");
                    {
                        float f1, f2;
                        f1 = 0; f2 = 0;

                        tflite->LoadInputImageBasis(GENERAL[n]->ROI[roi]->image);        
                        tflite->Invoke();
                        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "After Invoke");

                        f1 = tflite->GetOutputValue(0);
                        f2 = tflite->GetOutputValue(1);
                        float result = fmod(atan2(f1, f2) / (M_PI * 2) + 2, 1);
                              
                        if(GENERAL[n]->ROI[roi]->CCW)
                            GENERAL[n]->ROI[roi]->result_float = 10 - (result * 10);
                        else
                            GENERAL[n]->ROI[roi]->result_float = result * 10;
                              
                        ESP_LOGD(TAG, "General result (Analog)%i - CCW: %d -  %f", roi, GENERAL[n]->ROI[roi]->CCW, GENERAL[n]->ROI[roi]->result_float);
                        if (isLogImage)
                            LogImage(logPath, GENERAL[n]->ROI[roi]->name, &GENERAL[n]->ROI[roi]->result_float, NULL, time, GENERAL[n]->ROI[roi]->image_org);
                    } break;

                case Digital:
                    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CNN Type: Digital");
                    {
                        GENERAL[n]->ROI[roi]->result_klasse = 0;
                        GENERAL[n]->ROI[roi]->result_klasse = tflite->GetClassFromImageBasis(GENERAL[n]->ROI[roi]->image);
                        ESP_LOGD(TAG, "General result (Digit)%i: %d", roi, GENERAL[n]->ROI[roi]->result_klasse);

                        if (isLogImage)
                        {
                            string _imagename = GENERAL[n]->name +  "_" + GENERAL[n]->ROI[roi]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[n]->ROI[roi]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, NULL, &GENERAL[n]->ROI[roi]->result_klasse, time, GENERAL[n]->ROI[roi]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, NULL, &GENERAL[n]->ROI[roi]->result_klasse, time, GENERAL[n]->ROI[roi]->image_org);
                            }
                        }
                    } break;


                case DoubleHyprid10:
                    {
                    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CNN Type: DoubleHyprid10");
                        int _num, _numplus, _numminus;
                        float _val, _valplus, _valminus;
                        float _fit;
                        float _result_save_file;

                        tflite->LoadInputImageBasis(GENERAL[n]->ROI[roi]->image);        
                        tflite->Invoke();
                        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "After Invoke");

                        _num = tflite->GetOutClassification(0, 9);
                        _numplus = (_num + 1) % 10;
                        _numminus = (_num - 1 + 10) % 10;

                        _val = tflite->GetOutputValue(_num);
                        _valplus = tflite->GetOutputValue(_numplus);
                        _valminus = tflite->GetOutputValue(_numminus);

                        float result = _num;

                        if (_valplus > _valminus)
                        {
                            result = result + _valplus / (_valplus + _val);
                            _fit = _val + _valplus;
                        }
                        else
                        {
                            result = result - _valminus / (_val + _valminus);
                            _fit = _val + _valminus;

                        }
                        if (result >= 10)
                            result = result - 10;
                        if (result < 0)
                            result = result + 10;

                        string zw = "_num (p, m): " + to_string(_num) + " " + to_string(_numplus) + " " + to_string(_numminus);
                        zw = zw + " _val (p, m): " + to_string(_val) + " " + to_string(_valplus) + " " + to_string(_valminus);
                        zw = zw + " result: " + to_string(result) + " _fit: " + to_string(_fit);
                        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, zw);


                        _result_save_file = result;

                        if (_fit < CNNGoodThreshold)
                        {
                            GENERAL[n]->ROI[roi]->isReject = true;
                            result = -1;
                            _result_save_file+= 100;     // In case fit is not sufficient, the result should still be saved with "-10x.y".
                            string zw = "Value Rejected due to Threshold (Fit: " + to_string(_fit) + ", Threshold: " + to_string(CNNGoodThreshold) + ")";
                            LogFile.WriteToFile(ESP_LOG_WARN, TAG, zw);
                        }
                        else
                        {
                            GENERAL[n]->ROI[roi]->isReject = false;
                        }


                        GENERAL[n]->ROI[roi]->result_float = result;
                        ESP_LOGD(TAG, "Result General(Analog)%i: %f", roi, GENERAL[n]->ROI[roi]->result_float);

                        if (isLogImage)
                        {
                            string _imagename = GENERAL[n]->name +  "_" + GENERAL[n]->ROI[roi]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[n]->ROI[roi]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[n]->ROI[roi]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[n]->ROI[roi]->image_org);
                            }
                        }
                    }
                    break;
                case Digital100:
                case Analogue100:
                    {
                    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CNN Type: Digital100 or Analogue100");
                        int _num;
                        float _result_save_file;
                        
                        tflite->LoadInputImageBasis(GENERAL[n]->ROI[roi]->image);        
                        tflite->Invoke();
    
                        _num = tflite->GetOutClassification();
                        
                        if(GENERAL[n]->ROI[roi]->CCW)
                            GENERAL[n]->ROI[roi]->result_float = 10 - ((float)_num / 10.0);                              
                        else
                            GENERAL[n]->ROI[roi]->result_float = (float)_num / 10.0;

                        _result_save_file = GENERAL[n]->ROI[roi]->result_float;

                        
                        GENERAL[n]->ROI[roi]->isReject = false;
                        
                        ESP_LOGD(TAG, "Result General(Analog)%i - CCW: %d -  %f", roi, GENERAL[n]->ROI[roi]->CCW, GENERAL[n]->ROI[roi]->result_float);

                        if (isLogImage)
                        {
                            string _imagename = GENERAL[n]->name +  "_" + GENERAL[n]->ROI[roi]->name;
                            if (isLogImageSelect)
                            {
                                if (LogImageSelect.find(GENERAL[n]->ROI[roi]->name) != std::string::npos)
                                    LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[n]->ROI[roi]->image_org);
                            }
                            else
                            {
                                LogImage(logPath, _imagename, &_result_save_file, NULL, time, GENERAL[n]->ROI[roi]->image_org);
                            }
                        }

                    } break;
            
                default:
                    break;
            }
        }
    }

    delete tflite;

    return true;
}


bool ClassFlowCNNGeneral::isExtendedResolution(int _number)
{
    if (CNNType == Digital)
        return false;
    return true;
}


std::vector<HTMLInfo*> ClassFlowCNNGeneral::GetHTMLInfo()
{
    std::vector<HTMLInfo*> result;

    for (int _ana = 0; _ana < GENERAL.size(); ++_ana)
        for (int i = 0; i < GENERAL[_ana]->ROI.size(); ++i)
        {
            ESP_LOGD(TAG, "Image: %d", (int) GENERAL[_ana]->ROI[i]->image);
            if (GENERAL[_ana]->ROI[i]->image)
            {
                if (GENERAL[_ana]->name == "default")
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
                else
                    GENERAL[_ana]->ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".jpg"));
            }

            HTMLInfo *zw = new HTMLInfo;
            if (GENERAL[_ana]->name == "default")
            {
                zw->filename = GENERAL[_ana]->ROI[i]->name + ".jpg";
                zw->filename_org = GENERAL[_ana]->ROI[i]->name + ".jpg";
            }
            else
            {
                zw->filename = GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".jpg";
                zw->filename_org = GENERAL[_ana]->name + "_" + GENERAL[_ana]->ROI[i]->name + ".jpg";
            }

            if (CNNType == Digital)
                zw->val = GENERAL[_ana]->ROI[i]->result_klasse;
            else
                zw->val = GENERAL[_ana]->ROI[i]->result_float;
            zw->image = GENERAL[_ana]->ROI[i]->image;
            zw->image_org = GENERAL[_ana]->ROI[i]->image_org;

            result.push_back(zw);
        }

    return result;
}


int ClassFlowCNNGeneral::getNumberGENERAL()
{
    return GENERAL.size();
}


string ClassFlowCNNGeneral::getNameGENERAL(int _analog)
{
    if (_analog < GENERAL.size())
        return GENERAL[_analog]->name;

    return "GENERAL DOES NOT EXIST";
}


general* ClassFlowCNNGeneral::GetGENERAL(int _analog)
{
    if (_analog < GENERAL.size())
        return GENERAL[_analog];

    return NULL;
}


void ClassFlowCNNGeneral::UpdateNameNumbers(std::vector<std::string> *_name_numbers)
{
    for (int _dig = 0; _dig < GENERAL.size(); _dig++)
    {
        std::string _name = GENERAL[_dig]->name;
        bool found = false;
        for (int i = 0; i < (*_name_numbers).size(); ++i)
        {
            if ((*_name_numbers)[i] == _name)
                found = true;
        }
        if (!found)
            (*_name_numbers).push_back(_name);
    }
}


string ClassFlowCNNGeneral::getReadoutRawString(int _analog) 
{
    string rt = "";

    if (_analog >= GENERAL.size() || GENERAL[_analog]==NULL || GENERAL[_analog]->ROI.size() == 0)
        return rt;
 
    for (int i = 0; i < GENERAL[_analog]->ROI.size(); ++i)
    {
        if (CNNType == Analogue || CNNType == Analogue100)
        {
            rt = rt + "," + RundeOutput(GENERAL[_analog]->ROI[i]->result_float, 1);
        }

        if (CNNType == Digital)
        {
            if (GENERAL[_analog]->ROI[i]->result_klasse == 10)
                rt = rt + ",N";
            else
                rt = rt + "," + RundeOutput(GENERAL[_analog]->ROI[i]->result_klasse, 0);
        }

        if ((CNNType == DoubleHyprid10) || (CNNType == Digital100))
        {
            rt = rt + "," + RundeOutput(GENERAL[_analog]->ROI[i]->result_float, 1);
        }
    }
    return rt;
}

