#include "ClassFlowAlignment.h"

ClassFlowAlignment::ClassFlowAlignment()
{
    initalrotate = 0;
    anz_ref = 0;
    suchex = 40;
    suchey = 40;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
    ListFlowControll = NULL;
}

ClassFlowAlignment::ClassFlowAlignment(std::vector<ClassFlow*>* lfc)
{
    initalrotate = 0;
    anz_ref = 0;
    suchex = 40;
    suchey = 40;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";    
    ListFlowControll = lfc;
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
        if ((zerlegt[0] == "InitalRotate") && (zerlegt.size() > 1))
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
            this->reffilename[anz_ref] = FormatFileName("/sdcard" + zerlegt[0]);
            this->ref_x[anz_ref] = std::stod(zerlegt[1]);
            this->ref_y[anz_ref] = std::stod(zerlegt[2]);
            anz_ref++;
        }
    }
    return true;

}

bool ClassFlowAlignment::doFlow(string time)
{
    string input = namerawimage;
    string output = "/sdcard/img_tmp/rot.jpg";
    string output3 = "/sdcard/img_tmp/rot_roi.jpg";
    string output2 = "/sdcard/img_tmp/alg.jpg";
    string output4 = "/sdcard/img_tmp/alg_roi.jpg";

    input = FormatFileName(input);
    output = FormatFileName(output);
    output2 = FormatFileName(output2);

    CRotate *rt;

    if (this->initalrotate != 0)
    {
        rt = new CRotate(input);
        rt->Rotate(this->initalrotate);
        rt->SaveToFile(output);
        delete rt;
    }
    else
    {
        CopyFile(input, output);
    }

    CAlignAndCutImage *caic;
    caic = new CAlignAndCutImage(output);
    caic->Align(this->reffilename[0], this->ref_x[0], this->ref_y[0], this->reffilename[1], this->ref_x[1], this->ref_y[1], suchex, suchey, output3);
    caic->SaveToFile(output2);

    printf("Startwriting Output4:%s\n", output4.c_str());
    if (output4.length() > 0)
    {
        caic->drawRect(ref_x[0], ref_y[0], caic->t0_dx, caic->t0_dy, 255, 0, 0, 2);
        caic->drawRect(ref_x[1], ref_y[1], caic->t1_dx, caic->t1_dy, 255, 0, 0, 2);
        caic->SaveToFile(output4);
        printf("Write output4: %s\n", output4.c_str());
    }

    delete caic;

    // Align mit Templates
    return true;
}

