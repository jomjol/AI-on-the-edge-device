#include <stdint.h>
#include <string>
#include "psram.h"

#include "../../include/defines.h"


#define STBI_MALLOC(sz)           psram_reserve_shared_stbi_memory(sz)
#define STBI_REALLOC(p,newsz)     psram_reallocate_shared_stbi_memory(p, newsz)
#define STBI_FREE(p)              psram_free_shared_stbi_memory(p)


#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../stb/stb_image_resize.h"