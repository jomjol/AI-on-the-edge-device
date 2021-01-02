#include "CImageBasis.h"


class CRotateImage: public CImageBasis
{
    public:
        CImageBasis *ImageTMP;
        CRotateImage(std::string _image) : CImageBasis(_image) {ImageTMP = NULL;};
        CRotateImage(uint8_t* _rgb_image, int _channels, int _width, int _height, int _bpp) : CImageBasis(_rgb_image, _channels, _width, _height, _bpp) {ImageTMP = NULL;};
        CRotateImage(CImageBasis *_org, CImageBasis *_temp);

        void Rotate(float _angle);
        void Rotate(float _angle, int _centerx, int _centery);
        void Translate(int _dx, int _dy);
        void Mirror();
};
