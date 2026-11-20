#include "ClassLogFile.h"
#include "defines.h"
#include "psram.h"

#include <stdint.h>
#include <stddef.h>
#include <cstddef>
#include <limits.h>
#include <algorithm>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "PSRAM";

using namespace std;

size_t allocatedBytesForSTBI = 0;
std::string sharedMemoryInUseFor = "";

void *shared_region = NULL;
static size_t shared_region_size = 0;
static SemaphoreHandle_t shared_memory_mutex = nullptr;

namespace
{
    constexpr uint32_t STBI_ALLOCATION_MAGIC = 0x53544249; // "STBI"

    struct alignas(std::max_align_t) StbiAllocationHeader
    {
        uint32_t magic;
        size_t size;
        bool shared;
    };

    static bool checked_stbi_allocation_size(size_t requested, size_t *total)
    {
        if (total == nullptr || requested > SIZE_MAX - sizeof(StbiAllocationHeader))
        {
            return false;
        }

        *total = sizeof(StbiAllocationHeader) + requested;

        return true;
    }

    static StbiAllocationHeader *stbi_header_from_payload(void *payload)
    {
        return payload == nullptr ? nullptr : reinterpret_cast<StbiAllocationHeader *>(payload) - 1;
    }

    static bool is_in_shared_region(const void *ptr)
    {
        if (ptr == nullptr || shared_region == nullptr)
        {
            return false;
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        const uintptr_t begin = reinterpret_cast<uintptr_t>(shared_region);

        return shared_region_size <= UINTPTR_MAX - begin && address >= begin && address < begin + shared_region_size;
    }
}

static bool lock_shared_memory(void)
{
    return shared_memory_mutex != nullptr && xSemaphoreTake(shared_memory_mutex, portMAX_DELAY) == pdTRUE;
}

static void unlock_shared_memory(void)
{
    if (shared_memory_mutex != nullptr)
    {
        (void)xSemaphoreGive(shared_memory_mutex);
    }
}

/** Reserve a large block in the PSRAM which will be shared between the different steps.
 * Each step uses it differently but only wiuthin itself. */
bool reserve_psram_shared_region(void)
{
    if (shared_memory_mutex == nullptr)
    {
        shared_memory_mutex = xSemaphoreCreateMutex();

        if (shared_memory_mutex == nullptr)
        {
            LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to create shared PSRAM manager mutex");
            return false;
        }
    }

    if (!lock_shared_memory())
    {
        return false;
    }

    if (shared_region != NULL)
    {
        unlock_shared_memory();
        return true;
    }

    shared_region_size = static_cast<size_t>(TENSOR_ARENA_SIZE) + static_cast<size_t>(MAX_MODEL_SIZE);
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating shared PSRAM region (" + std::to_string(shared_region_size) + " bytes)...");
    shared_region = malloc_psram_heap("Shared PSRAM region", shared_region_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (shared_region == NULL)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate shared PSRAM region!");
        shared_region_size = 0;
        unlock_shared_memory();
        return false;
    }

    unlock_shared_memory();

    return true;
}

/*******************************************************************
 * Memory used in Take Image (STBI)
 *******************************************************************/
bool psram_init_shared_memory_for_take_image_step(void)
{
    if (!lock_shared_memory())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared PSRAM manager is not initialized");
        return false;
    }

    const bool alreadyInUse = !sharedMemoryInUseFor.empty();
    const std::string currentOwner = sharedMemoryInUseFor;

    if (!alreadyInUse)
    {
        allocatedBytesForSTBI = 0;
        sharedMemoryInUseFor = "TakeImage";
    }

    unlock_shared_memory();

    if (alreadyInUse)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + currentOwner + "!");
        return false;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Init shared memory for step 'Take Image' (STBI buffers)");
    return true;
}

void psram_deinit_shared_memory_for_take_image_step(void)
{
    if (!lock_shared_memory())
    {
        return;
    }

    const bool ownedByTakeImage = (sharedMemoryInUseFor == "TakeImage");

    if (ownedByTakeImage)
    {
        allocatedBytesForSTBI = 0;
        sharedMemoryInUseFor.clear();
    }

    const std::string currentOwner = sharedMemoryInUseFor;
    unlock_shared_memory();

    if (!ownedByTakeImage)
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Ignoring TakeImage release; shared PSRAM is owned by " + currentOwner);
        return;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Deinit shared memory for step 'Take Image' (STBI buffers)");
}

void *psram_reserve_shared_stbi_memory(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    size_t totalSize = 0;

    if (!checked_stbi_allocation_size(size, &totalSize))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "STBI allocation size overflow");
        return NULL;
    }

    /* Keep small STBI allocations on the heap; large decode buffers may use
       the shared arena only while TakeImage exclusively owns it. */
    if (size >= 100000 && lock_shared_memory())
    {
        const size_t alignment = alignof(StbiAllocationHeader);
        const size_t alignedOffset = (allocatedBytesForSTBI + alignment - 1) & ~(alignment - 1);

        if (shared_region != NULL && sharedMemoryInUseFor == "TakeImage" && alignedOffset <= shared_region_size && totalSize <= shared_region_size - alignedOffset)
        {
            auto *header = reinterpret_cast<StbiAllocationHeader *>(static_cast<uint8_t *>(shared_region) + alignedOffset);
            header->magic = STBI_ALLOCATION_MAGIC;
            header->size = size;
            header->shared = true;
            allocatedBytesForSTBI = alignedOffset + totalSize;
            void *result = static_cast<void *>(header + 1);
            unlock_shared_memory();
            LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating " + std::to_string(size) + " bytes for STBI in shared PSRAM region");
            return result;
        }

        const bool useTakeImage = (sharedMemoryInUseFor == "TakeImage");
        unlock_shared_memory();

        if (useTakeImage)
        {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Shared STBI region full; trying normal PSRAM allocation for " + std::to_string(size) + " bytes");
        }
    }

    // Never place an STBI buffer over the Tensor Arena/model or alignment
    // scratch buffer. Those stages own the shared area exclusively.
    void *allocation = heap_caps_malloc(totalSize, MALLOC_CAP_SPIRAM);

    if (allocation == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + std::to_string(size) + " bytes for STBI");
        return NULL;
    }

    auto *header = static_cast<StbiAllocationHeader *>(allocation);
    header->magic = STBI_ALLOCATION_MAGIC;
    header->size = size;
    header->shared = false;

    return static_cast<void *>(header + 1);
}

void *psram_reallocate_shared_stbi_memory(void *ptr, size_t newsize)
{
    if (ptr == nullptr)
    {
        return psram_reserve_shared_stbi_memory(newsize);
    }

    if (newsize == 0)
    {
        psram_free_shared_stbi_memory(ptr);
        return NULL;
    }

    size_t totalSize = 0;

    if (!checked_stbi_allocation_size(newsize, &totalSize))
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "STBI reallocation size overflow");
        return NULL;
    }

    StbiAllocationHeader *header = stbi_header_from_payload(ptr);

    if (header == nullptr || header->magic != STBI_ALLOCATION_MAGIC)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "STBI realloc received an unowned pointer");
        return NULL;
    }

    if (header->shared)
    {
        const size_t oldSize = header->size;
        void *replacement = psram_reserve_shared_stbi_memory(newsize);

        if (replacement == nullptr)
        {
            return NULL;
        }

        std::memcpy(replacement, ptr, std::min(oldSize, newsize));
        // Shared allocations are bump-allocated and reclaimed as a group.
        header->magic = 0;
        return replacement;
    }

    void *newAllocation = heap_caps_realloc(header, totalSize, MALLOC_CAP_SPIRAM);

    if (newAllocation == nullptr)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to reallocate " + std::to_string(newsize) + " bytes for STBI");
        return NULL;
    }

    auto *newHeader = static_cast<StbiAllocationHeader *>(newAllocation);
    newHeader->magic = STBI_ALLOCATION_MAGIC;
    newHeader->size = newsize;
    newHeader->shared = false;

    return static_cast<void *>(newHeader + 1);
}

void psram_free_shared_stbi_memory(void *p)
{
    if (p == nullptr)
    {
        return;
    }

    StbiAllocationHeader *header = stbi_header_from_payload(p);

    if (is_in_shared_region(header))
    {
        if (header->magic == STBI_ALLOCATION_MAGIC && header->shared)
        {
            header->magic = 0; // The arena is reclaimed in one operation.
        }
        return;
    }

    if (header->magic != STBI_ALLOCATION_MAGIC || header->shared)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "STBI free received an invalid allocation header");
        return;
    }

    header->magic = 0;
    heap_caps_free(header);
}

/*******************************************************************
 * Memory used in Aligning Step
 * During this step we only use the shared part of the PSRAM
 * for the tmpImage.
 *******************************************************************/
void *psram_reserve_shared_tmp_image_memory(size_t requiredSize)
{
    if (!lock_shared_memory())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared PSRAM manager is not initialized");
        return NULL;
    }

    if (!sharedMemoryInUseFor.empty())
    {
        const std::string currentOwner = sharedMemoryInUseFor;
        unlock_shared_memory();
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + currentOwner + "!");
        return NULL;
    }

    if (shared_region == NULL || requiredSize == 0 || requiredSize > static_cast<size_t>(IMAGE_SIZE) || requiredSize > shared_region_size)
    {
        unlock_shared_memory();
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared PSRAM region unavailable or too small for tmpImage (" + std::to_string(requiredSize) + " bytes)");
        return NULL;
    }

    sharedMemoryInUseFor = "Aligning";
    unlock_shared_memory();
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating tmpImage (" + std::to_string(requiredSize) + " bytes, use shared memory in PSRAM)...");

    return shared_region;
}

void psram_free_shared_temp_image_memory(void)
{
    if (!lock_shared_memory())
    {
        return;
    }

    const bool ownedByAligning = (sharedMemoryInUseFor == "Aligning");

    if (ownedByAligning)
    {
        sharedMemoryInUseFor.clear();
    }

    const std::string currentOwner = sharedMemoryInUseFor;
    unlock_shared_memory();

    if (!ownedByAligning)
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Ignoring tmpImage release because shared memory is owned by " + currentOwner);
        return;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Shared memory used for tmpImage is free again");
}

/*******************************************************************
 * Memory used in Digitization Steps
 * During this step we only use the shared part of the PSRAM for the
 * Tensor Arena and one of the Models.
 * The shared memory is large enough for the largest model and the
 * Tensor Arena. Therefore we do not need to monitor the usage.
 *******************************************************************/
void *psram_get_shared_tensor_arena_memory(void)
{
    if (!lock_shared_memory())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared PSRAM manager is not initialized for Tensor Arena");
        return NULL;
    }

    if (shared_region == NULL || shared_region_size < static_cast<size_t>(TENSOR_ARENA_SIZE))
    {
        unlock_shared_memory();
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared PSRAM region unavailable for Tensor Arena");
        return NULL;
    }

    if (sharedMemoryInUseFor.empty() || sharedMemoryInUseFor == "Digitization_Model")
    {
        sharedMemoryInUseFor = "Digitization_Tensor";
        unlock_shared_memory();
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating Tensor Arena (" + std::to_string(TENSOR_ARENA_SIZE) + " bytes)");
        return shared_region;
    }

    const std::string currentOwner = sharedMemoryInUseFor;
    unlock_shared_memory();
    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + currentOwner + "!");

    return NULL;
}

void *psram_get_shared_model_memory(void)
{
    const size_t requiredSize = static_cast<size_t>(TENSOR_ARENA_SIZE) + static_cast<size_t>(MAX_MODEL_SIZE);
    if (!lock_shared_memory())
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared PSRAM manager is not initialized for model memory");
        return NULL;
    }

    if (shared_region == NULL || shared_region_size < requiredSize)
    {
        unlock_shared_memory();
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared PSRAM region unavailable for model memory");
        return NULL;
    }

    if (sharedMemoryInUseFor.empty() || sharedMemoryInUseFor == "Digitization_Tensor")
    {
        sharedMemoryInUseFor = "Digitization_Model";
        unlock_shared_memory();
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocating Model memory (" + std::to_string(MAX_MODEL_SIZE) + " bytes)");
        return static_cast<uint8_t *>(shared_region) + TENSOR_ARENA_SIZE;
    }

    const std::string currentOwner = sharedMemoryInUseFor;
    unlock_shared_memory();
    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Shared memory in PSRAM already in use for " + currentOwner + "!");

    return NULL;
}

void psram_free_shared_tensor_arena_and_model_memory(void)
{
    if (!lock_shared_memory())
    {
        return;
    }

    const bool canRelease = sharedMemoryInUseFor.empty() || sharedMemoryInUseFor == "Digitization_Model" || sharedMemoryInUseFor == "Digitization_Tensor";

    if (canRelease)
    {
        sharedMemoryInUseFor.clear();
    }

    const std::string currentOwner = sharedMemoryInUseFor;
    unlock_shared_memory();

    if (!canRelease)
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Ignoring tensor/model release because shared memory is owned by " + currentOwner);
        return;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Shared memory used for Tensor Arena and model is free again");
}

/*******************************************************************
 * General
 *******************************************************************/
void *malloc_psram_heap(std::string name, size_t size, uint32_t caps)
{
    void *ptr = heap_caps_malloc(size, caps);

    if (ptr != NULL)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for '" + name + "'");
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + to_string(size) + " bytes in PSRAM for '" + name + "'!");
    }

    return ptr;
}

void *realloc_psram_heap(std::string name, void *ptr, size_t size, uint32_t caps)
{
    ptr = heap_caps_realloc(ptr, size, caps);

    if (ptr != NULL)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Reallocated " + to_string(size) + " bytes in PSRAM for '" + name + "'");
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to reallocate " + to_string(size) + " bytes in PSRAM for '" + name + "'!");
    }

    return ptr;
}

void *calloc_psram_heap(std::string name, size_t n, size_t size, uint32_t caps)
{
    void *ptr = heap_caps_calloc(n, size, caps);

    if (ptr != NULL)
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Allocated " + to_string(n * size) + " bytes in PSRAM for '" + name + "'");
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + to_string(size) + " bytes in PSRAM for '" + name + "'!");
    }

    return ptr;
}

void free_psram_heap(std::string name, void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Freeing memory in PSRAM used for '" + name + "'...");
    heap_caps_free(ptr);
}
