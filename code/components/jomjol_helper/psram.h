#pragma once
#ifndef PSRAM_h
#define PSRAM_h

#include "esp_heap_caps.h"


bool reserve_psram_shared_region(void);


/* Memory used in Take Image Step */
bool psram_init_shared_memory_for_take_image_step(void);
void psram_deinit_shared_memory_for_take_image_step(void);
void *psram_reserve_shared_stbi_memory(size_t size);
void *psram_reallocate_shared_stbi_memory(void *ptr, size_t newsize);
void psram_free_shared_stbi_memory(void *p);


/* Memory used in Aligning Step */
void *psram_reserve_shared_tmp_image_memory(void);
void psram_free_shared_temp_image_memory(void);

/* Memory used in Digitalization Steps */
void *psram_get_shared_tensor_arena_memory(void);
void *psram_get_shared_model_memory(void);
void psram_free_shared_tensor_arena_and_model_memory(void);

/* General */
void *malloc_psram_heap(std::string name, size_t size, uint32_t caps);
void *realloc_psram_heap(std::string name, void *ptr, size_t size, uint32_t caps);
void *calloc_psram_heap(std::string name, size_t n, size_t size, uint32_t caps);

void free_psram_heap(std::string name, void *ptr);

#endif // PSRAM_h
