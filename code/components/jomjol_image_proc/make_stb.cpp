#include <stdint.h>
#include <string>

#define STBI_ONLY_JPEG // (save 2% of Flash)
#include "../../include/defines.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
