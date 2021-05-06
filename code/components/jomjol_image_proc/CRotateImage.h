#include "CImageBasis.h"


class CRotateImage: public CImageBasis
{
    public:
        CImageBasis *ImageTMP, *ImageOrg;
        bool doflip;
        CRotateImage(std::string _image, bool _flip = false) : CImageBasis(_image) {ImageTMP = NULL; ImageOrg = NULL; doflip = _flip;};
        CRotateImage(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp, bool _flip = false) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {ImageTMP = NULL;  ImageOrg = NULL; doflip = _flip;};
        CRotateImage(CImageBasis *_org, CImageBasis *_temp, bool _flip = false);

        void Rotate(float _angle);
        void Rotate(float _angle, int _centerx, int _centery);
        void Translate(int _dx, int _dy);
        void Mirror();
};
