#include "esp_camera.h"

typedef struct
{
    framesize_t ImageFrameSize = FRAMESIZE_VGA; // 0 - 10
    gainceiling_t ImageGainceiling;             // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)

    int ImageQuality;    // 0 - 63
    int ImageBrightness; // (-2 to 2) - set brightness
    int ImageContrast;   //-2 - 2
    int ImageSaturation; //-2 - 2
    int ImageSharpness;  //-2 - 2
    bool ImageAutoSharpness;
    int ImageSpecialEffect; // 0 - 6
    int ImageWbMode;        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    int ImageAwb;           // white balance enable (0 or 1)
    int ImageAwbGain;       // Auto White Balance enable (0 or 1)
    int ImageAec;           // auto exposure off (1 or 0)
    int ImageAec2;          // automatic exposure sensor  (0 or 1)
    int ImageAeLevel;       // auto exposure levels (-2 to 2)
    int ImageAecValue;      // set exposure manually  (0-1200)
    int ImageAgc;           // auto gain off (1 or 0)
    int ImageAgcGain;       // set gain manually (0 - 30)
    int ImageBpc;           // black pixel correction
    int ImageWpc;           // white pixel correction
    int ImageRawGma;        // (1 or 0)
    int ImageLenc;          // lens correction (1 or 0)
    int ImageHmirror;       // (0 or 1) flip horizontally
    int ImageVflip;         // Invert image (0 or 1)
    int ImageDcw;           // downsize enable (1 or 0)

    int ImageDenoiseLevel; // The OV2640 does not support it, OV3660 and OV5640 (0 to 8)

    int ImageWidth;
    int ImageHeight;

    int ImageLedIntensity = 4096;

    bool ImageZoomEnabled;
    int ImageZoomOffsetX;
    int ImageZoomOffsetY;
    int ImageZoomSize;

    bool CameraFocusEnabled;
    bool CameraManualFocus;
    uint16_t CameraManualFocusLevel;
} cam_config_t;
