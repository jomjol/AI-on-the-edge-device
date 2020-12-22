#include "ClassFlowAlignment.h"
#include "ClassFlowMakeImage.h"
#include "ClassFlow.h"

#include "ClassLogFile.h"

bool AlignmentExtendedDebugging = true;


void ClassFlowAlignment::SetInitialParameter(void)
{
    initalrotate = 0;
    anz_ref = 0;
    suchex = 40;
    suchey = 40;
    initialmirror = false;
    SaveAllFiles = false;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
    ListFlowControll = NULL;
    AlignAndCutImage = NULL;
    ImageBasis = NULL;
    ImageTMP = NULL;
    previousElement = NULL;
    ref_dx[0] = 0; ref_dx[1] = 0;
    ref_dy[0] = 0; ref_dy[1] = 0;
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

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (aktparamgraph.compare("[Alignment]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] == "InitialMirror") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                initialmirror = true;
        }
        if (((zerlegt[0] == "InitalRotate") || (zerlegt[0] == "InitialRotate")) && (zerlegt.size() > 1))
        {
            this->initalrotate = std::stod(zerlegt[1]);
        }
        if ((zerlegt[0] == "SearchFieldX") && (zerlegt.size() > 1))
        {
            this->suchex = std::stod(zerlegt[1]);
        }   
        if ((zerlegt[0] == "SearchFieldY") && (zerlegt.size() > 1))
        {
            this->suchey = std::stod(zerlegt[1]);
        }               
        if ((zerlegt.size() == 3) && (anz_ref < 2))
        {
            reffilename[anz_ref] = FormatFileName("/sdcard" + zerlegt[0]);
            ref_x[anz_ref] = std::stod(zerlegt[1]);
            ref_y[anz_ref] = std::stod(zerlegt[2]);
            anz_ref++;
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }

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
    if (!ImageTMP) 
        ImageTMP = new CImageBasis(ImageBasis);

    if (AlignAndCutImage)
        delete AlignAndCutImage;
    AlignAndCutImage = new CAlignAndCutImage(ImageBasis, ImageTMP);   

    CRotate rt(AlignAndCutImage, ImageTMP);

    if (initialmirror){
        printf("do mirror\n");
        rt.Mirror();
        if (SaveAllFiles) AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/mirror.jpg"));
    }
 
    if (initalrotate != 0)
    {
        rt.Rotate(initalrotate);
        if (SaveAllFiles) AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/rot.jpg"));
    }

    AlignAndCutImage->Align(reffilename[0], ref_x[0], ref_y[0], reffilename[1], ref_x[1], ref_y[1], suchex, suchey, "");
    AlignAndCutImage->GetRefSize(ref_dx, ref_dy);
    if (SaveAllFiles) AlignAndCutImage->SaveToFile(FormatFileName("/sdcard/img_tmp/alg.jpg"));

    if (SaveAllFiles)
    {
        DrawRef(ImageTMP);
        ImageTMP->SaveToFile(FormatFileName("/sdcard/img_tmp/alg_roi.jpg"));
    }

    if (ImageTMP)       // nuss gelöscht werden, um Speicherplatz für das Laden von tflite zu haben
    {
        delete ImageTMP;
        ImageTMP = NULL;
    }  

    return true;
}


void ClassFlowAlignment::DrawRef(CImageBasis *_zw)
{
    _zw->drawRect(ref_x[0], ref_y[0], ref_dx[0], ref_dy[0], 255, 0, 0, 2);
    _zw->drawRect(ref_x[1], ref_y[1], ref_dx[1], ref_dy[1], 255, 0, 0, 2);
}

