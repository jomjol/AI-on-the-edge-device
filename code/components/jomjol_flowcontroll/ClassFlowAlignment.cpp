#include "ClassFlowAlignment.h"
#include "ClassFlowTakeImage.h"
#include "ClassFlow.h"
#include "MainFlowControl.h"

#include "CRotateImage.h"
#include "esp_log.h"


#include "ClassLogFile.h"
#include "psram.h"
#include "../../include/defines.h"


static const char *TAG = "ALIGN";

// #define DEBUG_DETAIL_ON  


void ClassFlowAlignment::SetInitialParameter(void)
{
    initialrotate = 0;
    anz_ref = 0;
    initialmirror = false;
    use_antialiasing = false;
    initialflip = false;
    SaveAllFiles = false;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
    FileStoreRefAlignment = "/sdcard/config/align.txt";
    ListFlowControll = NULL;
    AlignAndCutImage = NULL;
    ImageBasis = NULL;
    ImageTMP = NULL;
    #ifdef ALGROI_LOAD_FROM_MEM_AS_JPG
    AlgROI = (ImageData*)malloc_psram_heap(std::string(TAG) + "->AlgROI", sizeof(ImageData), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    #endif
    previousElement = NULL;
    disabled = false;
    SAD_criteria = 0.05;
}


ClassFlowAlignment::ClassFlowAlignment(std::vector<ClassFlow*>* lfc)
{
    SetInitialParameter();
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowTakeImage") == 0)
        {
            ImageBasis = ((ClassFlowTakeImage*) (*ListFlowControll)[i])->rawImage;
        }
    }

    if (!ImageBasis)            // the function take pictures does not exist --> must be created first ONLY FOR TEST PURPOSES
    {
        ESP_LOGD(TAG, "CImageBasis had to be created");
        ImageBasis = new CImageBasis("ImageBasis", namerawimage);
    }
}


bool ClassFlowAlignment::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> splitted;
    int suchex = 40;
    int suchey = 40;
    int alg_algo = 0; //default=0; 1 =HIGHACCURACY; 2= FAST; 3= OFF //add disable aligment algo |01.2023


    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (aktparamgraph.compare("[Alignment]") != 0)       //Paragraph does not fit Alignment
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        splitted = ZerlegeZeile(aktparamgraph);
        if ((toUpper(splitted[0]) == "FLIPIMAGESIZE") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                initialflip = true;
        }
        if ((toUpper(splitted[0]) == "INITIALMIRROR") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                initialmirror = true;
        }
        if (((toUpper(splitted[0]) == "initialrotate") || (toUpper(splitted[0]) == "INITIALROTATE")) && (splitted.size() > 1))
        {
            this->initialrotate = std::stod(splitted[1]);
        }
        if ((toUpper(splitted[0]) == "SEARCHFIELDX") && (splitted.size() > 1))
        {
            suchex = std::stod(splitted[1]);
        }   
        if ((toUpper(splitted[0]) == "SEARCHFIELDY") && (splitted.size() > 1))
        {
            suchey = std::stod(splitted[1]);
        }   
        if ((toUpper(splitted[0]) == "ANTIALIASING") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                use_antialiasing = true;
        }   
        if ((splitted.size() == 3) && (anz_ref < 2))
        {
            References[anz_ref].image_file = FormatFileName("/sdcard" + splitted[0]);
            References[anz_ref].target_x = std::stod(splitted[1]);
            References[anz_ref].target_y = std::stod(splitted[2]);
            anz_ref++;
        }

        if ((toUpper(splitted[0]) == "SAVEALLFILES") && (splitted.size() > 1))
        {
            if (toUpper(splitted[1]) == "TRUE")
                SaveAllFiles = true;
        }
        if ((toUpper(splitted[0]) == "ALIGNMENTALGO") && (splitted.size() > 1))
        {
            #ifdef DEBUG_DETAIL_ON
                std::string zw2 = "Alignment mode selected: " + splitted[1];
                LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, zw2);
            #endif
            if (toUpper(splitted[1]) == "HIGHACCURACY")
                alg_algo = 1;
            if (toUpper(splitted[1]) == "FAST")
                alg_algo = 2;
            if (toUpper(splitted[1]) == "OFF") //no align algo if set to 3 = off => no draw ref //add disable aligment algo |01.2023
                alg_algo = 3;
        }
    }

    for (int i = 0; i < anz_ref; ++i)
    {
        References[i].search_x = suchex;
        References[i].search_y = suchey;
        References[i].fastalg_SAD_criteria = SAD_criteria;
        References[i].alignment_algo = alg_algo;
        #ifdef DEBUG_DETAIL_ON
            std::string zw2 = "Alignment mode written: " + std::to_string(alg_algo);
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, zw2);
        #endif
    }

    //no align algo if set to 3 = off => no draw ref //add disable aligment algo |01.2023
    if(References[0].alignment_algo != 3){
        LoadReferenceAlignmentValues();
    }
    
    return true;

}


string ClassFlowAlignment::getHTMLSingleStep(string host)
{
    string result;

    result =          "<p>Rotated Image: </p> <p><img src=\"" + host + "/img_tmp/rot.jpg\"></p>\n";
    result = result + "<p>Found Alignment: </p> <p><img src=\"" + host + "/img_tmp/rot_roi.jpg\"></p>\n";
    result = result + "<p>Aligned Image: </p> <p><img src=\"" + host + "/img_tmp/alg.jpg\"></p>\n";
    return result;
}


bool ClassFlowAlignment::doFlow(string time) 
{
    #ifdef ALGROI_LOAD_FROM_MEM_AS_JPG
        if (!AlgROI)  // AlgROI needs to be allocated before ImageTMP to avoid heap fragmentation
        {
            AlgROI = (ImageData*)heap_caps_realloc(AlgROI, sizeof(ImageData), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);     
            if (!AlgROI) 
            {
                LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't allocate AlgROI");
                LogFile.WriteHeapInfo("ClassFlowAlignment-doFlow");
            }
        }

        if (AlgROI)
        {
            ImageBasis->writeToMemoryAsJPG((ImageData*)AlgROI, 90);
        }
    #endif

    if (!ImageTMP) 
    {
        ImageTMP = new CImageBasis("tmpImage", ImageBasis); // Make sure the name does not get change, it is relevant for the PSRAM allocation!
        if (!ImageTMP) 
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't allocate tmpImage -> Exec this round aborted!");
            LogFile.WriteHeapInfo("ClassFlowAlignment-doFlow");
            return false;
        }
    }

    delete AlignAndCutImage;
    AlignAndCutImage = new CAlignAndCutImage("AlignAndCutImage", ImageBasis, ImageTMP);
    if (!AlignAndCutImage) 
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Can't allocate AlignAndCutImage -> Exec this round aborted!");
        LogFile.WriteHeapInfo("ClassFlowAlignment-doFlow");
        return false;
    }

    CRotateImage rt("rawImage", AlignAndCutImage, ImageTMP, initialflip);
    if (initialflip)
    {
        int _zw = ImageBasis->height;
        ImageBasis->height = ImageBasis->width;
        ImageBasis->width = _zw;

        _zw = ImageTMP->width;
        ImageTMP->width = ImageTMP->height;
        ImageTMP->height = _zw;
    }

    if (initialmirror)
    {
        ESP_LOGD(TAG, "do mirror");
        rt.Mirror();
        
        if (SaveAllFiles)
            AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/mirror.jpg"));
    }
 
    if ((initialrotate != 0) || initialflip)
    {
        if (use_antialiasing)
            rt.RotateAntiAliasing(initialrotate);
        else
            rt.Rotate(initialrotate);
        
        if (SaveAllFiles)
            AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/rot.jpg"));
    }


        //no align algo if set to 3 = off //add disable aligment algo |01.2023
        if(References[0].alignment_algo != 3){
            if (!AlignAndCutImage->Align(&References[0], &References[1])) 
            {
                SaveReferenceAlignmentValues();
            }
        }// no align


    #ifdef ALGROI_LOAD_FROM_MEM_AS_JPG
        if (AlgROI) {
            //no align algo if set to 3 = off => no draw ref //add disable aligment algo |01.2023
            if(References[0].alignment_algo != 3){
                DrawRef(ImageTMP);
            }
            flowctrl.DigitalDrawROI(ImageTMP);
            flowctrl.AnalogDrawROI(ImageTMP);
            ImageTMP->writeToMemoryAsJPG((ImageData*)AlgROI, 90);
        }
    #endif
    
    if (SaveAllFiles)
    {
        AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/alg.jpg"));
        ImageTMP->SaveToFile(FormatFileName("/sdcard/img_tmp/alg_roi.jpg"));
    }

    // must be deleted to have memory space for loading tflite
    delete ImageTMP;
    ImageTMP = NULL;

    //no align algo if set to 3 = off => no draw ref //add disable aligment algo |01.2023
    if(References[0].alignment_algo != 3){
        LoadReferenceAlignmentValues();
    }

    return true;
}


void ClassFlowAlignment::SaveReferenceAlignmentValues()
{
    FILE* pFile;
    std::string zwtime, zwvalue;

    pFile = fopen(FileStoreRefAlignment.c_str(), "w");

    if (strlen(zwtime.c_str()) == 0)
    {
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, 80, "%Y-%m-%dT%H:%M:%S", timeinfo);
        zwtime = std::string(buffer);
    }

    fputs(zwtime.c_str(), pFile);
    fputs("\n", pFile);

    zwvalue = std::to_string(References[0].fastalg_x) + "\t" + std::to_string(References[0].fastalg_y);
    zwvalue = zwvalue + "\t" +std::to_string(References[0].fastalg_SAD)+ "\t" +std::to_string(References[0].fastalg_min); 
    zwvalue = zwvalue + "\t" +std::to_string(References[0].fastalg_max)+ "\t" +std::to_string(References[0].fastalg_avg); 
    fputs(zwvalue.c_str(), pFile);
    fputs("\n", pFile);

    zwvalue = std::to_string(References[1].fastalg_x) + "\t" + std::to_string(References[1].fastalg_y);
    zwvalue = zwvalue + "\t" +std::to_string(References[1].fastalg_SAD)+ "\t" +std::to_string(References[1].fastalg_min); 
    zwvalue = zwvalue + "\t" +std::to_string(References[1].fastalg_max)+ "\t" +std::to_string(References[1].fastalg_avg); 
    fputs(zwvalue.c_str(), pFile);
    fputs("\n", pFile);

    fclose(pFile);
}


bool ClassFlowAlignment::LoadReferenceAlignmentValues(void)
{
    FILE* pFile;
    char zw[1024];
    string zwvalue;
    std::vector<string> splitted;  


    pFile = fopen(FileStoreRefAlignment.c_str(), "r");
    if (pFile == NULL)
        return false;

    fgets(zw, 1024, pFile);
    ESP_LOGD(TAG, "%s", zw);

    fgets(zw, 1024, pFile);
    splitted = ZerlegeZeile(std::string(zw), " \t");
    if (splitted.size() < 6)
    {
        fclose(pFile);
        return false;
    }

    References[0].fastalg_x = stoi(splitted[0]);
    References[0].fastalg_y = stoi(splitted[1]);
    References[0].fastalg_SAD = stof(splitted[2]);
    References[0].fastalg_min = stoi(splitted[3]);
    References[0].fastalg_max = stoi(splitted[4]);
    References[0].fastalg_avg = stof(splitted[5]);

    fgets(zw, 1024, pFile);
    splitted = ZerlegeZeile(std::string(zw));
    if (splitted.size() < 6)
    {
        fclose(pFile);
        return false;
    }

    References[1].fastalg_x = stoi(splitted[0]);
    References[1].fastalg_y = stoi(splitted[1]);
    References[1].fastalg_SAD = stof(splitted[2]);
    References[1].fastalg_min = stoi(splitted[3]);
    References[1].fastalg_max = stoi(splitted[4]);
    References[1].fastalg_avg = stof(splitted[5]);

    fclose(pFile);


    /*#ifdef DEBUG_DETAIL_ON
        std::string _zw = "\tLoadReferences[0]\tx,y:\t" + std::to_string(References[0].fastalg_x) + "\t" + std::to_string(References[0].fastalg_x);
        _zw = _zw + "\tSAD, min, max, avg:\t" + std::to_string(References[0].fastalg_SAD) + "\t" + std::to_string(References[0].fastalg_min);
        _zw = _zw + "\t" + std::to_string(References[0].fastalg_max) + "\t" + std::to_string(References[0].fastalg_avg);
        LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", _zw);
        _zw = "\tLoadReferences[1]\tx,y:\t" + std::to_string(References[1].fastalg_x) + "\t" + std::to_string(References[1].fastalg_x);
        _zw = _zw + "\tSAD, min, max, avg:\t" + std::to_string(References[1].fastalg_SAD) + "\t" + std::to_string(References[1].fastalg_min);
        _zw = _zw + "\t" + std::to_string(References[1].fastalg_max) + "\t" + std::to_string(References[1].fastalg_avg);
        LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", _zw);
    #endif*/

    return true;
}


void ClassFlowAlignment::DrawRef(CImageBasis *_zw)
{
    if (_zw->ImageOkay()) 
    {
        _zw->drawRect(References[0].target_x, References[0].target_y, References[0].width, References[0].height, 255, 0, 0, 2);
        _zw->drawRect(References[1].target_x, References[1].target_y, References[1].width, References[1].height, 255, 0, 0, 2);
    }
}
