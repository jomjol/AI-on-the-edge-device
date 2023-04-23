#include <stdint.h>
#include <string>
#include "psram.h"

#include "../../include/defines.h"


#define USE_SHARED_PSRAM_FOR_STBI

#ifdef USE_SHARED_PSRAM_FOR_STBI
#define STBI_MALLOC(sz)           psram_reserve_shared_stbi_memory(sz)
#define STBI_REALLOC(p,newsz)     psram_reallocate_shared_stbi_memory(p, newsz)
#define STBI_FREE(p)              psram_free_shared_stbi_memory(p)
#else // Use normal PSRAM
#define STBI_MALLOC(sz)           malloc_psram_heap("STBI", sz, MALLOC_CAP_SPIRAM)
#define STBI_REALLOC(p,newsz)     realloc_psram_heap("STBI", p, newsz, MALLOC_CAP_SPIRAM)
#define STBI_FREE(p)              free_psram_heap("STBI", p)
#endif


#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../stb/stb_image_resize.h"