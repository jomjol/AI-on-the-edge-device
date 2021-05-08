#include "ClassFlowAlignment.h"
#include "ClassFlowMakeImage.h"
#include "ClassFlow.h"

#include "CRotateImage.h"


#include "ClassLogFile.h"



bool AlignmentExtendedDebugging = true;

// #define DEBUG_DETAIL_ON  


void ClassFlowAlignment::SetInitialParameter(void)
{
    initalrotate = 0;
    anz_ref = 0;
    initialmirror = false;
    initialflip = false;
    SaveAllFiles = false;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
    FileStoreRefAlignment = "/sdcard/config/align.txt";
    ListFlowControll = NULL;
    AlignAndCutImage = NULL;
    ImageBasis = NULL;
    ImageTMP = NULL;
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
        if (((*ListFlowControll)[i])->name().compare("ClassFlowMakeImage") == 0)
        {
            ImageBasis = ((ClassFlowMakeImage*) (*ListFlowControll)[i])->rawImage;
        }
    }

    if (!ImageBasis)            // die Funktion Bilder aufnehmen existiert nicht --> muss erst erzeugt werden NUR ZU TESTZWECKEN
    {
        if (AlignmentExtendedDebugging) printf("CImageBasis musste erzeugt werden\n");
        ImageBasis = new CImageBasis(namerawimage);
    }
}


bool ClassFlowAlignment::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;
    int suchex = 40;
    int suchey = 40;
    int alg_algo = 0;


    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (aktparamgraph.compare("[Alignment]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = ZerlegeZeile(aktparamgraph);
        if ((toUpper(zerlegt[0]) == "FLIPIMAGESIZE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                initialflip = true;
        }
        if ((toUpper(zerlegt[0]) == "INITIALMIRROR") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                initialmirror = true;
        }
        if (((toUpper(zerlegt[0]) == "INITALROTATE") || (toUpper(zerlegt[0]) == "INITIALROTATE")) && (zerlegt.size() > 1))
        {
            this->initalrotate = std::stod(zerlegt[1]);
        }
        if ((toUpper(zerlegt[0]) == "SEARCHFIELDX") && (zerlegt.size() > 1))
        {
            suchex = std::stod(zerlegt[1]);
        }   
        if ((toUpper(zerlegt[0]) == "SEARCHFIELDY") && (zerlegt.size() > 1))
        {
            suchey = std::stod(zerlegt[1]);
        }               
        if ((zerlegt.size() == 3) && (anz_ref < 2))
        {
            References[anz_ref].image_file = FormatFileName("/sdcard" + zerlegt[0]);
            References[anz_ref].target_x = std::stod(zerlegt[1]);
            References[anz_ref].target_y = std::stod(zerlegt[2]);
            anz_ref++;
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }
        if ((toUpper(zerlegt[0]) == "ALIGNMENTALGO") && (zerlegt.size() > 1))
        {
#ifdef DEBUG_DETAIL_ON
            std::string zw2 = "Alignmentmodus gewählt: " + zerlegt[1];
            LogFile.WriteToFile(zw2);
#endif
            if (toUpper(zerlegt[1]) == "HIGHACCURACY")
                alg_algo = 1;
            if (toUpper(zerlegt[1]) == "FAST")
                alg_algo = 2;
        }
    }

    for (int i = 0; i < anz_ref; ++i)
    {
        References[i].search_x = suchex;
        References[i].search_y = suchey;
        References[i].fastalg_SAD_criteria = SAD_criteria;
        References[i].alignment_algo = alg_algo;
#ifdef DEBUG_DETAIL_ON
        std::string zw2 = "Alignmentmodus geschrieben: " + std::to_string(alg_algo);
        LogFile.WriteToFile(zw2);
#endif
    }

    LoadReferenceAlignmentValues();
    
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
    if (!ImageTMP) 
        ImageTMP = new CImageBasis(ImageBasis, 5);

    if (AlignAndCutImage)
        delete AlignAndCutImage;
    AlignAndCutImage = new CAlignAndCutImage(ImageBasis, ImageTMP);   

    CRotateImage rt(AlignAndCutImage, ImageTMP, initialflip);
    if (initialflip)
    {
        int _zw = ImageBasis->height;
        ImageBasis->height = ImageBasis->width;
        ImageBasis->width = _zw;
    }

    if (initialmirror){
        printf("do mirror\n");
        rt.Mirror();
        if (SaveAllFiles) AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/mirror.jpg"));
    }
 
    if ((initalrotate != 0) || initialflip)
    {
        rt.Rotate(initalrotate);
        if (SaveAllFiles) AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/rot.jpg"));
    }

    if (!AlignAndCutImage->Align(&References[0], &References[1])) 
    {
        SaveReferenceAlignmentValues();
    }

    if (SaveAllFiles) AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/alg.jpg"));

    if (SaveAllFiles)
    {
        if (initialflip)
        {
            int _zw = ImageTMP->width;
            ImageTMP->width = ImageTMP->height;
            ImageTMP->height = _zw;
        }
        DrawRef(ImageTMP);
        ImageTMP->SaveToFile(FormatFileName("/sdcard/img_tmp/alg_roi.jpg"));
    }

    if (ImageTMP)       // nuss gelöscht werden, um Speicherplatz für das Laden von tflite zu haben
    {
        delete ImageTMP;
        ImageTMP = NULL;
    }  

    LoadReferenceAlignmentValues();

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
    std::vector<string> zerlegt;  


//    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", "LoadReferenceAlignmentValues01");      

    pFile = fopen(FileStoreRefAlignment.c_str(), "r");
    if (pFile == NULL)
        return false;

//    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", "LoadReferenceAlignmentValues01");      

    fgets(zw, 1024, pFile);
    printf("%s", zw);

//    zwvalue = "LoadReferenceAlignmentValues Time: " + std::string(zw);

//    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", zwvalue);      

//    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", "LoadReferenceAlignmentValues02");      

    fgets(zw, 1024, pFile);
    zerlegt = ZerlegeZeile(std::string(zw), " \t");
    if (zerlegt.size() < 6)
    {
//        LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", "Exit 01");      
        fclose(pFile);
        return false;
    }

//    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", "LoadReferenceAlignmentValues03");      

    References[0].fastalg_x = stoi(zerlegt[0]);
    References[0].fastalg_y = stoi(zerlegt[1]);
    References[0].fastalg_SAD = stof(zerlegt[2]);
    References[0].fastalg_min = stoi(zerlegt[3]);
    References[0].fastalg_max = stoi(zerlegt[4]);
    References[0].fastalg_avg = stof(zerlegt[5]);

    fgets(zw, 1024, pFile);
    zerlegt = ZerlegeZeile(std::string(zw));
    if (zerlegt.size() < 6)
    {
//        LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", "Exit 02");      
        fclose(pFile);
        return false;
    }

//    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", "LoadReferenceAlignmentValues03");      

    References[1].fastalg_x = stoi(zerlegt[0]);
    References[1].fastalg_y = stoi(zerlegt[1]);
    References[1].fastalg_SAD = stof(zerlegt[2]);
    References[1].fastalg_min = stoi(zerlegt[3]);
    References[1].fastalg_max = stoi(zerlegt[4]);
    References[1].fastalg_avg = stof(zerlegt[5]);

    fclose(pFile);


#ifdef DEBUG_DETAIL_ON  
    std::string _zw = "\tLoadReferences[0]\tx,y:\t" + std::to_string(References[0].fastalg_x) + "\t" + std::to_string(References[0].fastalg_x);
    _zw = _zw + "\tSAD, min, max, avg:\t" + std::to_string(References[0].fastalg_SAD) + "\t" + std::to_string(References[0].fastalg_min);
    _zw = _zw + "\t" + std::to_string(References[0].fastalg_max) + "\t" + std::to_string(References[0].fastalg_avg);
    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", _zw);
    _zw = "\tLoadReferences[1]\tx,y:\t" + std::to_string(References[1].fastalg_x) + "\t" + std::to_string(References[1].fastalg_x);
    _zw = _zw + "\tSAD, min, max, avg:\t" + std::to_string(References[1].fastalg_SAD) + "\t" + std::to_string(References[1].fastalg_min);
    _zw = _zw + "\t" + std::to_string(References[1].fastalg_max) + "\t" + std::to_string(References[1].fastalg_avg);
    LogFile.WriteToDedicatedFile("/sdcard/alignment.txt", _zw);
#endif

    return true;
}



void ClassFlowAlignment::DrawRef(CImageBasis *_zw)
{
    _zw->drawRect(References[0].target_x, References[0].target_y, References[0].width, References[0].height, 255, 0, 0, 2);
    _zw->drawRect(References[1].target_x, References[1].target_y, References[1].width, References[1].height, 255, 0, 0, 2);
}

