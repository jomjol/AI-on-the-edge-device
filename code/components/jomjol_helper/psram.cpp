#include "ClassLogFile.h"
#include "../../include/defines.h"
#include "psram.h"

static const char* TAG = "PSRAM";

using namespace std;


void *shared_region = NULL;
uint32_t allocatedBytes = 0;


bool reserve_psram_shared_region(void) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocating shared PSRAM region (" + 
            std::to_string(TENSOR_ARENA_SIZE + MAX_MODEL_SIZE) + " bytes)...");
    shared_region = malloc_psram_heap("Shared PSRAM region", TENSOR_ARENA_SIZE + MAX_MODEL_SIZE, MALLOC_CAP_SPIRAM);

    if (shared_region == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocating shared PSRAM region!");
        return false;
    }
    else {
        return true;
    }
}




/***********************************************
 * Memory used in Take Image (STBI)
 ***********************************************/
void *psram_reserve_shared_stbi_memory(size_t size) {
    // only do it this way for the defined 4 buffers!
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocating memory for STBI (PSRAM, part of shared memory, " + std::to_string(size) + " bytes)...");
    allocatedBytes += size;
    return shared_region + allocatedBytes - size;
}


void *psram_reallocate_shared_stbi_memory(void *ptr, size_t newsize) {
    // only do it this way for the defined 4 buffers!
    char buf[20];
    sprintf(buf, "%p", ptr);
    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "STBI requested realloc for " + std::string(buf) + " but this is currently unsupported!");
    return NULL;
}


void psram_free_shared_stbi_memory(void *p) {
    // only do it this way for the defined 4 buffers!
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Part of shared memory used for STBI (PSRAM, part of shared memory) is free again");
}



/***********************************************
 * Memory used in Aligning Step 
 ***********************************************/
void *psram_reserve_shared_tmp_image_memory(void) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocating tmpImage (PSRAM, part of shared memory, " + std::to_string(IMAGE_SIZE) + " bytes)...");
    allocatedBytes += IMAGE_SIZE;
    return shared_region; // Use 1th part of the shared memory for the tmpImage (only user)
}


void psram_free_shared_temp_image_memory(void) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Shared memory used for tmpImage (PSRAM, part of shared memory) is free again");
    allocatedBytes = 0;
}



/***********************************************
 * Memory used in Digitalization Steps
 ***********************************************/
void *psram_reserve_shared_tensor_arena_memory(void) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocating Tensor Arena (PSRAM, part of shared memory, " + std::to_string(TENSOR_ARENA_SIZE) + " bytes)...");
    allocatedBytes += TENSOR_ARENA_SIZE;
    return shared_region; // Use 1th part of the shared memory for Tensor
}


void *psram_reserve_shared_model_memory(void) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocating Model memory (PSRAM, part of shared memory, " + std::to_string(MAX_MODEL_SIZE) + " bytes)...");
    allocatedBytes += MAX_MODEL_SIZE;
    return shared_region + allocatedBytes - MAX_MODEL_SIZE; // Use 2nd part of the shared memory (after Tensor Arena) for the model
}


void psram_free_shared_tensor_arena_and_model_memory(void) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Shared memory used for Tensor Arena and model (PSRAM, part of shared memory) is free again");
    allocatedBytes = 0;
}



/***********************************************
 * General
 ***********************************************/
void *malloc_psram_heap(std::string name, size_t size, uint32_t caps) {
	void *ptr;

	ptr = heap_caps_malloc(size, caps);
    if (ptr != NULL) {
	    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for '" + name + "'");
	}
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + to_string(size) + " bytes in PSRAM for '" + name + "'!");
    }

	return ptr;
}


void *realloc_psram_heap(std::string name, void *ptr, size_t size, uint32_t caps) {
	ptr = heap_caps_realloc(ptr, size, caps);
    if (ptr != NULL) {
	    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Reallocated " + to_string(size) + " bytes in PSRAM for '" + name + "'");
	}
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to reallocate " + to_string(size) + " bytes in PSRAM for '" + name + "'!");
    }

	return ptr;
}


void *calloc_psram_heap(std::string name, size_t n, size_t size, uint32_t caps) {
	void *ptr;

	ptr = heap_caps_calloc(n, size, caps);
    if (ptr != NULL) {
	    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for '" + name + "'");
	}
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + to_string(size) + " bytes in PSRAM for '" + name + "'!");
    }

	return ptr;
}


void free_psram_heap(std::string name, void *ptr) {
    LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Freeing memory in PSRAM used for '" + name + "'...");
    heap_caps_free(ptr);
}
