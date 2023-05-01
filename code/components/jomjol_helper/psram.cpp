#include "ClassLogFile.h"
#include "../../include/defines.h"
#include "psram.h"

static const char* TAG = "PSRAM";

using namespace std;


void *shared_region = NULL;
uint32_t allocatedBytesForSTBI = 0;
std::string sharedMemoryInUseFor = "";


/** Reserve a large block in the PSRAM which will be shared between the different steps.
 * Each step uses it differently but only wiuthin itself. */
bool reserve_psram_shared_region(void) {
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating shared PSRAM region (" + 
            std::to_string(TENSOR_ARENA_SIZE + MAX_MODEL_SIZE) + " bytes)...");
    shared_region = malloc_psram_heap("Shared PSRAM region", TENSOR_ARENA_SIZE + MAX_MODEL_SIZE, 
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (shared_region == NULL) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocating shared PSRAM region!");
        return false;
    }
    else {
        return true;
    }
}



/*******************************************************************
 * Memory used in Take Image (STBI)
 *******************************************************************/
bool psram_init_shared_memory_for_take_image_step(void) {
    if (sharedMemoryInUseFor != "") {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + sharedMemoryInUseFor + "!");
        return false;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init shared memory for step 'Take Image' (STBI buffers)");
    allocatedBytesForSTBI = 0;
    sharedMemoryInUseFor = "TakeImage";

    return true;
}


void psram_deinit_shared_memory_for_take_image_step(void) {
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Deinit shared memory for step 'Take Image' (STBI buffers)");
    allocatedBytesForSTBI = 0;
    sharedMemoryInUseFor = "";
}


void *psram_reserve_shared_stbi_memory(size_t size) {
    /* Only large buffers should be placed in the shared PSRAM 
     * If we also place all smaller STBI buffers here, we get artefacts for some reasons. */
    if (size >= 100000) {
        if ((allocatedBytesForSTBI + size) > TENSOR_ARENA_SIZE + MAX_MODEL_SIZE) { // Check if it still fits in the shared region
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM too small (STBI) to fit additional " + 
                    std::to_string(size) + " bytes! Available: " + std::to_string(TENSOR_ARENA_SIZE + MAX_MODEL_SIZE - allocatedBytesForSTBI) + " bytes!");

            return NULL;
        }
        
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating memory (" + std::to_string(size) + " bytes) for STBI (use shared memory in PSRAM)...");
        allocatedBytesForSTBI += size;
        return (uint8_t *)shared_region + allocatedBytesForSTBI - size;
    }
    else { // Normal PSRAM
        return malloc_psram_heap("STBI", size, MALLOC_CAP_SPIRAM);
    }
}


void *psram_reallocate_shared_stbi_memory(void *ptr, size_t newsize) {
    char buf[20];
    sprintf(buf, "%p", ptr);
    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "STBI requested realloc for " + std::string(buf) + " but this is currently unsupported!");
    return NULL;
}


void psram_free_shared_stbi_memory(void *p) {
    if ((p >= shared_region) && (p <= ((uint8_t *)shared_region + allocatedBytesForSTBI))) { // was allocated inside the shared memory
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Part of shared memory used for STBI (PSRAM, part of shared memory) is free again");
    }
    else { // Normal PSRAM
        free_psram_heap("STBI", p);
    }
}



/*******************************************************************
 * Memory used in Aligning Step 
 * During this step we only use the shared part of the PSRAM
 * for the tmpImage. 
 *******************************************************************/
void *psram_reserve_shared_tmp_image_memory(void) {
    if (sharedMemoryInUseFor != "") {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + sharedMemoryInUseFor + "!");
        return NULL;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating tmpImage (" + std::to_string(IMAGE_SIZE) + " bytes, use shared memory in PSRAM)...");
    sharedMemoryInUseFor = "Aligning";
    return shared_region; // Use 1th part of the shared memory for the tmpImage (only user)
}


void psram_free_shared_temp_image_memory(void) {
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Shared memory used for tmpImage (PSRAM, part of shared memory) is free again");
    sharedMemoryInUseFor = "";
}



/*******************************************************************
 * Memory used in Digitalization Steps
 * During this step we only use the shared part of the PSRAM for the
 * Tensor Arena and one of the Models.
 * The shared memory is large enough for the largest model and the
 * Tensor Arena. Therefore we do not need to monitor the usage.
 *******************************************************************/
void *psram_get_shared_tensor_arena_memory(void) {
    if ((sharedMemoryInUseFor == "") || (sharedMemoryInUseFor == "Digitalization_Model")) {
        sharedMemoryInUseFor = "Digitalization_Tensor";
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating Tensor Arena (" + std::to_string(TENSOR_ARENA_SIZE) + " bytes, use shared memory in PSRAM)...");
        return shared_region; // Use 1th part of the shared memory for Tensor
    }
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + sharedMemoryInUseFor + "!");
        return NULL;
    }
}


void *psram_get_shared_model_memory(void) {
    if ((sharedMemoryInUseFor == "") || (sharedMemoryInUseFor == "Digitalization_Tensor")) {
        sharedMemoryInUseFor = "Digitalization_Model";
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating Model memory (" + std::to_string(MAX_MODEL_SIZE) + " bytes, use shared memory in PSRAM)...");
        return (uint8_t *)shared_region + TENSOR_ARENA_SIZE; // Use 2nd part of the shared memory (after Tensor Arena) for the model
    }
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + sharedMemoryInUseFor + "!");
        return NULL;
    }
}


void psram_free_shared_tensor_arena_and_model_memory(void) {
    sharedMemoryInUseFor = "";
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Shared memory used for Tensor Arena and model (PSRAM, part of shared memory) is free again");
}



/*******************************************************************
 * General
 *******************************************************************/
void *malloc_psram_heap(std::string name, size_t size, uint32_t caps) {
	void *ptr;

	ptr = heap_caps_malloc(size, caps);
    if (ptr != NULL) {
	    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for '" + name + "'");
	}
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + to_string(size) + " bytes in PSRAM for '" + name + "'!");
    }

	return ptr;
}


void *realloc_psram_heap(std::string name, void *ptr, size_t size, uint32_t caps) {
	ptr = heap_caps_realloc(ptr, size, caps);
    if (ptr != NULL) {
	    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Reallocated " + to_string(size) + " bytes in PSRAM for '" + name + "'");
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
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Freeing memory in PSRAM used for '" + name + "'...");
    heap_caps_free(ptr);
}
