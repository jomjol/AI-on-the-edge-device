
#include "esp_heap_caps.h"

void *malloc_psram_heap(std::string name, size_t size, uint32_t caps);
void *realloc_psram_heap(std::string name, void *ptr, size_t size, uint32_t caps);
void *calloc_psram_heap(std::string name, size_t n, size_t size, uint32_t caps);

void free_psram_heap(std::string name, void *ptr);