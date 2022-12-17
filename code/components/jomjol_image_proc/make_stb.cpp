#include <stdint.h>
#include <string>

#include "../../include/defines.h"

//#define STB_IMAGE_STATIC //added 17.12.2022
//#define STBI_ONLY_JPEG //added 17.12.2022

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"