#pragma once

#ifndef CLASSFFLOWTAKEIMAGE_H
#define CLASSFFLOWTAKEIMAGE_H

#include "ClassFlowImage.h"
#include "ClassControlCamera.h"
#include "defines.h"

#include <string>

class ClassFlowTakeImage : public ClassFlowImage
{
protected:
    time_t TimeImageTaken;
    std::string namerawimage;

    esp_err_t camera_capture(void);
    bool take_picture_with_flash(int flash_duration);

    void set_initial_parameter(void);

public:
    CImageBasis *rawImage;

    ClassFlowTakeImage(std::vector<ClassFlow *> *lfc);
    ~ClassFlowTakeImage(void);

    bool ReadParameter(FILE *pfile, std::string &aktparamgraph);

    bool doFlow(std::string time);

    std::string get_html_single_step(std::string host);
    time_t get_time_image_taken(void);

    ImageData *send_raw_image(void);
    esp_err_t send_raw_jpg(httpd_req_t *req);

    std::string name(void) { return "ClassFlowTakeImage"; };
};

#endif // CLASSFFLOWTAKEIMAGE_H