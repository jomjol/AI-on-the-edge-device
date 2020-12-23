#include "CImageBasis.h"


class CFindTemplate : public CImageBasis
{
    public:
        int tpl_width, tpl_height, tpl_bpp;    
//        CFindTemplate(std::string _image);
        CFindTemplate(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {};


        void FindTemplate(std::string _template, int* found_x, int* found_y, std::string _imageout);
        void FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy, std::string _imageout);
        void FindTemplate(std::string _template, int* found_x, int* found_y);
        void FindTemplate(std::string _template, int* found_x, int* found_y, int _dx, int _dy);
};